# Changelog

   The format is based on Keep a Changelog and this project adheres to Semantic Versioning.

   ## [Unreleased]

   - Melhorias na integração com WSL no windows

   ## [0.1.13] - 2025-09-27

   - Manter o histórico de navegação entre as páginas, permitindo voltar e avançar.
   - Revisão a interação via chat pois está muito pobre, já esteve melhor.
   - Corrigir o fluxo de versão, pois houve um erro ao liberar a versão 0.1.12 que foi com as alterações da versão 0.1.13
   - Corrigido o cmake para atualizar a versão da aplicação, infelizmente ainda força a recompilação de todo o código. 
  - Renomeados os botões de navegação de página para deixar mais claros:
    - "Próxima Página" (antes: "Próxima")
    - "Página Anterior" (antes: "Anterior")


   ## [0.1.12] - 2025-09-26

  - Chat/OPF:
  - OPF agora é usado apenas para identificação mínima do livro no chat (arquivo, título, autor, ISBN).
  - Removida a exposição da ferramenta `query_opf` no Function Calling por padrão, evitando latência e ruído durante a conversa. O handler interno permanece para compatibilidade.
  - Corrigido o schema da ferramenta `propose_search` (faltava declarar `properties.query`).
  - Revisão do processo de criar um novo chat, agora ele pergunta se deseja salvar o chat atual no histórico. E inicia um novo esquecendo o anterior.
  - Adicionar spiner para indicar que a IA está processando a requisição.
  - Adicionado opção de tradução e sinônimos do texto marcado.
  - Corrigido o BUG de listagem dos modelos da OpenRouter.AI
  - Revisar o Algorítimo de OPF, correções e melhorias.
  - Removido o BUG relativo a seleção do Ollama
  - Ajuste no tamanho da janela Recriando embeddings para melhor visualização.
  - Suporte ao provedor Perplexity nos engines de LLM:
    - Novo provedor `perplexity` em Configurações de LLM, com seleção de modelos recomendados (`sonar-small-online`, `sonar-medium-online`, etc.).
    - Uso do endpoint compatível com OpenAI em `https://api.perplexity.ai/chat/completions`.
    - Leitura da chave via `QSettings` (ai/api_keys/perplexity) com fallback à env `PERPLEXITY_API_KEY`.
  - Chat com pesquisa (web-enabled) via Perplexity:
    - Permite escolher modelos “online” para respostas com buscas na web.
  - Consulta estruturada para OPF (Function Calling):
    - Nova ferramenta `query_opf` disponível no chat para retornar campos do OPF atual de forma estruturada (JSON).
    - Campos suportados: `title`, `author`, `publisher`, `language`, `identifier`, `description`, `summary`, `keywords`, `edition`, `source`, `format`, `isbn`.
  - Agora pode escolher em qual idioma se quer que a IA responda.
  - Revisar a passagem dos metadados do e-book para o chat.
  - Informa matadados do livro no prompt de sistema, melhorando a contextualização das consultas.


  ## [0.1.11] - 2025-09-24

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


  [Unreleased]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/compare/v0.1.13...HEAD
  [0.1.13]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.13
  [0.1.12]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.12
  [0.1.11]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.11
  [0.1.10]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.10
  [0.1.9]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.9
  [0.1.8]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.8
  [0.1.7]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.7
  [0.1.6]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.6
  [0.1.5]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.5
