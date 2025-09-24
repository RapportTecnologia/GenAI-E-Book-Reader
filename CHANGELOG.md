# Changelog

  All notable changes to this project will be documented in this file.

  The format is based on Keep a Changelog and this project adheres to Semantic Versioning.

  ## [Unreleased]


  - Melhorias na integração com WSL no windows

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

### Added/Planned
{{ ... }}

### Fixed
- Problemas de exibição de tabelas Markdown no chat.
- Ordem de renderização para garantir MathJax consistente após parsing e highlighting.

  [Unreleased]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/compare/v0.1.11...HEAD
  [0.1.11]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.11
  [0.1.10]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.10
  [0.1.9]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.9
  [0.1.8]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.8
  [0.1.7]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.7
  [0.1.6]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.6
  [0.1.5]: https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/tag/v0.1.5
