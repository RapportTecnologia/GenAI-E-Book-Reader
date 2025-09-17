#include "ai/EmbeddingIndexer.h"

#include <QFileInfo>
#include <QDir>
#include <QCryptographicHash>
#include <QProcess>
#include <QPdfDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QUuid>
#include <QThread>
#include <QDebug>

EmbeddingIndexer::EmbeddingIndexer(const Params& p, QObject* parent)
    : QObject(parent), p_(p) {}

void EmbeddingIndexer::forEachChunks(const QString& text, int chunkSize, int overlap,
                       const std::function<bool(QStringView, int)>& consume) {
    // Minimal normalization: CR -> LF
    QStringView fullView(text);
    // parameter validation similar to chunkText
    int cs = chunkSize;
    if (cs <= 0) { emit warn(tr("chunkSize inválido (%1). Usando 1000.").arg(cs)); cs = 1000; }
    int ov = overlap;
    if (ov < 0) { emit warn(tr("overlap negativo (%1). Usando 100.").arg(ov)); ov = 100; }
    if (ov >= cs) {
        int newOv = qMax(0, cs / 4);
        emit warn(tr("overlap (%1) >= chunkSize (%2). Ajustando overlap para %3.").arg(ov).arg(cs).arg(newOv));
        ov = newOv;
    }

    // Work with indices over the original string to avoid large allocations
    const int n = text.size();
    if (n == 0) return;
    int i = 0;
    int localIdx = 0;
    while (i < n) {
        const int take = qMax(1, qMin(cs, n - i));
        int j = i + take;
        // Trim leading/trailing whitespace for the current chunk via indices
        int start = i;
        int end = j; // exclusive
        // skip CR -> treat as LF replacement when creating view
        while (start < end && text.at(start).isSpace()) ++start;
        while (end > start && text.at(end - 1).isSpace()) --end;
        if (end > start) {
            QStringView v(fullView.mid(start, end - start));
            if (!consume(v, localIdx)) {
                // consumer requested to stop early
                break;
            }
        }
        // Emit metric sparsely to avoid flooding (every 1000 chunks)
        if (((localIdx + 1) % 1000) == 0) {
            emit metric(QStringLiteral("chunk"), QString::number(localIdx + 1));
        }
        int nextI = j - ov;
        if (nextI <= i) nextI = i + take;
        i = nextI;
        ++localIdx;
    }
    // Final metric with total chunks processed for this call
    emit metric(QStringLiteral("chunk_total"), QString::number(localIdx));
}

QStringList EmbeddingIndexer::extractPagesText() {
    QStringList pages;
    QPdfDocument doc;
    auto st = doc.load(p_.pdfPath);
    if (st != static_cast<QPdfDocument::Error>(0)) {
        emit warn(tr("Falha ao abrir PDF para contagem de páginas."));
        return pages;
    }
    const int pageCount = doc.pageCount();
    emit metric(QStringLiteral("pages"), QString::number(pageCount));

    const bool hasPdfToText = !QStandardPaths::findExecutable("pdftotext").isEmpty();
    const bool hasPdfToPpm = !QStandardPaths::findExecutable("pdftoppm").isEmpty();
    const bool hasTesseract = !QStandardPaths::findExecutable("tesseract").isEmpty();

    if (!hasPdfToText) {
        emit warn(tr("Ferramenta 'pdftotext' não encontrada. Instale o pacote 'poppler-utils' para extração de texto mais rápida."));
    }
    if (!hasTesseract) {
        emit warn(tr("Ferramenta 'tesseract' não encontrada. Instale 'tesseract-ocr' para fallback via OCR."));
    }

    // Preferir pdftotext; fallback para pdftoppm+tesseract; por fim páginas vazias
    for (int i = 1; i <= pageCount; ++i) {
        if (QThread::currentThread()->isInterruptionRequested()) break;
        QString pageText;
        if (hasPdfToText) {
            QProcess proc;
            QStringList args; args << "-q" << "-layout" << "-eol" << "unix" << "-f" << QString::number(i) << "-l" << QString::number(i) << p_.pdfPath << "-";
            proc.start("pdftotext", args);
            if (proc.waitForStarted(2000)) {
                proc.waitForFinished(20000);
                pageText = QString::fromUtf8(proc.readAllStandardOutput());
            }
        }
        if (pageText.trimmed().isEmpty() && hasPdfToPpm && hasTesseract) {
            // Fallback: renderiza a página como PNG e roda OCR
            const QString tmpDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
            QDir().mkpath(tmpDir);
            const QString base = QUuid::createUuid().toString(QUuid::WithoutBraces);
            const QString outPngBase = QDir(tmpDir).filePath(base);
            // pdftoppm -f i -l i -r 200 -png pdf outPngBase
            QProcess ppm;
            QStringList a; a << "-f" << QString::number(i) << "-l" << QString::number(i) << "-r" << "200" << "-png" << p_.pdfPath << outPngBase;
            ppm.start("pdftoppm", a);
            if (ppm.waitForStarted(2000)) {
                ppm.waitForFinished(30000);
                const QString imgPath = outPngBase + "-" + QString::number(i) + ".png";
                if (QFileInfo::exists(imgPath)) {
                    QProcess ocr;
                    QStringList ta; ta << imgPath << "stdout" << "-l" << "por+eng" << "--psm" << "6";
                    ocr.start("tesseract", ta);
                    if (ocr.waitForStarted(2000)) {
                        ocr.waitForFinished(30000);
                        pageText = QString::fromUtf8(ocr.readAllStandardOutput());
                    }
                    QFile::remove(imgPath);
                }
            }
        }
        if (pageText.isEmpty()) {
            if (!hasPdfToText && !(hasPdfToPpm && hasTesseract)) {
                emit warn(tr("Sem ferramentas de extração instaladas. Instale 'poppler-utils' (pdftotext) e/ou 'tesseract-ocr'."));
            }
        }
        pages << pageText;
    }
    return pages;
}


