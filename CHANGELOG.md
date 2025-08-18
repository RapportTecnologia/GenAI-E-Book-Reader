# Changelog

All notable changes to this project will be documented in this file.

The format is based on Keep a Changelog and this project adheres to Semantic Versioning.

## [Unreleased]

---

## [0.1.3] - 2025-08-18
### Added
- Ícone do aplicativo a partir de `docs/imgs/logo-do-projeto.png` (RF-29), aplicado globalmente.
- Splash screen exibindo logo, versão e autor (RF-31).
- Painel de Sumário (TOC) remodelado com barra de ferramentas própria no topo.
  - Ações para alternar entre modos: lista de páginas ("Páginas") e árvore de capítulos ("Conteúdo").
  - Botões de navegação que agem por página (modo Páginas) ou por item/capítulo (modo Conteúdo).

### Changed
- Barra de título agora mostra o nome do livro em leitura. Para PDFs no Qt6, usa metadados Title quando disponíveis; caso contrário, usa o nome do arquivo (RF-30).
- Ajustes de layout para que a área de leitura ocupe 100% do espaço disponível.
- Tamanho padrão do divisor (splitter) aplicando ~10% da largura para o painel de TOC e ~90% para o visualizador na primeira execução (sem layout salvo).

---

## [0.1.2] - 2025-08-18
### Added
- "Salvar como" (RF-28): salvar o livro/estado atual em um novo arquivo via menu Arquivo e toolbar.

### Changed
- Pequenos aprimoramentos de leitura e usabilidade.

---

## [0.1.1] - 2025-08-17
### Added
- Combobox de seleção de página na toolbar para navegação direta.
- Restauração de sessão: reabre automaticamente o último arquivo e lembra o último diretório.

### Changed
- Ajuste do PDF para abrir com zoom adequado (base para "fit to width").
- Persistência via `QSettings` de geometria, estado da UI, tema e zoom.
- Correções de compilação e sincronização de cabeçalhos (`MainWindow.h/.cpp`).
 - TOC e combobox de páginas agora contemplam todas as páginas de PDFs.

---

## [0.1.0] - 2025-08-17
### Added
- Planejamento do MVP 0.1.0 conforme `ROADMAP.md` (Leitura básica: abrir PDF/EPUB, navegação, tema claro/escuro, sumário, preferências mínimas).

### Changed
- Bump de versão para 0.1.0 em `CMakeLists.txt`, `include/app/App.h` e badge no `README.md`.
- Documentação revisada para apontar o alvo do MVP (0.1.0) e alinhar com o plano.
 - Revisado `REQUISITOS.md`: correções ortográficas e remoção de subseção duplicada de "Leitura Multiformato" (RF-01..RF-03).

### Notes
- Esta release foca em alinhar a base do projeto e a documentação para iniciar a implementação do MVP de leitura básica.

## [0.0.10] - 2025-08-17
### Added
- Initial project scaffolding (CMake, C++17, optional Qt Widgets placeholder).
- Doxygen documentation pipeline with custom theme and sidebar/search enabled.
- Project logo and visual polish for docs (`docs/assets/customdox.css`).
- Documentation sources included in site: `README.md`, `REQUISITOS.md`, `PLANO-DE-DESENVOLVIMENTO.md`.
- New docs: `CHANGELOG.md`, `ROADMAP.md`.

### Changed
- Set project version to 0.0.10 in `CMakeLists.txt`, `README.md` and headers.
- Improved Doxyfile to avoid copy-to-self issues and keep default header.

### Known Issues
- If Qt is not installed, a console placeholder is built (see `CMakeLists.txt` message).
- One unresolved reference in `README.md` badge/anchor noted by Doxygen (harmless).

[Unreleased]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/compare/v0.1.3...HEAD
[0.1.3]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.3
[0.1.2]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.2
[0.1.1]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.1
[0.1.0]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.0
[0.0.10]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.0.10

