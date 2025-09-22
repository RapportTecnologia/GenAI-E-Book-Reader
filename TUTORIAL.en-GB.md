# GenAI E‑Book Reader — Tutorial

This guide shows you how to use the app’s main features, from basic reading to advanced AI capabilities such as semantic search (RAG) and chatting with an LLM.

## 1. Getting Started: Opening and Navigating

1. **Open a Book**: Go to `File > Open...` to select a PDF file.
2. **Navigation**:
   * Use the mouse wheel to scroll through pages.
   * Use `Ctrl + mouse wheel` to zoom in/out.
3. **TOC (Table of Contents)**: The left‑hand panel lets you navigate the content. Use the buttons at the top of the panel to toggle between **Pages** and **Contents** (chapters) views.

## 2. Interactive Reading: Selection, Dictionary and Search

### Select Text and Images

- **Select Text**: In the `Edit` menu, choose `Select text` and mark the desired passage.
- **Select Image**: Choose `Select rectangle (image)` to capture an area of the document as an image.
- **Actions**: With text or an image selected, you can copy, save to a file, or send it to the AI chat.

### Consult the Dictionary

1. **Configure**: Go to `Settings > Dictionary`. By default, the dictionary uses the configured LLM to provide definitions.
2. **Use**: Select a word in the text and right‑click. In the context menu, choose the option to consult the dictionary. The definition will appear in the chat panel.

### Search the Document

The top search bar supports two types of search:

- **Text Search**: Type a word or phrase and click "Search". The app will locate exact matches in the text.
- **Semantic Search (RAG)**: If text search finds nothing, the app will automatically perform a semantic search that understands the meaning of your query. For this to work, the document needs to be indexed first (see the RAG section below).

## 3. Semantic Search with RAG (Retrieval‑Augmented Generation)

RAG lets you ask natural‑language questions about the book’s content. To use it, you must first create an "embedding index" for your document.

### Step 1: Configure Embeddings

1. Go to `Settings > Embeddings`.
2. **Provider**: Choose a provider to generate embeddings. `Ollama` is a great option to run locally on your machine.
3. **Model**: Select an embedding model (e.g., `nomic-embed-text`).
4. **Chunking Parameters**: Adjust `Chunk size` and `Overlap` to control how text is split before processing. Default values usually work well.

### Step 2: Index the Document

1. With the book open, right‑click inside the viewer and choose `Recreate document embeddings...`.
2. Wait for the process to finish. A window will show the progress.

### Step 3: Perform a Semantic Search

- Now use the search bar to ask questions such as "What is the main idea of chapter 2?" or "Explain the concept of X". The app will use the RAG index to find the most relevant parts of the book and answer your question.

## 4. AI Integration (LLM)

You can connect the app to an LLM service to use chat, request summaries, synonyms, and more.

### Configure Access to the LLM

1. Go to `Settings > LLM Settings`.
2. **Provider**: Choose a provider — `OpenAI`, `GenerAtiva`, `Ollama` (local) or `OpenRouter`.
3. **List/Test Model**: Use the "List models" action (when available for the provider) and the "Test model" button to validate credentials and connectivity.
4. **API Key**: Enter your API key (for cloud services). For `Ollama`, no token is required; just make sure the service is running at `http://localhost:11434`.
5. **Model**: Select the model you want to use (e.g., `gpt-4o-mini`, `llama3`).
6. **Prompts**: Customise the prompts the AI will use to generate summaries, synonyms, etc.

Settings are stored in `QSettings` with the keys:
- `ai/provider`: `openai` | `generativa` | `ollama` | `openrouter`
- `ai/base_url`: base URL override (optional)
- `ai/api_key`: secret token (not applicable to `ollama`)
- `ai/model`: model name (e.g., `gpt-4o-mini`, `llama3`)
- `ai/prompts/synonyms`, `ai/prompts/summaries`, `ai/prompts/explanations`, `ai/prompts/chat`

Privacy:
- The application always asks for confirmation before sending any passage to the AI.
- Tokens are stored in the user’s preferences.

### Use the AI Features

- **Chat**: Open the chat panel to converse with the AI. You can ask questions, request explanations, or send passages and images from the book for analysis.
- **Summaries and Synonyms**: Select some text, right‑click, and choose the option to summarise or find synonyms.
- **Dictionary**: Select a word and use the dictionary function to get its definition in the chat panel.

Notes:
- Messages use the model selected in LLM Settings.
- Dialogue prompts can be customised in LLM Settings.
- In 0.1.9, compatibility with `OpenRouter` was added. A courtesy key may be available for initial testing; we recommend configuring your own key for ongoing use.

## 5. Tips and Additional Resources

- **Title Button**: The button in the centre of the top bar shows your book’s name. Right‑click it to access quick actions such as renaming the file or opening it in the file manager.
- **Troubleshooting**: If the AI does not respond, check your LLM settings (API Key, Base URL) and your Internet connection.
- **Shortcuts**: Use `Ctrl+Enter` to send messages in the chat panel.
