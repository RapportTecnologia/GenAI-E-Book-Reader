# GenAI E-Book Reader — Descrição e Requisitos de Software

RF -> Requisito Funcional
RNF -> Requisito Não Funcional
CA -> Critério de Aceitação

## 1. Visão Geral
Este software visa atender a uma necessidade prática durante a leitura de e-books (PDF, MOBI e EPUB), oferecendo uma experiência simples, fluida e produtiva, com recursos de leitura, marcação, anotações, estatísticas de leitura, dicionário, Text-to-Speech (TTS) e apoio de IA para resumos e explicações via RAG.

- Plataforma principal: Linux (prioritário).
- Portabilidade: Windows (compilável em versões futuras).
- Tecnologias: C/C++ com Qt6 (Widgets, PdfWidgets, Network) para GUI, CMake para build, Doxygen para documentação.

### Estado Atual (implementação)
- Suporte de leitura: PDF com Qt6 PdfWidgets.
 - Suporte adicional: leitura de arquivos OPF para exibição de metadados básicos.
- UI e usabilidade:
  - Painel de Sumário (TOC) com alternância entre "Páginas" e "Conteúdo" (bookmarks quando disponíveis), com botões de navegação.
  - Barra superior com botão de título central exibindo o nome do documento e oferecendo menu contextual (abrir diretório, adicionar ao Calibre com migração de embeddings, renomear com migração).
  - Barra de busca com pesquisa por texto e fallback para busca semântica por frases usando embeddings; opções rápidas: métrica (cos/dot/L2), Top‑K, limiar/distance. Parâmetros de embeddings configuráveis em `Configurações > Embeddings`.
  - Preferência de granularidade do zoom via Ctrl+roda do mouse (configurável).
  - Modos de seleção: texto e retângulo (imagem), com cópia para área de transferência e exportação da seleção para TXT/Markdown. OCR opcional via Tesseract quando disponível no PATH.
  - "Salvar como" (RF-28).
  - Branding/UI (RF-48, RF-49, RF-50): ícone do app, título com nome do livro, splash screen com versão/autor.
- Perfil do leitor: diálogo "Dados do leitor" com campos opcionais (nome, e‑mail, WhatsApp e apelido) para personalização local; apelido pode ser usado pelas LLMs para tratamento personalizado.
- Integração LLM (0.1.9): provedores compatíveis com API OpenAI — `openai`, `generativa`, `ollama` (local) e `openrouter`; melhorias na UI de LLM com listagem de modelos (quando suportado) e botão de "Testar modelo"; ajustes de prompts; chaves e preferências persistidas em `QSettings`.
 - Indicador de Function Calling (0.1.10): a janela de Configurações de LLM exibirá uma caixa de texto somente leitura informando se o provedor/modelo selecionado declara suporte a Function Calling. A detecção é automática quando a API expõe metadados; quando não houver essa informação, o indicador exibirá uma mensagem informativa.

## 2. Objetivos
- Proporcionar leitura confortável e acessível de e-books, com navegação fluida e UI intuitiva.
- Oferecer ferramentas eficientes de estudo (marcação, anotações, dicionário, TTS, IA para explicações e resumos).
- Registrar e apresentar estatísticas de leitura úteis (tempo ativo, páginas, palavras etc.).
- Integrar-se ao Calibre como biblioteca digital principal e apoiar conversões de formato.

## 3. Escopo
- Leitura e visualização de livros em janelas simples e responsivas.
- Marcações e anotações persistentes, com painel lateral dedicado.
- Painel lateral que permite selecionar páginas ou capítulos para marcação.
- Recurso de dicionário on-demand ao clicar em palavras/trechos.
- TTS para leitura de trechos selecionados ou capítulo atual.
- IA (OpenAI) para explicar e resumir seleções, com RAG para contextualização.
- Registro de histórico, progresso e estatísticas, com persistência e restauração de sessão.
- Integração com Calibre Desktop e conversão entre formatos suportados.

