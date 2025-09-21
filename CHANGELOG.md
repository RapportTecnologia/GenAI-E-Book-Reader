# Changelog

All notable changes to this project will be documented in this file.

The format is based on Keep a Changelog and this project adheres to Semantic Versioning.

## [Unreleased]

## [0.1.8] - 2025-09-21

- Compilação para o Windows, ainda em implementação, Ajuda será bem vinda. 

## [0.1.7] - 2025-09-20

### Added
- Ajustado painel de Informações da Aplicação
- Carga de e-books pela linha de comando ou menu no explorador (Abrir Com).
- Associação de extenção com o applicativo.
- Dicionário iniciado, funcionando apenas com LLM. preciso testar com APIs especializadas.
- Adicionado Target para Gerar AppImage

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

[Unreleased]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/compare/v0.1.8...HEAD
[0.1.8]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.8
[0.1.7]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.7
[0.1.6]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.6
[0.1.5]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.5
