# GenAI E-Book Reader

![Visitantes do Projeto](https://visitor-badge.laobi.icu/badge?page_id=rapporttecnologia.genai-e-book-reader)
[![License: CC BY 4.0](https://img.shields.io/badge/License-CC%20BY%204.0-lightgrey.svg)](LICENSE)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue)
![Qt](https://img.shields.io/badge/Qt-Widgets%20%7C%20QML-brightgreen)
![CMake](https://img.shields.io/badge/CMake-%3E%3D3.16-informational)
[![Docs](https://img.shields.io/badge/docs-Doxygen-blueviolet)](docs/index.html)
![Version](https://img.shields.io/badge/version-0.1.2-blue)
[![Contributions Welcome](https://img.shields.io/badge/contributions-welcome-success.svg)](#contribuindo)

Leitor de e-books moderno com foco em produtividade e estudo, desenvolvido em C/C++ com Qt, com recursos de anotações, dicionário, Text-to-Speech (TTS), estatísticas de leitura e apoio de IA (RAG) para resumos e explicações.

- Requisitos e escopo completos: consulte [REQUISITOS.md](REQUISITOS.md).
- Plano do projeto (fases, sprints e critérios): consulte [PLANO-DE-DESENVOLVIMENTO.md](PLANO-DE-DESENVOLVIMENTO.md).
- Histórico de mudanças: consulte [CHANGELOG.md](CHANGELOG.md).
- Planejamento de releases: consulte [ROADMAP.md](ROADMAP.md).

## Principais Recursos (MVP)
- Leitura de PDF/EPUB com navegação básica e tema claro/escuro.
- Marcações e anotações com painel lateral e exportação JSON.
- Dicionário on-click (mínimo 1 idioma).
- TTS para trechos selecionados, com controles básicos.
- IA (OpenAI) para resumir/explicar trechos com RAG.
- Estatísticas: tempo ativo, progresso, sessões.
- Restauração de sessão (reabre livros abertos anteriormente).
- Integração com Calibre para abrir livros da biblioteca.

Nota (0.1.2): implementado "Salvar como" (RF-28) e pequenos aprimoramentos de leitura.

Nota (0.1.1): adicionados seleção de página via combobox e restauração do último arquivo/diretório aberto.

Observação: para PDFs, o sumário (TOC) usa bookmarks (capítulos/subcapítulos) quando disponíveis; na ausência, lista todas as páginas. A seleção por combobox contempla todas as páginas do documento.

## Próximas versões
- 0.1.3 (Refinamento de UI):
  - Ícone do aplicativo a partir de `docs/imgs/logo-do-projeto.png`.
  - Barra de título exibe o nome do livro (metadados quando disponíveis, senão nome do arquivo).
  - Splash screen com logo, versão do app e autor: Carlos Delfino <consultoria@carlosdelfino.eti.br>.
- Planejamento contínuo em `ROADMAP.md`.

## Como rodar (Linux)
Pré-requisitos: CMake (>=3.16), compilador C++17, Qt5/Qt6 (Widgets), Doxygen (opcional).

```bash
cmake -S . -B build
cmake --build build -j
# Executável: build/bin/genai_reader

# Gerar documentação
cmake --build build --target docs
# ou
doxygen docs/Doxyfile
# Abrir: docs/index.html
```

Observação: se o Qt não estiver instalado, um placeholder de console será gerado. Para a UI, instale os headers de desenvolvimento do Qt (ex.: `qt6-base-dev` no Debian/Ubuntu) e recompile.

## Dependências de Build por Plataforma

### Linux
- Geral:
  - Compilador C++17 (g++/clang), CMake (>= 3.16)
  - Qt Widgets (Qt5 ou Qt6). Para PDF, o módulo Qt PDF é recomendado; Poppler-Qt é alternativa.
  - Doxygen (opcional) para documentação

- Debian/Ubuntu (24.04+ sugerido):
  ```bash
  sudo apt update
  sudo apt install -y build-essential cmake ninja-build \
      qt6-base-dev qt6-tools-dev qt6-tools-dev-tools \
      qt6-pdf-dev doxygen graphviz
  # Alternativa (Poppler em vez de Qt PDF):
  # sudo apt install -y libpoppler-qt6-dev
  ```

- Ubuntu 22.04 / Debian estáveis com Qt5:
  ```bash
  sudo apt install -y build-essential cmake ninja-build \
      qtbase5-dev qttools5-dev qttools5-dev-tools \
      qtpdf5-dev doxygen graphviz
  # Alternativa (Poppler-Qt5):
  # sudo apt install -y libpoppler-qt5-dev
  ```

- Fedora
  ```bash
  sudo dnf install -y gcc-c++ cmake ninja-build \
      qt6-qtbase-devel qt6-qttools-devel qt6-qttools \
      qt6-qtpdf-devel doxygen graphviz
  # Alternativa Poppler-Qt6: sudo dnf install -y poppler-qt6-devel
  ```

- Arch/Manjaro
  ```bash
  sudo pacman -S --needed base-devel cmake ninja \
      qt6-base qt6-tools qt6-pdf doxygen graphviz
  # Alternativa Poppler-Qt6: sudo pacman -S poppler-qt6
  ```

Notas:
- Se sua distro não tiver pacote de Qt PDF, use Poppler-Qt correspondente à sua versão do Qt.
- Caso o include de `QPdfBookmarkModel` não esteja disponível, o TOC de PDFs cairá automaticamente para a listagem de páginas.

### Windows
- Opção A: Visual Studio + Qt (recomendado)
  1) Instale Visual Studio (Community) ou "Build Tools for Visual Studio" com MSVC C++.
  2) Instale Qt via Qt Online Installer (mesmo compilador/versão do MSVC). Inclua Qt Widgets e Qt PDF.
  3) Instale CMake e Ninja (opcional, melhora builds):
     - CMake: https://cmake.org/download/
     - Ninja: https://github.com/ninja-build/ninja/releases
  4) Configure `CMAKE_PREFIX_PATH` apontando para o `.../Qt/<versão>/<kit>/lib/cmake` (Qt5 ou Qt6):
     - Ex.: `set CMAKE_PREFIX_PATH=C:\Qt\6.7.0\msvc2022_64\lib\cmake`
  5) Gere e compile:
  ```bat
  cmake -S . -B build -G "Ninja"
  cmake --build build -j
  ```

- Opção B: MSYS2/MinGW
  1) Instale MSYS2 e atualize.
  2) Instale toolchain e Qt:
  ```bash
  pacman -S --needed mingw-w64-ucrt-x86_64-toolchain \
      mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja \
      mingw-w64-ucrt-x86_64-qt6-base mingw-w64-ucrt-x86_64-qt6-tools \
      mingw-w64-ucrt-x86_64-qt6-pdf doxygen graphviz
  ```
  3) Abra o shell UCRT64/MinGW correspondente e compile com CMake.

## Contribuindo
Contribuições são muito bem-vindas! Você pode ajudar de várias formas:
- Reportando bugs e abrindo issues com sugestões de melhorias.
- Implementando funcionalidades do MVP conforme `REQUISITOS.md`.
- Propondo melhorias de UX, documentação ou exemplos.

Passos sugeridos:
1. Faça um fork do repositório.
2. Crie um branch para sua feature ou fix: `git checkout -b feat/nome-da-feature`.
3. Commits pequenos e objetivos com mensagens claras.
4. Abra um Pull Request descrevendo o problema/solução e referenciando RF/RNF/CA afetados.

## Código de Conduta
Seja respeitoso e colaborativo. Não serão tolerados comportamentos abusivos. Utilize linguagem cordial e empática nas interações.

## Licença
Este projeto é licenciado sob a licença Creative Commons Attribution 4.0 International (CC BY 4.0). Consulte o arquivo [`LICENSE`](LICENSE).

Você é livre para:
- Compartilhar — copiar e redistribuir o material em qualquer suporte ou formato.
- Adaptar — remixar, transformar e criar a partir do material para qualquer fim, inclusive comercial.

Desde que atribua o crédito apropriado e indique se mudanças foram feitas. Veja os termos completos em:
- https://creativecommons.org/licenses/by/4.0/

## Agradecimentos
- Comunidade Qt e bibliotecas de leitura de documentos (Poppler/Qt PDF, etc.).
- Projeto Calibre pela inspiração de integração.
- Contribuidores e testadores que ajudam a melhorar o projeto.