## 4. Personas e Principais Casos de Uso
- Leitor focado: leitura simples, ajuste de fonte/tema, continuidade entre sessões.
- Estudante/Pesquisador: marcações, anotações, exportação, dicionário e IA para explicações/resumos.
- Usuário com acessibilidade: TTS para ouvir trechos/obras, controles simples.

## 5. Requisitos Funcionais

1. Tratamento de Arquivos
   - RF-00: Abrir e exibir e-books nos formatos PDF, EPUB e MOBI.
   - RF-01: Suportar navegação por página/capítulo, zoom, busca no texto e índice.
   - RF-02: Ajustes de visualização: tema claro/escuro, tamanho de fonte (quando aplicável), margens.
   - RF-03: Salvar em arquivo de apoio (sidecar) junto do arquivo original.
   - RF-04: Ao abrir o arquivo, lembrar a última pasta aberta.
   - RF-05: Permitir adicionar a pasta a favoritas.
   - RF-06: Lembrar os últimos livros abertos; exibir imediatamente os 5 primeiros e permitir pesquisar os demais.
   - RF-07: Permitir que o usuário selecione uma página ou capítulo para marcação.
   - RF-08: Ao rolar a página, o usuário deve ver o número da página atual.
   - RF-09: A rolagem deve ser continua entre páginas. Seja com o botão scroll do mouse ou com a tecla page up/page down.

