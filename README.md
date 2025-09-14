![Visitantes do Projeto](https://visitor-badge.laobi.icu/badge?page_id=rapporttecnologia.genai-e-book-reader)
[![License: CC BY 4.0](https://img.shields.io/badge/License-CC%20BY%204.0-lightgrey.svg)](LICENSE)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue)
![Qt](https://img.shields.io/badge/Qt6-Widgets-brightgreen)
![CMake](https://img.shields.io/badge/CMake-%3E%3D3.16-informational)
[![Docs](https://img.shields.io/badge/docs-Doxygen-blueviolet)](docs/index.html)
![Version](https://img.shields.io/badge/version-0.1.4-blue)
[![Contributions Welcome](https://img.shields.io/badge/contributions-welcome-success.svg)](#contribuindo)


# GenAI E-Book Reader

Leitor de e-books moderno com foco em produtividade e estudo, desenvolvido em C/C++ com Qt6, com recursos planejados de anotações, dicionário, Text-to-Speech (TTS), estatísticas de leitura e apoio de IA (RAG) para resumos e explicações.

- Requisitos e escopo completos: consulte [REQUIREMENTS.md](REQUIREMENTS.md).
- Plano do projeto (fases, sprints e critérios): consulte [PLANO-DE-DESENVOLVIMENTO.md](PLANO-DE-DESENVOLVIMENTO.md).
- Histórico de mudanças: consulte [CHANGELOG.md](CHANGELOG.md).
- Planejamento de releases: consulte [ROADMAP.md](ROADMAP.md).

## Principais Recursos (MVP)
- Leitura de PDF com navegação básica e tema claro/escuro. (Suporte a EPUB/MOBI está no roadmap.)
- Marcações e anotações com painel lateral e exportação JSON.
- Dicionário on-click (mínimo 1 idioma).
- TTS para trechos selecionados, com controles básicos.
- IA (OpenAI) para resumir/explicar trechos com RAG.
- Estatísticas: tempo ativo, progresso, sessões.
- Restauração de sessão (reabre livros abertos anteriormente).
- Integração com Calibre para abrir livros da biblioteca.
- Painel de Sumário (TOC) com barra de ferramentas:
  - Alternar entre lista de páginas ("Páginas") e árvore de capítulos ("Conteúdo").
  - Botões "Voltar" e "Avançar" que navegam página a página (modo Páginas) ou por item/capítulo (modo Conteúdo).
  - Largura padrão do painel ~10% e área do leitor ~90% na primeira execução (pode ser ajustado via splitter).

Nota (0.1.3): refinamento de UI — ícone do app a partir de `docs/imgs/logo-do-projeto.png`, splash screen com versão/autor e título da janela exibindo o nome do livro (metadados quando disponíveis; senão, nome do arquivo). Ajuste para a área de leitura ocupar 100% do espaço disponível. Painel de TOC remodelado com barra de ferramentas (alternar entre "Páginas"/"Conteúdo" e botões de navegação) e tamanho padrão inicial do splitter ~10% (TOC) / ~90% (visualizador).

Nota (0.1.2): implementado "Salvar como" (RF-28) e pequenos aprimoramentos de leitura.

Nota (0.1.1): adicionados seleção de página via combobox e restauração do último arquivo/diretório aberto.

Observação: para PDFs, o sumário (TOC) usa bookmarks (capítulos/subcapítulos) quando disponíveis; na ausência, lista todas as páginas. A seleção por combobox contempla todas as páginas do documento. O painel de TOC inclui uma barra de ferramentas para alternar entre "Páginas" e "Conteúdo" e botões de navegação; por padrão o painel ocupa ~10% da largura da janela na primeira execução. Atualmente, o binário suporta apenas arquivos PDF.

## Sumário (TOC) e Navegação
- Alternar modo do TOC:
  - "Páginas": lista plana de páginas; os botões "Voltar/Avançar" mudam a página atual.
  - "Conteúdo": capítulos (grupos de páginas); os botões percorrem o item anterior/próximo (capítulo ou página filho). 
- Atalhos do painel: arraste o divisor para redimensionar; o tamanho fica salvo para as próximas sessões.

## Nota (0.1.4)

- Preferência de granularidade do zoom (Ctrl+roda) configurável em Configurações.
- Modos de seleção no PDF: texto e retângulo (imagem); cópia com toast e exportação de seleção para TXT/Markdown.
- Diálogo "Dados do leitor" com envio opcional para PHPList via `.env`.
- "Recentes": novo diálogo rolável com busca e filtro, acessível em `Arquivo > Documento > Recentes > Mostrar todos...`.
  - Persiste metadados de PDFs abertos (caminho, título, autor, resumo/subject, palavras‑chave) para facilitar a pesquisa.
  - Busca por trecho do nome do arquivo, título, autor, resumo e palavras‑chave.

### Preferências de Zoom e Seleção
- Granularidade do zoom via roda do mouse com Ctrl: configurável em `Configurações > Granularidade do zoom (Ctrl+roda)`. Valor padrão: 1.10 por incremento.
- Modos de seleção no PDF: texto ou retângulo (imagem). A cópia exibe um toast de confirmação. Exportar seleção para TXT/Markdown disponível no menu Editar.

### Arquivos Recentes e Pesquisa
- Acesse itens recentes por `Arquivo > Documento > Recentes` (submenu) ou `Mostrar todos...` para abrir o diálogo completo.
- O diálogo de Recentes apresenta uma lista rolável com colunas "Título" e "Arquivo" e um campo de busca.
- Pesquise por trecho do nome do arquivo, título, autor, resumo (subject) ou palavras‑chave.
- Os metadados são persistidos em `QSettings` e atualizados quando você abre um PDF.

## Próximas versões
- Planejamento contínuo em `ROADMAP.md`.

## Como rodar (Linux)
Pré-requisitos: CMake (>=3.16), compilador C++17, Qt6 (Widgets, PdfWidgets, Network), Doxygen (opcional).

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

Observação: o projeto exige Qt6 (Widgets, PdfWidgets, Network). Sem Qt6 a aplicação não compila.

## Dependências de Build por Plataforma

### Linux
- Geral:
  - Compilador C++17 (g++/clang), CMake (>= 3.16)
  - Qt6 Widgets + Qt6 PdfWidgets + Qt6 Network.
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
- Se sua distro não tiver pacote de Qt PDF, use Poppler-Qt6 correspondente.
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

## Integração opcional com PHPList (.env)
- O aplicativo possui um diálogo "Dados do leitor" (`Arquivo > Leitor > Dados do leitor...`) para registrar nome, e-mail e WhatsApp.
- Opcionalmente, é possível enviar esses dados para uma lista do PHPList. Para isso, crie um arquivo `.env` na raiz do projeto ou ao lado do executável com as variáveis:

```env
PHPLIST_URL=http://seu-servidor/phplist/api/v2
PHPLIST_USER=usuario
PHPLIST_PASS=senha
```

Observações:
- As credenciais não são exibidas pela UI; o envio é sempre confirmado com o usuário.
- O `.env` é criado com valores vazios automaticamente no primeiro configure do CMake (veja `CMakeLists.txt`).

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
