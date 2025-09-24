# Changelog

All notable changes to this project will be documented in this file.

The format is based on Keep a Changelog and this project adheres to Semantic Versioning.

## [Unreleased]


- Melhorias na integração com WSL no windows

## [0.1.11]

- Ampliado a documentação do código.
- Melhorado a detecção de dados como título, autor, editor, etc. relativos a metadados.
- Usando o nome do arquivo para construir os metadados do e-book.
- Interação com a IA Generativa para obter mais dados sobre o livro.
- Obtenção de dados para OPF junto ao Google Livros.
- Contabilidade do tempo gasto para gerar Indice de Embeddings
- Checagem se é a ultima versão, se não for, exibe uma janela com a versão mais recente e pergunta se deseja fazer o download.
- Pergunta ao usuário se ele deseja associar o leitor aos arquivos PDFs
- Recurso de impressão adicionado:
  - Menu Arquivo > Documento > "Imprimir documento..." envia o PDF atual para a impressora do sistema (Linux `lp`).
  - Clique direito no leitor de PDF oferece "Imprimir página atual..." (imprime a página visível).
  - Chat: botão "Imprimir" acima da caixa de texto e opção no clique direito para imprimir a transcrição (gera PDF temporário e envia para `lp`).
  
## [0.1.10] - 2024-09-23

### Focus
- Melhoria nas buscas (full‑text e semântica) e na interação do chat com suporte a Function Calling, quando disponível no modelo/provedor.

### Added/Planned
- Indicador de capacidade de Function Calling na janela de Configurações de LLM: uma caixa de texto somente leitura exibirá se o provedor/modelo selecionado suporta Function Calling (detecção automática quando a API expuser essa informação; caso contrário, texto informativo com instruções).
- Documentação atualizada descrevendo o comportamento esperado de Function Calling por provedor/modelo e as limitações conhecidas.

### Notes
- Esta versão é focada em preparar a infraestrutura e UX para Function Calling e em aprimorar a qualidade das pesquisas (texto e semântica). Implementações adicionais podem ser entregues de forma incremental ao longo do ciclo 0.1.10.

## [0.1.9] - 2025-09-22

- Ajuste no CI para refletir os ajustes em CMakeList.txt
- Automatizado o ajuste de versão com base no primeiro nota de versão
- Adicionado novos provedores de LLM: Ollama, GenerAtive AI e OpenRouter.
- Melhorias na interface de configuração de LLM. listagem de modelos e teste de modelo.
- Adicionado ao CMakeList.txt um target para fazer release local para testes.
- Aumentado a depuração no processo de seleção de Provedores de IA e LLMs
- Chave de cortesia para uso do OpenRouter.ia adicionada
- Removido a dependência do PHPList e personalizado a interação com a LLM
- Personalizado a interação com o usuário.
- Iniciado estudos de envio do e-book para resumo pela IA.
- Adicionado suporte a arquivos OPF.

## [0.1.8] - 2025-09-21

- Compilação para o Windows, ainda em implementação, Ajuda será bem vinda.
- Ajuste na documentação, em especial tutorial e arquivo readme.
- Adicionado ao menu help o conteúdo do arquivo TUTORIAL.md
- Melhorias na seleção de página e sincronismo do e-book com o TOC e seletor de página na barra de ferramentas. 
- Sincronizado o TOC e a página selecionada na barra de ferramentas com a página exibida do e-book.
- Revisão do menu "Arquivos Recentes" e da janela de listagem.
- Iniciado o estudo para implementar a funcionalidade do menu/conteudo/indice/sumário do e-book.
- Melhorado gestão de chat por arquivos.
- Adoção do Metadados para AppImage. Melhorias relativas no CMakeFile.txt

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

[Unreleased]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/compare/v0.1.10...HEAD
[0.1.10]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/compare/v0.1.9...v0.1.10
[0.1.9]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.9
[0.1.8]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.8
[0.1.7]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.7
[0.1.6]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.6
[0.1.5]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.5
