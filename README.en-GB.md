![Project Visitors](https://visitor-badge.laobi.icu/badge?page_id=rapporttecnologia.genai-e-book-reader)
[![License: CC BY 4.0](https://img.shields.io/badge/License-CC%20BY%204.0-lightgrey.svg)](LICENSE)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue)
![Qt](https://img.shields.io/badge/Qt6-Widgets-brightgreen)
![CMake](https://img.shields.io/badge/CMake-%3E%3D3.16-informational)
[![Docs](https://img.shields.io/badge/docs-Doxygen-blueviolet)](docs/index.html)
[![Latest Release](https://img.shields.io/github/v/release/RapportTecnologia/GenAi-E-Book-Reader?label=version)](https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/latest)
[![Contributions Welcome](https://img.shields.io/badge/contributions-welcome-success.svg)](#contributing)

<!-- Translations -->
**Translations:** 🇧🇷 [Português (original)](README.md) | 🇸🇦 [العربية](README.ar.md) | 🇬🇧 English (UK) | 🇫🇷 [Français](README.fr-FR.md)

<figure style="text-align: center;">
    <img src="docs/imgs/logo-do-projeto.png" alt="GenAI E-Book Reader">
    <figcaption>GenAI E-Book Reader</figcaption>
    </figure>

## Project Financial Support

If this project is useful to you and you want to support ongoing development, you can send a PIX of any amount. Thank you!

![PIX](docs/imgs/pix.png)

PIX: _consultoria@carlosdelfino.eti.br_ or _(+55 85) 98520-5490_

# GenAI E‑Book Reader

A modern e‑book reader focused on productivity and study, developed in C/C++ with Qt6, with planned features such as annotations, a dictionary, Text‑to‑Speech (TTS), reading statistics, and AI support (RAG) for summaries and explanations.

- Changelog: see [CHANGELOG.md](CHANGELOG.md).
- Release planning: see [ROADMAP.md](ROADMAP.md).
- Step‑by‑step tutorial: see [TUTORIAL.en-GB.md](TUTORIAL.en-GB.md).

## How to Get the App

You can download the latest stable release or build the development version to access the newest features.

### Stable Release (Recommended)

The latest stable version is **v0.1.12**. For most users we recommend downloading the ready‑to‑run executable.

1. Go to the [Releases page](https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/latest).
2. Download `GenAI_EBook_Reader-v0.1.12-x86_64.AppImage`.
3. Make it executable:
    ```bash
    chmod +x GenAI_EBook_Reader-v0.1.12-x86_64.AppImage
    ```
4. Run the app:
    ```bash
    ./GenAI_EBook_Reader-v0.1.12-x86_64.AppImage
    ```

### Development Version

If you want to try the latest features that will land in the next release, you can build the project from source. This version includes new features and bug fixes but may be unstable.

## Tutorials
- 🇧🇷 [Tutorial in Portuguese](TUTORIAL.md)
- 🇸🇦 [الدليل بالعربية](TUTORIAL.ar.md)
- 🇬🇧 [Tutorial (English, UK)](TUTORIAL.en-GB.md)
- 🇫🇷 [Tutoriel (Français)](TUTORIAL.fr-FR.md)

## Video Tutorials
* [Project Presentation](https://www.youtube.com/watch?v=4wveYzO_Lko)
* [OpenRouter.ia Presentation](https://www.youtube.com/watch?v=dHggyhodAH4&t=4s)
* [How to associate E‑Book files with GER](https://www.youtube.com/watch?v=2a1KO5Vig0k)

* [More videos](https://www.youtube.com/@RapportTecnologia/videos)

## Key Features (MVP)
- PDF reading with basic navigation and light/dark theme. (EPUB/MOBI support is on the roadmap.)
- Highlights and annotations with a side panel and JSON export.
- On‑click dictionary (at least 1 language).
- TTS for selected passages, with basic controls.
- AI (OpenAI) to summarise/explain passages with RAG.
- Statistics: active time, progress, sessions.
- Session restore (reopens previously opened books).
- Calibre integration to open books from your library.
- TOC panel with toolbar:
  - Switch between a list of pages ("Pages") and a chapter tree ("Contents").
  - Back and Forward buttons navigate page by page (Pages mode) or item/chapter by chapter (Contents mode).
  - Default panel width ~10% and reader area ~90% on first run (adjustable via splitter).
- Top bar with a centred title button showing the current document name and providing a context menu (open folder, add to Calibre with embedding migration, rename with migration).
- Search bar with literal text search and fallback to semantic phrase search using embeddings; includes quick options (metric, Top‑K and thresholds).
- Support for OPF files (e‑book metadata), with basic reading and display of information.

### Release Notes

Note (0.1.12 – development): Focus on improving search (text and semantic) and chat interaction with Function Calling support where the model/provider supports it. The LLM Settings window now displays a read‑only indicator stating whether the selected model declares support for Function Calling.

Note (0.1.11): New supported LLM providers (local Ollama, GenerAtiva and OpenRouter), improvements to the LLM Settings UI (model listing and model test), CI and CMake tweaks (local release target), expanded debugging for provider/model selection, removal of the PHPList dependency, and customised user/LLM interaction. A courtesy key for initial OpenRouter use is included (we recommend configuring your own key).

Note (0.1.10): Support for clickable internal links in PDFs (index/summary within the document) using QPdfLinkModel (Qt6), improved navigation via the TOC (synchronisation with clicks and Back/Forward buttons), fixes in text selection and minor UI tweaks. Documentation updated.

Note (0.1.7): Added the ability to open e‑books directly from the command line and file associations in the OS. The dictionary was started (currently using an LLM) and the app info panel was improved.

Note (0.1.6): Optimised chat rendering by moving Markdown conversion to the back end (C++), fixing instability bugs with MathJax.

Note (0.1.3): UI refinements — app icon from `docs/imgs/logo-do-projeto.png`, splash screen with version/author and window title showing the book name (metadata when available; otherwise, file name). Adjusted reading area to fill 100% of the available space. TOC panel redesigned with a toolbar (switch between "Pages"/"Contents" and navigation buttons) and an initial default splitter size of ~10% (TOC) / ~90% (viewer).

Note (0.1.2): Implemented "Save as" (RF‑28) and small reading improvements.

Note (0.1.1): Added page selection via combobox and restoration of the last opened file/folder.

Observation: for PDFs, the TOC uses bookmarks (chapters/subchapters) when available; otherwise, it lists all pages. The combobox selection covers all document pages. The TOC panel includes a toolbar to switch between "Pages" and "Contents" and navigation buttons; by default, the panel takes ~10% of the window width on first run. Currently, the binary supports reading PDFs; OPF files are accepted for reading metadata.

## TOC and Navigation
- Toggle TOC mode:
  - "Pages": flat list of pages; the "Back/Forward" buttons change the current page.
  - "Contents": chapters (page groups); the buttons move to the previous/next item (chapter or child page).
- Panel tips: drag the splitter to resize; the size is saved for subsequent sessions.

## Document Search (text and semantic)

- The search bar is at the top and contains:
  - Search field, "Search", "Previous" and "Next" buttons.
  - "Options" menu with quick similarity settings: metric (Cosine, Dot or L2), Top‑K, similarity threshold (for cosine/dot) and maximum distance (for L2).
- How it works:
  - The search first tries to find the literal text in the PDF (per page).
  - If nothing is found, the app performs a semantic phrase search using the document's embedding index and jumps to the most relevant pages.
- Prerequisite for semantic search: the document must have embeddings indexed.
  - To recreate the index, right‑click within the PDF and choose "Recreate document embeddings...".
  - You can also adjust provider/model and parameters in `Settings > Embeddings`.

### Title Button (top bar)

- Shows the document title (for PDFs, uses Title metadata when available; otherwise, file name).
- Click to see the full file path and copy it to the clipboard.
- Context menu (right‑click):
  - "Open folder in file manager"
  - "Add to Calibre and migrate embeddings..."
  - "Rename file and migrate embeddings..."

## AI (LLM): Configuration and Usage
The application integrates with providers compatible with the OpenAI API for chat, summaries and synonyms.
Currently supported:
- OpenAI (`https://api.openai.com`)
- GenerAtiva (`https://generativa.rapport.tec.br`)
- Ollama (local, `http://localhost:11434`)
- OpenRouter (`https://openrouter.ai`)

How to configure:
- Open the "LLM Settings" dialogue (Settings menu).
- Select the provider (OpenAI, GenerAtiva, Ollama or OpenRouter) and the model.
- List and select an available model (when the provider offers model listing) and use the "Test model" button to validate credentials and connectivity.
- Enter the API Key of the chosen provider (for OpenAI/GenerAtiva/OpenRouter). For local Ollama, no key is required; just ensure the service is running at `http://localhost:11434`.
- Optional: fill in "Base URL" to point to an OpenAI‑compatible endpoint when applicable.
- Adjust the default prompts for Synonyms, Summaries, Explanations and Chat as you prefer.

### Function Calling Indicator (0.1.10)
- The LLM Settings window now displays a read‑only field indicating whether the selected provider/model supports Function Calling.
- When the provider's API exposes this capability via model listings or metadata, detection is automatic and the indicator shows "Supports Function Calling". Otherwise, an informative message is shown (e.g., "Capability not reported by provider").
- Note: effective Function Calling support depends on the chosen provider/model and may vary over time as APIs evolve.

Usage in the reader:
- Synonyms: select a word/phrase and trigger the AI action for synonyms; you will be asked for consent before sending.
- Summary: select a passage and trigger the AI action for summary; the result opens in the summary dialogue.
- Chat: send a passage to the AI chat or type freely in the chat panel.
  - Advanced rendering in the chat panel (Markdown/HTML):
    - Markdown tables (GFM) with borders, header and horizontal scrolling when necessary.
    - Syntax highlighting for code blocks (highlight.js, GitHub theme).
    - MathJax v3 for formulae (inline and display), applied after Markdown parsing.
    - Auto‑scroll to the last received/sent message.
  - Chat sessions:
    - "New" button starts a new conversation (asks if you want to save the current conversation to history, with an automatic title).
    - "History" button lists and restores saved conversations (per open file), keeping the AI context.
  - Continuous context: new sends include the full message history (system/user/assistant) for better continuity.

Persistence/Settings (QSettings):
- `ai/provider`: `openai` | `generativa` | `ollama` | `openrouter` (default: `openai`)
- `ai/base_url`: base URL override (optional)
- `ai/api_key`: provider secret token (not applicable to local Ollama)
- `ai/model`: model name (e.g., `gpt-4o-mini`, `llama3`, `gpt-4o-mini-transcribe`)
- `ai/prompts/synonyms`, `ai/prompts/summaries`, `ai/prompts/explanations`, `ai/prompts/chat`

Privacy notes:
- Before sending any content to the AI, the application asks for your confirmation.
- Tokens are stored in user preferences (`QSettings`).

For a step‑by‑step guide with images and tips, see [TUTORIAL.en-GB.md](TUTORIAL.en-GB.md).

## RAG (Experimental)

WARNING: This feature is under testing and may cause resource overload on some machines, leading to freezes or early termination of the application/terminal. We are stabilising the pipeline with staged processing and consumption limits.

### What it is
- Indexing PDFs into vectors to enable semantic search and answers with context.
- 100% C++ pipelines (no Python on the client), with embedding providers over HTTP:
  - `openai`, `generativa` (OpenAI‑compatible) and `ollama` (local).
- Local vector storage on disk (simple binary format + JSONs for ids/metadata) at `~/.cache/br.tec.rapport.genai-reader/`.

### How to use
1. Open a PDF in the reader.
2. Right‑click within the PDF and choose `Recreate document embeddings...`.
3. Follow the progress dialogue (stages, percentages, metrics). If configured in stages, run again to continue from where you left off.

### Configuration (Settings > Embeddings)
- Provider: `OpenAI`, `GenerAtiva` (Base URL and API Key) or local `Ollama`.
- Embedding model (e.g., `text-embedding-3-small`, `nomic-embed-text:latest`).
- Store (cache directory): default `~/.cache/br.tec.rapport.genai-reader`.
- Tuning:
  - Chunk size (default 1000)
  - Chunk overlap (default 200)
  - Batch size (default 16)
  - Pages per stage (optional; process N pages per run to avoid exhaustion)
  - Pause between batches (ms) (optional; insert a pause between batches)

Suggested safe values:
- Pages per stage: 10–25
- Pause between batches: 100–250 ms
- Batch: 8–16
- Chunk size: 800–1200
- Overlap: 100–250

### Text extraction dependencies (recommended)
- Preferred: `poppler-utils` (provides `pdftotext` and `pdftoppm`) — faster and with lower memory usage.
- Fallback: `tesseract-ocr` (per‑page OCR; heavier and slower, use only when necessary).
- The app auto‑detects and warns if they are missing, suggesting installation.

### Current state and limitations
- Experimental: CPU/RAM overload and application/terminal termination may occur on large documents.
- Mitigations implemented:
  - Staged processing (stops after N pages; run again to continue).
  - Incremental writing of vectors/ids/metadata (no massive in‑memory buffers).
  - Throttling between batches (configurable pause) and respect for cancellation.
  - Logs of stages, metrics and progress in the indexing window.
- Next improvements:
  - Dependency diagnostics (status of `pdftotext`, `pdftoppm`, `tesseract`) in the UI.
  - Additional fallbacks and I/O optimisations for metadata.

## Upcoming versions
- Ongoing planning in `ROADMAP.md`.

#### Building from source (Linux)
Prerequisites: CMake (>=3.16), C++17 compiler, Qt6 (Widgets, PdfWidgets, Network), Doxygen (optional).

```bash
cmake -S . -B build
cmake --build build -j
# Executable: build/bin/genai_reader

# Generate documentation
cmake --build build --target docs
# or
doxygen docs/Doxyfile
# Open: docs/index.html
```

## How to create a GitHub release

Prerequisites:
- Have the remote repository configured (origin) and push access.
- Optional: authenticated GitHub CLI (`gh`): `gh auth login`.

Suggested steps:

```bash
# 1) Ensure CHANGELOG.md and README.md are updated (e.g., 0.1.9)
git add CHANGELOG.md README.md
git commit -m "docs: update changelog and readme for v0.1.9"

# 2) Version in code if applicable (CMakeLists.txt, headers) and commit
# Example (if there was a build version change)
# git add CMakeLists.txt include/app/App.h
# git commit -m "chore(release): bump version to v0.1.9"

# 3) Create an annotated tag and push it
git tag -a v0.1.9 -m "v0.1.9"
git push origin v0.1.9

# 4A) Create release via GitHub CLI (attaching CHANGELOG notes)
gh release create v0.1.9 \
  --title "v0.1.9" \
  --notes "See CHANGELOG.md for details of this release."

# 4B) Alternative: create release via GitHub UI
# - Go to Releases > Draft a new release > Choose tag v0.1.9 > Fill in title/notes > Publish

# 5) (Optional) Attach binaries
# If you have artefacts in dist/, attach with:
# gh release upload v0.1.9 dist/genai_reader-v0.1.9-linux-x86_64 dist/genai_reader-v0.1.9-linux-x86_64.tar.gz
```

Note: the project requires Qt6 (Widgets, PdfWidgets, Network). Without Qt6 the application will not build.

## Build dependencies by platform

### Linux
- General:
  - C++17 compiler (g++/clang), CMake (>= 3.16)
  - Qt6 Widgets + Qt6 PdfWidgets + Qt6 Network
  - Doxygen (optional) for documentation

- Debian/Ubuntu (24.04+ suggested):
  ```bash
  sudo apt update
  sudo apt install -y build-essential cmake ninja-build \
      qt6-base-dev qt6-tools-dev qt6-tools-dev-tools \
      qt6-pdf-dev doxygen graphviz
  # Alternative (Poppler instead of Qt PDF):
  # sudo apt install -y libpoppler-qt6-dev
  ```

- Fedora
  ```bash
  sudo dnf install -y gcc-c++ cmake ninja-build \
      qt6-qtbase-devel qt6-qttools-devel qt6-qttools \
      qt6-qtpdf-devel doxygen graphviz
  # Poppler-Qt6 alternative: sudo dnf install -y poppler-qt6-devel
  ```

- Arch/Manjaro
  ```bash
  sudo pacman -S --needed base-devel cmake ninja \
      qt6-base qt6-tools qt6-pdf doxygen graphviz
  # Poppler-Qt6 alternative: sudo pacman -S poppler-qt6
  ```

Notes:
- If your distro does not have a Qt PDF package, use the corresponding Poppler-Qt6.
- If the `QPdfBookmarkModel` include is not available, the PDF TOC will automatically fall back to a page listing.

### Windows
- Option A: Visual Studio + Qt (recommended)
  1) Install Visual Studio (Community) or "Build Tools for Visual Studio" with MSVC C++.
  2) Install Qt via the Qt Online Installer (same compiler/version as MSVC). Include Qt Widgets and Qt PDF.
  3) Install CMake and Ninja (optional, improves builds):
     - CMake: https://cmake.org/download/
     - Ninja: https://github.com/ninja-build/ninja/releases
  4) Set `CMAKE_PREFIX_PATH` pointing to `.../Qt/<version>/<kit>/lib/cmake` (Qt6):
     - Example: `set CMAKE_PREFIX_PATH=C:\\Qt\\6.7.0\\msvc2022_64\\lib\\cmake`
  5) Generate and build:
  ```bat
  cmake -S . -B build -G "Ninja"
  cmake --build build -j
  ```

- Option B: MSYS2/MinGW
  1) Install MSYS2 and update it.
  2) Install toolchain and Qt:
  ```bash
  pacman -S --needed mingw-w64-ucrt-x86_64-toolchain \
      mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja \
      mingw-w64-ucrt-x86_64-qt6-base mingw-w64-ucrt-x86_64-qt6-tools \
      mingw-w64-ucrt-x86_64-qt6-pdf doxygen graphviz
  ```
  3) Open the corresponding UCRT64/MinGW shell and build with CMake.

## Reader Profile (optional)
- The application includes a "Reader data" dialogue (`File > Reader > Reader data...`) to record user preferences.
- All fields are optional and can be used to personalise interactions:
  - Name
  - E‑mail
  - WhatsApp
  - Nickname (used by LLMs to address you when appropriate)

Notes:
- No data is sent externally by the app. Data is stored locally via `QSettings`.
- If a "Nickname" is provided, it will be considered in the system instructions sent to LLMs to personalise addressing.

## Contributing
Contributions are very welcome! You can help in several ways:
- Reporting bugs and opening issues with improvement suggestions.
- Implementing MVP features according to `REQUISITOS.md`.
- Proposing UX, documentation or example improvements.

Suggested steps:
1. Fork the repository.
2. Create a branch for your feature or fix: `git checkout -b feat/feature-name`.
3. Small, focused commits with clear messages.
4. Open a Pull Request describing the problem/solution and referencing affected RF/RNF/CA.

## Code of Conduct
Be respectful and collaborative. Abusive behaviour will not be tolerated. Use courteous and empathetic language in interactions.

## Licence
This project is licensed under the Creative Commons Attribution 4.0 International (CC BY 4.0) licence. See the [`LICENSE`](LICENSE) file.

You are free to:
- Share — copy and redistribute the material in any medium or format.
- Adapt — remix, transform, and build upon the material for any purpose, even commercially.

As long as you give appropriate credit and indicate if changes were made. See the full terms at:
- https://creativecommons.org/licenses/by/4.0/

## Acknowledgements
- Qt community and document reading libraries (Poppler/Qt PDF, etc.).
- Calibre project for integration inspiration.
- Contributors and testers who help improve the project.
