# GenAI E-Book Reader — Descrição e Requisitos de Software

## 1. Visão Geral
Este software visa atender a uma necessidade prática durante a leitura de e-books (PDF, MOBI e EPUB), oferecendo uma experiência simples, fluida e produtiva, com recursos de leitura, marcação, anotações, estatísticas de leitura, dicionário, Text-to-Speech (TTS) e apoio de IA para resumos e explicações via RAG.

- Plataforma principal: Linux (prioritário).
- Portabilidade: Windows (compilável em versões futuras).
- Tecnologias: C/C++ com Qt para GUI, CMake para build, Doxygen para documentação.

## 2. Objetivos
- Proporcionar leitura confortável e acessível de e-books, com navegação fluida e UI intuitiva.
- Oferecer ferramentas eficientes de estudo (marcação, anotações, dicionário, TTS, IA para explicações e resumos).
- Registrar e apresentar estatísticas de leitura úteis (tempo ativo, páginas, palavras etc.).
- Integrar-se ao Calibre como biblioteca digital principal e apoiar conversões de formato.

## 3. Escopo
- Leitura e visualização de livros em janelas simples e responsivas.
- Marcações e anotações persistentes, com painel lateral dedicado.
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

1. Menu e painel lateral esquerdo
   - RF-07: Menu com opções de leitura, marcações, anotações, estatísticas, configurações, ajuda e sobre.
   - RF-08: Painel lateral esquerdo com lista de livros abertos, favoritos e arquivos recém-abertos.
   - RF-09: Painel lateral direito com lista de marcações e anotações.
   - RF-10: Painel esquerdo alterna entre thumbnails das páginas e árvore dos capítulos.
     - Para PDFs, a árvore de capítulos/subcapítulos é derivada dos bookmarks do documento (quando disponíveis); caso contrário, o TOC recai para a lista de páginas.
   - RF-11: Painel esquerdo pode ser recolhido e expandido.
   - RF-12: Painel esquerdo pode ser arrastado para ajustar seu tamanho.
   - RF-13: A página deve ser aberta na largura exata de exibição.
   - RF-14: A seleção de página deve ser um combobox na barra de ferramentas, listando todas as páginas do documento, com dois botões: anterior e próximo.
   
2. Tratamento de DRM
   - RF-15: Arquivos com DRM devem ser  normalmente, mas devem ser abertos com a opção de desbloquear DRM.
   - RF-16: Arquivos com DRM também podem ser salvos normalmente, mas devem ser salvos com a opção de desbloquear DRM.

2. Marcações e Anotações
   - RF-04: Marcar texto (realce) e criar anotações vinculadas ao trecho.
   - RF-05: Exibir painel lateral com lista de marcações/anotações, com filtro e busca.
   - RF-06: Exportar/importar marcações e anotações (ex.: JSON/Markdown) por livro.
   - RF-07: As marcações devem ser salvas em arquivo de apoio (sidecar) junto do arquivo original.

3. Dicionário
   - RF-08: Ao clicar em uma palavra/trecho, exibir definição em um popover/painel.
   - RF-09: Permitir alternar dicionários/idiomas (deve autodetectar o idioma).

4. Text-to-Speech (TTS)
   - RF-10: Integrar com um mecanismo TTS do sistema (ex.: eSpeak/mbrola/festival/SAPI no futuro) para ler trechos marcados ou seção atual.
   - RF-11: Controles de TTS: reproduzir/pausar/parar, velocidade e voz (quando suportado).

5. IA (OpenAI) para Explicações e Resumos (RAG)
   - RF-12: Selecionar um trecho ou parágrafo e solicitar explicação ou resumo via OpenAI API.
   - RF-13: Contextualizar a solicitação via RAG, incluindo parágrafos anteriores e posteriores quando necessário.
   - RF-14: Salvar as respostas da IA como anotações vinculadas ao trecho (opcional).
   - RF-15: Registrar logs (locais) das chamadas (metadados, não conteúdo sensível) para depuração.

6. Estatísticas e Histórico
   - RF-16: Registrar tempo de leitura ativo somente quando a janela do livro estiver em primeiro plano.
   - RF-17: Registrar progresso por livro (página atual, total de páginas quando aplicável, posição no EPUB/MOBI).
   - RF-18: Calcular estatísticas: páginas lidas, palavras (quando disponível), tempo total, sessões.
   - RF-19: Apresentar painel/resumo das estatísticas por livro e geral.

7. Sessão e Restauração
   - RF-20: Lembrar os livros abertos e reabrir automaticamente após reinício/travamento do sistema.
   - RF-21: Persistir preferências de visualização por livro.

8. Integração com Calibre
   - RF-22: Integração nativa com a biblioteca do Calibre: listar livros, abrir diretamente, atualizar metadados.
   - RF-23: Após conversões, cadastrar/atualizar o registro no Calibre.

9. Conversões de Formato
   - RF-24: Converter e-books entre EPUB, MOBI e PDF (quando tecnicamente viável e permitido).
   - RF-25: Expor fila de conversões e status.

10. Configurações
   - RF-26: Tela de Configurações com: URL da OpenAI API, token, dicionários instalados/ordem, TTS/voz, tema, diretórios (Calibre), privacidade.
   - RF-27: Teste de conexão para OpenAI API e validação segura do token.

    11. Operações de Arquivo (complementares)
   - RF-28: Salvar como — permitir salvar o livro/estado atual em um novo arquivo (planejado para a versão 0.1.2).

## 6. Requisitos Não Funcionais
- RNF-01: Desempenho — abertura de livros e navegação devem ser rápidas; UI responsiva.
- RNF-02: Usabilidade — interface simples, acessível, com atalhos de teclado e hints.
- RNF-03: Portabilidade — Linux prioritário; Windows suportado em builds futuros.
- RNF-04: Manutenibilidade — código modular, CMake, documentação Doxygen.
- RNF-05: Confiabilidade — recuperação de sessão após falhas; persistência robusta.
- RNF-06: Segurança/Privacidade — armazenar segredos (token) de forma segura; não enviar conteúdo do livro sem consentimento explícito.
- RNF-07: Localização — suporte inicial a PT-BR; arquitetura preparada para i18n.
- RNF-08: Distribuição — gerar pacote .deb para distribuição, instalação, atualização e remoção. Integração com o Calibre na instalação e com o desktop.

## 7. Arquitetura e Tecnologias
- Qt (Widgets/QML a definir) para UI e renderização de documentos.
- Bibliotecas para parsing/renderização:
  - PDF: Poppler/Qt PDF.
  - EPUB/MOBI: bibliotecas de terceiros (ex.: LibCM, epub SDKs), a avaliar.
- Persistência: arquivos locais (JSON/SQLite) para histórico, anotações e configurações.
- Integrações: Calibre via CLI/DB/API local; OpenAI API via HTTPS.

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
