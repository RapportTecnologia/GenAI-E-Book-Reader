# Changelog

All notable changes to this project will be documented in this file.

The format is based on Keep a Changelog and this project adheres to Semantic Versioning.

## [Unreleased]
- Compilação para o Windows, ainda em implementação, Ajuda será bem vinda. 

## [0.1.7] - 2025-09-20

### Added
- Ajustado painel de Informações da Aplicação
- Carga de e-books pela linha de comando ou menu no explorador (Abrir Com).
- Associação de extenção com o applicativo.
- Dicionário iniciado, funcionando apenas com LLM. preciso testar com APIs especializadas.

## [0.1.6] - 2025-09-18

### Added
- Indexação de embeddings por documento com controle de chunking/batching e escrita incremental. Issue #3
- Busca por frases usando embeddings (fallback semântico quando a busca por texto não encontra resultados), com opções rápidas: métrica (cosine/dot/L2), Top‑K e limiares. Issue #3
- Botão de Título central na barra superior exibindo o nome do documento atual e menu contextual com ações: abrir diretório, adicionar ao Calibre com migração de embeddings e renomear arquivo com migração. Issues #6, #7, #8


### Changed
- Otimizada a renderização do painel de chat, movendo a conversão de Markdown do front-end (JavaScript, `marked.js`) para o back-end (C++ com a biblioteca `cmark`). Isso simplifica o código do cliente e melhora a manutenibilidade.
- Barra de busca unificada na toolbar com campo de pesquisa, botões Anterior/Próximo e menu de Opções para a busca semântica.
- Ajuste na exibição dos Desenvolvedores
- Ajuste na exibição da licença

### Fixed
- Corrigidos travamentos e erros de renderização (`Uncaught TypeError`) do MathJax que ocorriam durante a atualização do conteúdo do chat. (Resolve Issue #1)

## [0.1.5] - 2025-09-14

### Added
- Integração com LLM (OpenAI/GenerAtiva) com diálogo de Configurações de LLM:
  - Provedor, Modelo, Base URL (opcional), API Key e prompts personalizáveis para Sinônimos, Resumos, Explicações e Chat.
- Painel de Chat com IA (dock à direita):
  - Enviar mensagens, salvar a conversa em `.txt` e solicitar um compilado (resumo) do diálogo atual.
- Ações de IA no leitor:
  - Sinônimos e Resumo a partir da seleção, com confirmação prévia antes do envio do conteúdo.
- Painel de Chat com renderização avançada:
  - Suporte a Markdown GFM com tabelas corretamente renderizadas (bordas, cabeçalho e rolagem horizontal quando necessário).
  - Syntax highlighting para blocos de código via highlight.js (tema GitHub).
  - MathJax v3 configurado para inline/display LaTeX; typeset após o Markdown e highlighting.
  - Auto-scroll para a última mensagem após cada atualização do chat.
- Sessões de chat:
  - Botão "Novo" para iniciar uma nova conversa (com confirmação para salvar a conversa atual).
  - Botão "Histórico" para listar e restaurar conversas salvas.
  - Título automático de cada conversa salva (baseado na última resposta da IA ou primeira mensagem do usuário).
  - Persistência por arquivo em `QSettings` como lista JSON (título, HTML e turns role/content).
- Contexto contínuo com a IA: envios passam a incluir todo o histórico da conversa para melhor continuidade.

### Changed
- Documentação: `README.md` ganhou seção de LLM e link para `TUTORIAL.md` com passo a passo.
- Removido o fallback de QTextEdit no Chat: `QWebEngineView` passa a ser obrigatório, garantindo Markdown/MathJax/highlighting consistentes.

### Fixed
- Problemas de exibição de tabelas Markdown no chat.
- Ordem de renderização para garantir MathJax consistente após parsing e highlighting.


---

## [0.1.4] - 2025-09-14

### Added
- Preferência de granularidade do zoom (Ctrl+roda) configurável em Configurações.
- Modos de seleção no PDF: texto e retângulo (imagem), com cópia exibindo toast de confirmação.
- Exportação de seleção para TXT e Markdown.
- Diálogo "Dados do leitor" com envio opcional para PHPList via `.env`.
- "Recentes": novo diálogo rolável com busca e filtro.
  - Persiste metadados dos PDFs abertos (caminho, título, autor, resumo/subject, palavras‑chave) em `QSettings`.
  - Busca por trecho do nome do arquivo, título, autor, resumo e palavras‑chave.
  - Submenu `Arquivo > Documento > Recentes` mostra últimos itens com título e nome do arquivo.

### Changed
- Documentação atualizada (`README.md`, `REQUIREMENTS.md`, `PLANO-DE-DESENVOLVIMENTO.md`, `ROADMAP.md`) para refletir Qt6 obrigatório e estado atual (suporte a PDF).
- Esclarecida a dependência de Qt6 (Widgets, PdfWidgets, Network) no guia de build.

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

[0.1.6]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.6
[0.1.5]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.5
[0.1.4]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.4
[0.1.3]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.3
[0.1.2]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.2
[0.1.1]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.1
[0.1.0]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.0
[0.0.10]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.0.10

