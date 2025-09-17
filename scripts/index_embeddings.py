#!/usr/bin/env python3
import argparse
import os
import sys
import time
import hashlib
import json
from typing import List, Dict, Any

# Lazy imports with helpful error messages

def require(pkg, pip_name=None):
    try:
        return __import__(pkg)
    except Exception as e:
        print(f"ERROR Missing dependency: {pkg}. Please install with: pip install {pip_name or pkg}", file=sys.stderr)
        raise


def load_pdf_text(pdf_path: str) -> List[str]:
    pypdf = require('pypdf')
    reader = pypdf.PdfReader(pdf_path)
    pages = []
    for i, page in enumerate(reader.pages):
        try:
            text = page.extract_text() or ''
        except Exception:
            text = ''
        pages.append(text)
    return pages


def chunk_text(text: str, chunk_size: int = 1000, chunk_overlap: int = 200) -> List[str]:
    text = text.replace('\r', '\n')
    text = '\n'.join(line.strip() for line in text.splitlines())
    chunks = []
    i = 0
    n = len(text)
    if n == 0:
        return []
    while i < n:
        j = min(n, i + chunk_size)
        chunk = text[i:j]
        chunk = chunk.strip()
        if chunk:
            chunks.append(chunk)
        i = j - chunk_overlap
        if i < 0:
            i = 0
        if i >= n:
            break
    return chunks


def sha1(s: str) -> str:
    return hashlib.sha1(s.encode('utf-8')).hexdigest()


class EmbeddingProvider:
    def __init__(self, name: str, model: str, base_url: str = None, api_key: str = None):
        self.name = name
        self.model = model
        self.base_url = base_url
        self.api_key = api_key
        self._st_model = None

    def embed_batch(self, texts: List[str]) -> List[List[float]]:
        if self.name == 'openai' or self.name == 'generativa':
            openai = require('openai')
            client = openai.OpenAI(api_key=self.api_key, base_url=self.base_url) if self.name == 'generativa' else openai.OpenAI(api_key=self.api_key)
            # OpenAI API supports batching inputs
            resp = client.embeddings.create(model=self.model, input=texts)
            return [d.embedding for d in resp.data]
        elif self.name == 'ollama':
            requests = require('requests')
            url = (self.base_url or 'http://localhost:11434').rstrip('/') + '/api/embeddings'
            out = []
            for t in texts:
                r = requests.post(url, json={"model": self.model, "prompt": t})
                r.raise_for_status()
                data = r.json()
                out.append(data['embedding'])
            return out
        elif self.name == 'sentence_transformers':
            sentence_transformers = require('sentence_transformers')
            if self._st_model is None:
                self._st_model = sentence_transformers.SentenceTransformer(self.model)
            # Show_progress_bar False to keep stdout clean
            return self._st_model.encode(texts, show_progress_bar=False, normalize_embeddings=False).tolist()
        else:
            raise RuntimeError(f"Unknown provider {self.name}")


def upsert_chroma(db_path: str, collection: str, ids: List[str], docs: List[str], metadatas: List[Dict[str, Any]], embeddings: List[List[float]]):
    chromadb = require('chromadb')
    client = chromadb.PersistentClient(path=db_path)
    coll = client.get_or_create_collection(collection_name=collection)

    # Chroma accepts upserts with ids, documents, metadatas, embeddings
    coll.upsert(ids=ids, documents=docs, metadatas=metadatas, embeddings=embeddings)


def main():
    parser = argparse.ArgumentParser(description='Index PDF embeddings into ChromaDB')
    parser.add_argument('--pdf', required=True, help='Path to PDF file')
    parser.add_argument('--db-path', required=True, help='ChromaDB persistent path')
    parser.add_argument('--provider', required=True, choices=['generativa', 'openai', 'ollama', 'sentence_transformers'])
    parser.add_argument('--model', required=True)
    parser.add_argument('--base-url', default=None)
    parser.add_argument('--api-key', default=None)
    parser.add_argument('--chunk-size', type=int, default=1000)
    parser.add_argument('--chunk-overlap', type=int, default=200)
    parser.add_argument('--batch-size', type=int, default=16)

    args = parser.parse_args()

    t0 = time.time()
    print('STAGE loading_pdf')
    pages = load_pdf_text(args.pdf)
    print(f'METRIC pages {len(pages)}')

    # Build chunks per page
    print('STAGE chunking')
    chunks = []
    locs = []  # (page_index, chunk_index_within_page)
    for i, ptxt in enumerate(pages):
        cs = chunk_text(ptxt, args.chunk_size, args.chunk_overlap)
        if not cs:
            continue
        for j, c in enumerate(cs):
            chunks.append(c)
            locs.append((i+1, j))
    print(f'METRIC chunks {len(chunks)}')

    if not chunks:
        print('WARN no_chunks_extracted')
        return 0

    # Provider init
    print('STAGE provider_init')
    provider = EmbeddingProvider(args.provider, args.model, base_url=args.base_url, api_key=args.api_key)

    # Embeddings
    print('STAGE embedding')
    embs: List[List[float]] = []
    total = len(chunks)
    bs = max(1, args.batch_size)
    for start in range(0, total, bs):
        end = min(total, start + bs)
        batch = chunks[start:end]
        t_batch0 = time.time()
        vecs = provider.embed_batch(batch)
        t_batch1 = time.time()
        if len(vecs) != len(batch):
            print('ERROR batch_mismatch', file=sys.stderr)
            sys.exit(2)
        embs.extend(vecs)
        # Progress notify
        pct = int((end / total) * 100)
        dt = t_batch1 - t_batch0
        print(f'PROGRESS {pct} embedding_batch={start//bs} size={len(batch)} time_s={dt:.3f}')
        sys.stdout.flush()

    # Upsert
    print('STAGE upsert')
    file_hash = sha1(os.path.abspath(args.pdf))
    collection = f'pdf_{file_hash}_{args.model}'.replace(':', '_').replace('/', '_')
    ids = [f'{file_hash}_{i}' for i in range(len(chunks))]
    metadatas = [
        {
            'file': os.path.abspath(args.pdf),
            'page': int(locs[i][0]),
            'chunk': int(locs[i][1]),
            'model': args.model,
            'provider': args.provider,
        }
        for i in range(len(chunks))
    ]
    upsert_chroma(args.db_path, collection, ids, chunks, metadatas, embs)

    t1 = time.time()
    print('STAGE done')
    print(f'METRIC total_time_s {t1 - t0:.3f}')
    print(f'METRIC collection {collection}')
    print('PROGRESS 100 completed')

    return 0


if __name__ == '__main__':
    sys.exit(main())