2. Integração com IA
   - RF-10: Integração com a OpenAI para Sinônimos, Resumo, Explicação etc. (implementado)
   - RF-11: Integração com a Anthropic para Sinônimos, Resumo, Explicação etc. (planejado)
   - RF-12: Integração com a Claude para Sinônimos, Resumo, Explicação etc. (planejado)
   - RF-13: Integração com a Cohere para Sinônimos, Resumo, Explicação etc. (planejado)
   - RF-14: Integração com a Gemini para Sinônimos, Resumo, Explicação etc. (planejado)
   - RF-15: Integração com a GenerAtiva (https://generativa.rapport.tec.br) para Sinônimos, Resumo, Explicação etc. (implementado)
   - RF-16: Integração com a Ollama (http://localhost:11434) para Sinônimos, Resumo, Explicação etc. (implementado)
   - RF-17: Integração com a OpenRouter (https://openrouter.ai) para Sinônimos, Resumo, Explicação etc. (implementado)
   - RF-17.1: Exibir indicador somente leitura de suporte a Function Calling conforme capacidade declarada pelo provedor/modelo selecionado.
   
2. Integração com APIs externas
   - RF-18: Integração com a OpenLibrary.org para obter informações do livro. Metadados, capa, ISBN, Editora etc. (https://openlibrary.org/dev/docs/restful_api)
   - RF-19: Integração com o Google para obter dados de livros.
   - RF-20: Integração com a Amazon para obter dados de livros.

3. Menu e painel lateral esquerdo
   - RF-13: Menu com opções de leitura, marcações, anotações, estatísticas, configurações, ajuda e sobre.
   - RF-14: Painel lateral esquerdo com lista de livros abertos, favoritos e arquivos recém-abertos.
   - RF-15: Painel lateral direito com lista de marcações e anotações.
   - RF-16: Painel esquerdo alterna entre thumbnails das páginas e árvore dos capítulos.
     - Para PDFs, a árvore de capítulos/subcapítulos é derivada dos bookmarks do documento (quando disponíveis); caso contrário, o TOC recai para a lista de páginas.
   - RF-17: Painel esquerdo pode ser recolhido e expandido.
   - RF-18: Painel esquerdo pode ser arrastado para ajustar seu tamanho.
   - RF-19: A página deve ser aberta na largura exata de exibição.
   - RF-20: A seleção de página deve ser um combobox na barra de ferramentas, listando todas as páginas do documento, com dois botões: anterior e próximo.
   
4. Tratamento de DRM
   - RF-21: Arquivos com DRM devem ser  normalmente, mas devem ser abertos com a opção de desbloquear DRM.
   - RF-22: Arquivos com DRM também podem ser salvos normalmente, mas devem ser salvos com a opção de desbloquear DRM.

5. Marcações e Anotações
   - RF-23: Marcar texto (realce) e criar anotações vinculadas ao trecho.
   - RF-24: Exibir painel lateral com lista de marcações/anotações, com filtro e busca.
   - RF-25: Exportar/importar marcações e anotações (ex.: JSON/Markdown) por livro.
   - RF-26: As marcações devem ser salvas em arquivo de apoio (sidecar) junto do arquivo original.

6. Dicionário
   - RF-27: Ao clicar em uma palavra/trecho, exibir definição em um popover/painel.
   - RF-28: Permitir alternar dicionários/idiomas (deve autodetectar o idioma).
   - RF-29: Gerar explicações em contexto (“por que set aqui é ‘ajustar’ e não ‘conjunto’?”).
   - RF-30: Desambiguação por frase/sentença.
   - RF-31: Paráfrases e exemplos naturais.

7. Text-to-Speech (TTS)
   - RF-32: Integrar com um mecanismo TTS do sistema (ex.: eSpeak/mbrola/festival/SAPI no futuro) para ler trechos marcados ou seção atual.
   - RF-33: Controles de TTS: reproduzir/pausar/parar, velocidade e voz (quando suportado).

8. IA (OpenAI) para Explicações e Resumos (RAG)
   - RF-34: Selecionar um trecho ou parágrafo e solicitar explicação ou resumo via OpenAI API.
   - RF-35: Contextualizar a solicitação via RAG, incluindo parágrafos anteriores e posteriores quando necessário.
   - RF-36: Salvar as respostas da IA como anotações vinculadas ao trecho (opcional).
   - RF-37: Registrar logs (locais) das chamadas (metadados, não conteúdo sensível) para depuração.

9. Estatísticas e Histórico
   - RF-38: Registrar tempo de leitura ativo somente quando a janela do livro estiver em primeiro plano.
   - RF-39: Registrar progresso por livro (página atual, total de páginas quando aplicável, posição no EPUB/MOBI).
   - RF-40: Calcular estatísticas: páginas lidas, palavras (quando disponível), tempo total, sessões.
   - RF-41: Apresentar painel/resumo das estatísticas por livro e geral.

10. Sessão e Restauração
   - RF-42: Lembrar os livros abertos e reabrir automaticamente após reinício/travamento do sistema.
   - RF-43: Persistir preferências de visualização por livro.

11. Integração com Calibre
   - RF-44: Integração nativa com a biblioteca do Calibre: listar livros, abrir diretamente, atualizar metadados.
   - RF-45: Após conversões, cadastrar/atualizar o registro no Calibre.

12. Conversões de Formato
   - RF-46: Converter e-books entre EPUB, MOBI e PDF (quando tecnicamente viável e permitido).
   - RF-47: Expor fila de conversões e status.

13. Configurações
   - RF-50: Tela de Configurações com: URL da OpenAI API, token, dicionários instalados/ordem, TTS/voz, tema, diretórios (Calibre), privacidade.
   - RF-51: Teste de conexão para OpenAI API e validação segura do token.

14. Operações de Arquivo (complementares)
   - RF-52: Salvar como — permitir salvar o livro/estado atual em um novo arquivo (planejado para a versão 0.1.2).

15. Interface e Branding (entregue em 0.1.3)
   - RF-53: Ícone do aplicativo baseado em `docs/imgs/logo-do-projeto.png`, exibido na barra de título.
   - RF-54: Exibir na barra de título o nome do livro em leitura; usar metadados do arquivo quando disponíveis; caso contrário, usar o nome do arquivo.
   - RF-55: Splash screen exibindo `docs/imgs/logo-do-projeto.png`, a versão do app e os dados do autor: "Carlos Delfino <consultoria@carlosdelfino.eti.br>".

## 6. Requisitos Não Funcionais
- RNF-01: Desempenho — abertura de livros e navegação devem ser rápidas; UI responsiva.
- RNF-02: Usabilidade — interface simples, acessível, com atalhos de teclado e hints.
- RNF-03: Portabilidade — Linux prioritário; Windows suportado em builds futuros.
- RNF-04: Manutenibilidade — código modular, CMake, documentação Doxygen.
- RNF-05: Confiabilidade — recuperação de sessão após falhas; persistência robusta.
- RNF-06: Segurança/Privacidade — armazenar segredos (token) de forma segura; não enviar conteúdo do livro sem consentimento explícito.
- RNF-07: Localização — suporte inicial a PT-BR; arquitetura preparada para i18n.
- RNF-08: Distribuição — gerar pacote .deb para distribuição, instalação, atualização e remoção. Integração com o Calibre na instalação e com o desktop.
- RNF-09: Adicionar logs para facilitar a depuração em todo o código.
- RNF-10: Adicionar comentários ao código para facilitar a identificação de partes do código.
- RNF-11: Adicionar documentação do código para facilitar a compreensão de partes do código.
- RNF-12: Adicionar documentação doxygen.
- RNF-13: Adicionar documentação de instalação e uso.
- RNF-14: Adicionar documentação de desenvolvimento.
- RNF-15: Adicionar documentação de teste.
- RNF-16: Adicionar documentação de segurança.
- RNF-17: Adicionar documentação de suporte.

### 6.1 RAG (Experimental) — Requisitos e Limitações

- Dependências recomendadas de extração de texto:
  - `poppler-utils` (fornece `pdftotext` e `pdftoppm`) para extração rápida e com menor uso de memória.
  - `tesseract-ocr` como fallback (OCR por página; mais pesado/lento).
- Configurações de ajuste fino (persistidas em `QSettings`):
  - `emb/chunk_size` (padrão 1000), `emb/chunk_overlap` (padrão 200), `emb/batch_size` (padrão 16).
  - `emb/pages_per_stage` (opcional; processar N páginas por execução) e `emb/pause_ms_between_batches` (opcional; pausa entre lotes).
  - Chaves de LLM em `QSettings` (0.1.9): `ai/provider` (`openai`|`generativa`|`ollama`|`openrouter`), `ai/base_url`, `ai/api_key` (não aplicável a `ollama`), `ai/model`, e prompts (`ai/prompts/*`).
- Restrições atuais (estado experimental):
  - Pode ocorrer sobrecarga de CPU/RAM e encerramento prematuro em documentos grandes.
  - Mitigações implementadas: processamento em etapas, escrita incremental de vetores/ids/metadados, throttling entre batches, cancelamento.
  - Recomenda-se valores conservadores: pages_per_stage=10–25, pause_ms_between_batches=100–250, batch_size=8–16, chunk_size=800–1200, chunk_overlap=100–250.

## 7. Arquitetura e Tecnologias
- Qt6 (Widgets) para UI e renderização de documentos.
- Bibliotecas para parsing/renderização:
  - PDF: Qt PDF (Qt6 PdfWidgets). Alternativa: Poppler-Qt6.
  - EPUB/MOBI: bibliotecas de terceiros (ex.: LibCM, epub SDKs), a avaliar.
- Persistência: arquivos locais (JSON/SQLite) para histórico, anotações e configurações.
- Integrações: Calibre via CLI/DB/API local; OpenAI API via HTTPS.

### Perfil do Leitor (opcional)
- O aplicativo possui diálogo para coleta de "Dados do leitor": nome, e‑mail, WhatsApp e apelido. Todos os campos são opcionais e armazenados localmente via `QSettings`.
- O apelido, quando informado, pode ser inserido nas instruções de sistema enviadas às LLMs para que o assistente se refira ao usuário de forma personalizada.

## 8. Modelo de Dados (alto nível)
- Livro: id, título, autor(es), caminho/URI, formato, metadados.
- Progresso: livroId, posição/página, timestamp.
- Marcação: id, livroId, localização (offset/capítulo/página), texto, cor, criadoEm.
- Anotação: id, marcaçãoId|livroId, texto, origem (usuário|IA), criadoEm.
- Estatística: livroId, sessões [início, fim, ativo], palavras/páginas estimadas.
- Configurações: tema, fonte, dicionários, TTS, OpenAI (URL/token), diretórios, privacidade.

## 9. UX/UI (linhas gerais)
- Leitor com área principal + painel lateral de marcações/anotações recolhível.
- Barra superior com navegação, busca, TTS, IA, dicionário, tema.
- Popover de dicionário sobre seleção/click; opção para abrir no painel.
- Diálogos discretos para chamadas de IA com feedback de carregamento e histórico.

## 10. Integrações
- OpenAI API: endpoints de chat/completions; chave armazenada com segurança; opção para desativar/usar proxy local.
- Calibre: integração com biblioteca local; abrir livros, atualizar metadados, registrar conversões.
- TTS: integração com mecanismo disponível no SO; fallback configurável.

## 11. Segurança e Privacidade
- Armazenar token OpenAI com criptografia local (quando possível) ou permissões restritas.
- Confirmação explícita antes de enviar trechos do livro à IA.
- Logs sem conteúdo sensível (apenas metadados técnicos).

## 12. Critérios de Aceitação (MVP)
- CA-01: Abrir e ler PDF/EPUB com navegação básica, tema claro/escuro.
  - Para PDFs, exibir TOC por capítulos via bookmarks quando disponíveis; na ausência de bookmarks, exibir TOC por páginas.
- CA-02: Criar marcações e anotações; exibir no painel lateral; exportar JSON.
- CA-03: Dicionário funcionando ao clicar em palavras (pelo menos 1 idioma).
- CA-04: TTS lendo trecho selecionado com controles básicos.
- CA-05: OpenAI gerando resumo/explicação de um trecho com RAG básico.
- CA-06: Estatísticas básicas de tempo ativo e progresso por livro.
- CA-07: Restauração de sessão com reabertura dos livros abertos.
- CA-08: Integração com Calibre para abrir livro a partir da biblioteca.

Notas de alinhamento com o estado atual:
- Até a versão 0.1.3, a leitura funcional está focada em PDF (Qt6). EPUB/MOBI permanecem no roadmap.
- Recursos adicionais úteis já presentes: preferência de zoom (Ctrl+roda), seleção/cópia e exportação de seleção (TXT/Markdown), OCR opcional via Tesseract.

## 13. Roadmap (sugestão)
- Fase 1: MVP conforme critérios de aceitação.
- Fase 2: Conversões de formato robustas e fila; dicionários multilíngues; atalhos avançados.
- Fase 3: IA avançada (RAG otimizado, sumários por capítulo, flashcards);
  exportação avançada (Markdown/PDF de marcações/anotações); sincronização opcional.
- Fase 4: Build/instalador Windows; i18n completo; acessibilidade ampliada.

## 14. Riscos e Dependências
- Licenciamento e capacidade técnica das bibliotecas de renderização para EPUB/MOBI.
- Limitações de TTS no ambiente do usuário.
- Custo/latência da OpenAI API; necessidade de alternativas locais.
- Mudanças no Calibre (estrutura de dados/CLI) entre versões.

## 15. Glossário
- RAG (Retrieval-Augmented Generation): técnica que complementa a geração por IA com busca/recuperação de contexto relevante do corpus local.
- TTS (Text-to-Speech): conversão de texto em fala.

## 16. Referências Técnicas
- Qt, CMake, Doxygen.
- Poppler/Qt PDF, bibliotecas para EPUB/MOBI.
- Calibre (CLI/BD), OpenAI API.

---
Este documento substitui e organiza os requisitos iniciais, corrige ortografia/clareza e adiciona detalhes de arquitetura, UX, dados e critérios de aceitação para orientar o desenvolvimento.
