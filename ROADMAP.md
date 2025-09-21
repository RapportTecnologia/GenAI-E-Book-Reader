# Roadmap

Esta é uma visão de alto nível das entregas previstas. Ajuste conforme evoluírem requisitos e prioridades.


## 0.0.1 – Bootstrap
- Infra CMake e esqueleto C++17
- Placeholder com/sem Qt (Widgets)
- Pipeline de documentação com Doxygen (tema, sidebar, busca)
- Logo e estilo custom de docs

## 0.1.0 – MVP Leitura Básica (Entregue na v0.1.0)
- Abrir PDF/EPUB (biblioteca: Qt PDF/Poppler)
- Navegação básica (páginas, zoom, modo claro/escuro)
- Barra lateral de sumário
- Preferências mínimas persistidas
  
Resumo: abertura de PDF/EPUB, navegação (anterior/próxima, seleção direta por combobox), sumário lateral, persistência via QSettings (tema, zoom, geometria/estado), restauração de sessão (último arquivo/diretório). Para PDFs, o TOC usa bookmarks (capítulos/subcapítulos) quando disponíveis; na ausência, lista todas as páginas.

## 0.1.2 – Refinos de Leitura (Entregue em 2025-08-18)
- "Salvar como" (RF-28)
- Pequenos aprimoramentos de usabilidade

## 0.1.3 - Refinamento da interface de usuário (Entregue em 2025-08-18)
- Ícone do app a partir de `docs/imgs/logo-do-projeto.png` aplicado globalmente.
- Barra de título exibe o nome do livro em leitura (metadados quando disponíveis; senão, nome do arquivo).
- Ajuste de layout para área de leitura ocupar 100% do espaço do frame.
- Splash screen com `docs/imgs/logo-do-projeto.png`, versão e autor: Carlos Delfino <consultoria@carlosdelfino.eti.br>.
- Painel de TOC com barra de ferramentas para alternar entre "Páginas" e "Conteúdo" (capítulos).
- Botões "Voltar" e "Avançar" que respeitam o modo selecionado (página anterior/próxima ou item/capítulo anterior/próximo).
- Tamanho padrão do splitter configurado para ~10% (TOC) e ~90% (visualizador) na primeira execução.
- Preferência de granularidade de zoom via Ctrl+roda (configurável em Configurações).
- Modos de seleção (texto/retângulo), cópia com toast e exportação de seleção para TXT/Markdown.
- Diálogo "Dados do leitor" com envio opcional para PHPList via `.env`.

## 0.1.4 – Aperfeiçoamentos de leitura e integração (Entregue em 2025-09-14)
- Preferência de granularidade de zoom (Ctrl+roda) configurável em Configurações.
- Modos de seleção (texto/retângulo) com cópia e toast de confirmação.
- Exportação de seleção para TXT/Markdown.
- Documentação revisada para Qt6 obrigatório e estado atual (PDF).
 - "Recentes": diálogo rolável com busca e filtro por nome/título/autor/resumo/palavras‑chave e submenu com últimos itens.
   - Persistência de metadados (caminho, título, autor, resumo, palavras‑chave) em `QSettings`.

## 0.1.5 - Itengração IA
- Integração com APIs de IA (pré cadastradas: OpenAI, GenerAtiva, Claude, Anthropic, Google, DeepSeek)
- Resumo e explicação de trechos selecionados
- Histórico de prompts e respostas
- Chat com IA (OpenAI) para explicar e resumir seleções.

## 0.1.6 – RAG, MathJax e Markdown
- RAG :
  - Risco conhecido: sobrecarga de recursos em documentos grandes causando encerramento prematuro; mitigado por etapas/pausas
  - UI para ajuste fino de chunking/batching (chunk size, overlap, batch size)
  - Execução por etapas (pages per stage) e pausa entre lotes (throttling)
  - Detecção de dependências de extração (`pdftotext`, `pdftoppm`, `tesseract`) e fallbacks
  - Métricas e logs de estágios no diálogo de indexação
- Otimizada a renderização do chat movendo a conversão de Markdown para o back-end (C++ com `cmark`).
- Corrigidos bugs de renderização e travamentos relacionados ao MathJax no painel de chat.
- Busca full-text no documento

## 0.1.7 – Anotações e Dicionário
- Empacotamento (AppImage)
- Ajustado painel de Informações da Aplicação
- Carga de e-books pela linha de comando ou menu no explorador (Abrir Com).
- Associação de extenção com o applicativo.
- Dicionário iniciado, funcionando apenas com LLM. preciso testar com APIs especializadas.
- Adicionado Target ao cmakefile.txt para gerar Release no GitHub


## 0.1.8 – TTS e Controles
- Compilação para o Windows, ainda em implementação, Ajuda será bem vinda.
- Ajuste na documentação, em especial tutorial e arquivo readme.
- Adicionado ao menu help o conteúdo do arquivo TUTORIAL.md
- Melhorias na seleção de página e sincronismo do e-book com o TOC e seletor de página na barra de ferramentas. 
- Sincronizado o TOC e a página selecionada na barra de ferramentas com a página exibida do e-book.
- Revisão do menu "Arquivos Recentes" e da janela de listagem.
- Iniciado o estudo para implementar a funcionalidade do menu/conteudo/indice/sumário do e-book.
- Melhorado gestão de chat por arquivos.
- Adoção do Metadados para AppImage. Melhorias relativas no CMakeFile.txt
- Documentação de uso e limitações
- Escrita incremental de vetores/ids/metadados

## 0.1.9 – Ajustes
- Ajuste no CI para refletir os ajustes em CMakeList.txt
- Automatizado o ajuste de versão com base no primeiro nota de versão
- Corrigir as configurações para acesso a GenAI.
- Adicionar Opção de uso do Ollama localmente em http://localhost:11434
- Adicionar OPção de uso do OpenRouter em https://openrouter.ai

## 0.2.0 – TTS e Dicionario
- Reestruturar o MainWindows para reduzir seu tamanho, aplicando fachadas para agrupar funções por contextos gráficos.
- Continuar o desenvolvimento: Dicionário on-click (mínimo 1 idioma), PT-BR e EN-US, AR-EG, AR-SA, AR-YE, AR-DZ, AR-KW, AR-LB, AR-LY, AR-MA, AR-OM, AR-QA, AR-SA, AR-SY, AR-TN, AR-AE, AR-IL, AR-JO, AR-LB, AR-PS, AR-SY, AR-YE, Chines,


## 0.2.1 – Reestruturação do código
- Reprodução TTS de seleção atual
- Controles de velocidade/pausa/retomar
- Output device selection

## 0.5.0 – Estatísticas e Sessões
- Tempo ativo de leitura, progresso, sessões
- Restauração de sessão (últimos livros abertos)
- Export de métricas

## 0.6.0 – Integração Calibre e Melhorias UX
- Abrir itens da biblioteca do Calibre
- Melhorias de performance e usabilidade

## 1.x – Estabilidade e Releases
- Testes automatizados
- Internacionalização (i18n)

## Backlog Técnico
- CI (build + testes)
- Lint/format (clang-format, cmake-format)
- Benchmarks de render de páginas