QString EmbeddingIndexer::sha1(const QString& s) const {
    QCryptographicHash h(QCryptographicHash::Sha1);
    h.addData(s.toUtf8());
    return QString::fromLatin1(h.result().toHex());
}

void EmbeddingIndexer::run() {
    QElapsedTimer total; total.start();
    emit stage(tr("Lendo PDF"));
    const QStringList pages = extractPagesText();
    const int pageCount = pages.size();
    if (pageCount == 0) { emit error(tr("Sem páginas extraídas.")); emit finished(false, tr("Falha na extração de texto")); return; }
    qInfo() << "[EmbeddingIndexer] total pages to process=" << pageCount;

    // Preparar arquivos de saída (escrita incremental)
    emit stage(tr("Preparando arquivos"));
    QDir().mkpath(p_.dbDir);
    const QString fileHash = sha1(QFileInfo(p_.pdfPath).absoluteFilePath());
    // Build output file base name, sanitizing only the filename part (not the directory)
    QString fileBaseName = QStringLiteral("index_%1_%2").arg(fileHash, p_.providerCfg.model);
    fileBaseName.replace(':', '_');
    fileBaseName.replace('/', '_'); // e.g., sentence-transformers model IDs
    const QString base = QDir(p_.dbDir).filePath(fileBaseName);
    const QString binPath = base + ".bin";
    const QString idsPath = base + ".ids.json";
    const QString metaPath = base + ".meta.json";
    qInfo() << "[EmbeddingIndexer] output paths" << "bin=" << binPath << "ids=" << idsPath << "meta=" << metaPath;

    QFile fb(binPath);
    if (!fb.open(QIODevice::WriteOnly)) {
        const QString em = tr("Falha ao abrir binário '%1' para escrita: %2").arg(binPath, fb.errorString());
        emit error(em);
        qCritical() << em;
        emit finished(false, tr("Falha ao abrir binário"));
        return;
    }
    // Header temporário: magic + count + dim(0)
    fb.write("VEC1", 4);
    int countInt = 0; // streaming mode: will be updated at the end with globalChunkIdx
    int dimInt = 0; // desconhecido até a 1ª resposta
    qint64 posCount = fb.pos();
    fb.write(reinterpret_cast<const char*>(&countInt), sizeof(int));
    qint64 posDim = fb.pos();
    fb.write(reinterpret_cast<const char*>(&dimInt), sizeof(int));

    QFile fids(idsPath);
    if (!fids.open(QIODevice::WriteOnly)) {
        const QString em = tr("Falha ao abrir ids '%1' para escrita: %2").arg(idsPath, fids.errorString());
        emit error(em);
        qCritical() << em;
        fb.close(); QFile::remove(binPath);
        emit finished(false, tr("Falha ao abrir ids"));
        return;
    }
    fids.write("["); bool firstId = true;

    QFile fmeta(metaPath);
    if (!fmeta.open(QIODevice::WriteOnly)) {
        const QString em = tr("Falha ao abrir metadados '%1' para escrita: %2").arg(metaPath, fmeta.errorString());
        emit error(em);
        qCritical() << em;
        fb.close(); fids.close(); QFile::remove(binPath); QFile::remove(idsPath);
        emit finished(false, tr("Falha ao abrir meta"));
        return;
    }
    fmeta.write("["); bool firstMeta = true;

    emit stage(tr("Gerando embeddings"));
    EmbeddingProvider prov(p_.providerCfg);
    const int bs = qMax(1, p_.batchSize);
    qint64 processed = 0; // number of chunks processed
    int globalChunkIdx = 0;
    int pagesProcessed = 0;
    for (int i=0;i<pageCount;++i) {
        if (QThread::currentThread()->isInterruptionRequested()) { emit warn(tr("Interrompido")); break; }
        emit metric(QStringLiteral("page"), QString::number(i+1));
        QStringList batchTexts; batchTexts.reserve(bs);
        QList<QPair<int,int>> batchLocs; batchLocs.reserve(bs);

        auto processBatch = [&](bool finalFlush=false) -> bool {
            if (batchTexts.isEmpty()) return true;
            // Embedding do lote
            QList<QVector<float>> vecs;
            // Log batch details prior to external API call
            {
                const int totalChars = std::accumulate(batchTexts.begin(), batchTexts.end(), 0, [](int s, const QString& t){ return s + t.size(); });
                qInfo() << "[EmbeddingIndexer] embedding batch start"
                        << "provider=" << p_.providerCfg.provider
                        << "model=" << p_.providerCfg.model
                        << "baseUrl=" << p_.providerCfg.baseUrl
                        << "page=" << (i+1)
                        << "batch_size=" << batchTexts.size()
                        << "chars_total=" << totalChars;
            }
            try { vecs = prov.embedBatch(batchTexts); }
            catch (const std::exception& ex) {
                const QString em = tr("Falha em embedBatch (page=%1, batch=%2): %3").arg(i+1).arg(batchTexts.size()).arg(QString::fromUtf8(ex.what()));
                emit error(em);
                qCritical() << em;
                return false;
            }
            qInfo() << "[EmbeddingIndexer] embedding batch ok"
                    << "vectors=" << vecs.size()
                    << "dim=" << (vecs.isEmpty() ? 0 : vecs.first().size());
            // Ensure files are open before writing
            if (!fb.isOpen() || !fids.isOpen() || !fmeta.isOpen()) {
                const QString em = tr("Arquivos de saída não estão abertos para escrita (fb=%1, fids=%2, fmeta=%3).")
                                      .arg(fb.isOpen()).arg(fids.isOpen()).arg(fmeta.isOpen());
                emit error(em);
                qCritical() << em;
                return false;
            }
            // Inicializar dim na 1ª vez
            if (dimInt == 0 && !vecs.isEmpty()) {
                dimInt = vecs.first().size();
                qint64 cur = fb.pos(); fb.seek(posDim); fb.write(reinterpret_cast<const char*>(&dimInt), sizeof(int)); fb.seek(cur);
            }
            // Persistir vetores/ids/meta
            for (int k=0;k<vecs.size();++k) {
                const QVector<float>& v = vecs[k];
                if (fb.write(reinterpret_cast<const char*>(v.data()), sizeof(float)*v.size()) <= 0) {
                    const QString em = tr("Falha ao escrever vetor no arquivo binário '%1': %2").arg(binPath, fb.errorString());
                    emit error(em);
                    qCritical() << em;
                    return false;
                }
                if (!firstId) fids.write(","); firstId = false;
                if (fids.write("\"") <= 0 || fids.write(QString::number(globalChunkIdx).toUtf8()) <= 0 || fids.write("\"") <= 0) {
                    const QString em = tr("Falha ao escrever id no arquivo '%1': %2").arg(idsPath, fids.errorString());
                    emit error(em);
                    qCritical() << em;
                    return false;
                }
                const auto& loc = batchLocs[k];
                QJsonObject o; o.insert("file", QFileInfo(p_.pdfPath).absoluteFilePath()); o.insert("page", loc.first); o.insert("chunk", loc.second); o.insert("model", p_.providerCfg.model); o.insert("provider", p_.providerCfg.provider);
                if (!firstMeta) fmeta.write(","); firstMeta = false;
                if (fmeta.write(QJsonDocument(o).toJson(QJsonDocument::Compact)) <= 0) {
                    const QString em = tr("Falha ao escrever metadados no arquivo '%1': %2").arg(metaPath, fmeta.errorString());
                    emit error(em);
                    qCritical() << em;
                    return false;
                }
                ++globalChunkIdx;
            }
            processed += batchTexts.size();
            const int pctDoc = int((double(i + 1) / double(pageCount)) * 100.0);
            emit progress(pctDoc, tr("embedding batch size=%1 (página %2)").arg(batchTexts.size()).arg(i+1));
            if (p_.pauseMsBetweenBatches > 0) {
                QThread::msleep(static_cast<unsigned long>(p_.pauseMsBetweenBatches));
            }
            batchTexts.clear(); batchLocs.clear();
            return true;
        };

        bool aborted = false;
        int chunkIdx = 0;
        forEachChunks(pages[i], p_.chunkSize, p_.chunkOverlap, [&](QStringView v, int localIdx) -> bool {
            // Convert view to QString only for the provider input
            batchTexts << v.toString();
            batchLocs << QPair<int,int>(i+1, localIdx);
            ++chunkIdx;
            if (batchTexts.size() == bs) {
                if (!processBatch()) {
                    // Abort cleanly: close files and stop processing
                    fb.close(); fids.close(); fmeta.close();
                    const QString wm = tr("Abortando processamento: falha ao gerar embeddings (página %1). Fechando arquivos.").arg(i+1);
                    emit warn(wm);
                    qWarning() << wm;
                    emit finished(false, tr("Falha ao gerar embeddings"));
                    aborted = true;
                    return false; // stop forEachChunks
                }
            }
            return true; // continue
        });
        if (aborted) return; // exit run()
        // flush remaining
        if (!processBatch(true)) {
            fb.close(); fids.close(); fmeta.close();
            const QString wm = tr("Abortando processamento no flush final da página %1. Fechando arquivos.").arg(i+1);
            emit warn(wm);
            qWarning() << wm;
            emit finished(false, tr("Falha ao gerar embeddings"));
            return; // exit run()
        }
        ++pagesProcessed;
    }

    // Finalizar arquivos
    if (fids.isOpen()) { if (fids.write("]") <= 0) { const QString wm = tr("Falha ao finalizar ids '%1': %2").arg(idsPath, fids.errorString()); emit warn(wm); qWarning() << wm; } fids.close(); }
    if (fmeta.isOpen()) { if (fmeta.write("]") <= 0) { const QString wm = tr("Falha ao finalizar meta '%1': %2").arg(metaPath, fmeta.errorString()); emit warn(wm); qWarning() << wm; } fmeta.close(); }
    // Garantir que count e dim estejam corretos
    if (fb.isOpen()) {
        fb.seek(posCount); countInt = globalChunkIdx; fb.write(reinterpret_cast<const char*>(&countInt), sizeof(int));
        if (dimInt == 0) { dimInt = 1; }
        fb.seek(posDim); fb.write(reinterpret_cast<const char*>(&dimInt), sizeof(int));
        fb.close();
    } else {
        const QString wm = tr("Arquivo binário '%1' fechou antes da atualização do cabeçalho.").arg(binPath);
        emit warn(wm);
        qWarning() << wm;
    }

    if (processed == 0) { emit warn(tr("Nenhum vetor persistido.")); emit finished(false, tr("Nada produzido")); return; }

    emit stage(tr("Concluído"));
    emit metric(QStringLiteral("total_time_ms"), QString::number(total.elapsed()));
    // Always process all pages in one indexing run
    emit progress(100, QStringLiteral("completed"));
    emit finished(true, tr("Indexação concluída"));
}
