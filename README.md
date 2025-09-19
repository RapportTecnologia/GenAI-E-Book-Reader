![Visitantes do Projeto](https://visitor-badge.laobi.icu/badge?page_id=rapporttecnologia.genai-e-book-reader)
[![License: CC BY 4.0](https://img.shields.io/badge/License-CC%20BY%204.0-lightgrey.svg)](LICENSE)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue)
![Qt](https://img.shields.io/badge/Qt6-Widgets-brightgreen)
![CMake](https://img.shields.io/badge/CMake-%3E%3D3.16-informational)
[![Docs](https://img.shields.io/badge/docs-Doxygen-blueviolet)](docs/index.html)
![Version](https://img.shields.io/badge/version-0.1.6-blue)
[![Contributions Welcome](https://img.shields.io/badge/contributions-welcome-success.svg)](#contribuindo)


# GenAI E-Book Reader

Leitor de e-books moderno com foco em produtividade e estudo, desenvolvido em C/C++ com Qt6, com recursos planejados de anotações, dicionário, Text-to-Speech (TTS), estatísticas de leitura e apoio de IA (RAG) para resumos e explicações.

- Requisitos e escopo completos: consulte [REQUIREMENTS.md](REQUIREMENTS.md).
- Plano do projeto (fases, sprints e critérios): consulte [PLANO-DE-DESENVOLVIMENTO.md](PLANO-DE-DESENVOLVIMENTO.md).
- Histórico de mudanças: consulte [CHANGELOG.md](CHANGELOG.md).
- Planejamento de releases: consulte [ROADMAP.md](ROADMAP.md).
- Tutorial passo a passo: consulte [TUTORIAL.md](TUTORIAL.md).

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
- Barra superior com botão de título central que mostra o nome do documento em leitura e oferece menu contextual (abrir diretório, adicionar ao Calibre com migração de embeddings, renomear com migração).
- Barra de busca com pesquisa por texto e fallback para busca semântica por frases usando embeddings; inclui opções rápidas (métrica, Top‑K e limiares).

Nota (0.1.6): otimização da renderização do chat, com a conversão de Markdown movida para o back-end (C++), corrigindo bugs de instabilidade com MathJax.

Nota (0.1.3): refinamento de UI — ícone do app a partir de `docs/imgs/logo-do-projeto.png`, splash screen com versão/autor e título da janela exibindo o nome do livro (metadados quando disponíveis; senão, nome do arquivo). Ajuste para a área de leitura ocupar 100% do espaço disponível. Painel de TOC remodelado com barra de ferramentas (alternar entre "Páginas"/"Conteúdo" e botões de navegação) e tamanho padrão inicial do splitter ~10% (TOC) / ~90% (visualizador).

Nota (0.1.2): implementado "Salvar como" (RF-28) e pequenos aprimoramentos de leitura.

Nota (0.1.1): adicionados seleção de página via combobox e restauração do último arquivo/diretório aberto.

Observação: para PDFs, o sumário (TOC) usa bookmarks (capítulos/subcapítulos) quando disponíveis; na ausência, lista todas as páginas. A seleção por combobox contempla todas as páginas do documento. O painel de TOC inclui uma barra de ferramentas para alternar entre "Páginas" e "Conteúdo" e botões de navegação; por padrão o painel ocupa ~10% da largura da janela na primeira execução. Atualmente, o binário suporta apenas arquivos PDF.

## Sumário (TOC) e Navegação
- Alternar modo do TOC:
  - "Páginas": lista plana de páginas; os botões "Voltar/Avançar" mudam a página atual.
  - "Conteúdo": capítulos (grupos de páginas); os botões percorrem o item anterior/próximo (capítulo ou página filho). 
- Atalhos do painel: arraste o divisor para redimensionar; o tamanho fica salvo para as próximas sessões.

## Busca no documento (texto e semântica)

- A barra de busca fica na parte superior e contém:
  - Campo de pesquisa, botões "Pesquisar", "Anterior" e "Próximo".
  - Menu "Opções" com ajustes rápidos de similaridade: métrica (Cosseno, Dot ou L2), Top‑K, limiar de similaridade (para cosseno/dot) e distância máxima (para L2).
- Funcionamento:
  - A busca tenta primeiro localizar o texto literal no PDF (por página).
  - Se nada for encontrado, a aplicação executa a busca semântica por frases usando o índice de embeddings do documento e navega para as páginas mais relevantes.
- Pré‑requisito para a busca semântica: o documento precisa ter embeddings indexados.
  - Para recriar o índice, clique com o botão direito dentro do PDF e escolha "Recriar embeddings do documento...".
  - Também é possível ajustar o provedor/modelo e parâmetros em `Configurações > Embeddings`.

### Botão de Título (barra superior)

- Exibe o título do documento (para PDFs, usa metadados Title quando disponíveis; senão, nome do arquivo).
- Clique para ver o caminho completo do arquivo e copiá‑lo para a área de transferência.
- Menu contextual (clique direito):
  - "Abrir diretório no gerenciador"
  - "Adicionar ao Calibre e migrar embeddings..."
  - "Renomear arquivo e migrar embeddings..."

## IA (LLM): Configuração e Uso
A aplicação possui integração com provedores compatíveis com a API da OpenAI para chat, resumos e sinônimos.
Atualmente são suportados:
- OpenAI (`https://api.openai.com`)
- GenerAtiva (`https://generativa.rapport.tec.br`)

Como configurar:
- Abra o diálogo "Configurações de LLM" (menu de Configurações).
- Selecione o provedor (OpenAI ou GenerAtiva) e o modelo (ex.: `gpt-4o-mini`).
- Informe a API Key do provedor escolhido.
- Opcional: preencha "Base URL" para apontar a um endpoint compatível com OpenAI.
- Ajuste os prompts padrão para Sinônimos, Resumos, Explicações e Chat conforme sua preferência.

Uso no leitor:
- Sinônimos: selecione uma palavra/locução e acione a ação de IA para sinônimos; será solicitado consentimento antes do envio.
- Resumo: selecione um trecho e acione a ação de IA para resumo; o resultado abre no diálogo de resumo.
- Chat: envie um trecho ao chat da IA ou digite livremente no painel de chat.
  - Renderização avançada no painel de chat (Markdown/HTML):
    - Tabelas Markdown (GFM) com bordas, cabeçalho e rolagem horizontal quando necessário.
    - Syntax highlighting para blocos de código (highlight.js, tema GitHub).
    - MathJax v3 para fórmulas (inline e display), aplicado após o parse do Markdown.
    - Auto-scroll para a última mensagem recebida/enviada.
  - Sessões de chat:
    - Botão "Novo" inicia uma nova conversa (pergunta se deseja salvar a conversa atual no histórico, com título automático).
    - Botão "Histórico" lista e restaura conversas salvas (por arquivo aberto), mantendo o contexto da IA.
  - Contexto contínuo: novos envios incluem o histórico completo de mensagens (system/user/assistant) para melhor continuidade.

Persistência/Configurações (QSettings):
- `ai/provider`: `openai` | `generativa` (padrão: `openai`)
- `ai/base_url`: URL base para override (opcional)
- `ai/api_key`: token secreto do provedor
- `ai/model`: nome do modelo (ex.: `gpt-4o-mini`)
- `ai/prompts/synonyms`, `ai/prompts/summaries`, `ai/prompts/explanations`, `ai/prompts/chat`

Observações de privacidade:
- Antes de qualquer envio de conteúdo à IA, a aplicação solicita sua confirmação.
- Tokens são armazenados nas preferências do usuário (`QSettings`).

Para um guia passo a passo com imagens e dicas, consulte o [TUTORIAL.md](TUTORIAL.md).

## RAG (Experimental)

ATENÇÃO: Este recurso está em testes e pode causar sobrecarga de recursos em algumas máquinas, levando a travamentos ou encerramento prematuro da aplicação/terminal. Estamos estabilizando o pipeline com processamento em etapas e limites de consumo.

### O que é
- Indexação de PDFs em vetores para permitir buscas semânticas e respostas com contexto.
- Pipelines 100% em C++ (sem Python no cliente), com provedores de embeddings via HTTP:
  - `openai`, `generativa` (compatível com OpenAI) e `ollama` (local).
- Armazenamento vetorial local em disco (formato binário simples + JSONs de ids/metadados) em `~/.cache/br.tec.rapport.genai-reader/`.

### Como usar
1. Abra um PDF no leitor.
2. Clique com o botão direito dentro do PDF e escolha `Recriar embeddings do documento...`.
3. Acompanhe o diálogo de progresso (estágios, percentuais, métricas). Se configurado por etapas, execute novamente para continuar do ponto onde parou.

### Configuração (Configurações > Embeddings)
- Provedor: `OpenAI`, `GenerAtiva` (Base URL e API Key) ou `Ollama` local.
- Modelo de embeddings (ex.: `text-embedding-3-small`, `nomic-embed-text:latest`).
- Banco (diretório de cache): padrão `~/.cache/br.tec.rapport.genai-reader`.
- Tuning (ajustes finos):
  - `Tamanho do chunk` (padrão 1000)
  - `Sobreposição do chunk` (padrão 200)
  - `Tamanho do lote (batch)` (padrão 16)
  - `Páginas por etapa` (opcional; processa N páginas por execução para evitar exaustão)
  - `Pausa entre lotes (ms)` (opcional; insere uma pausa entre batches)

Sugestões de valores seguros:
- Páginas por etapa: 10–25
- Pausa entre lotes: 100–250 ms
- Batch: 8–16
- Chunk size: 800–1200
- Overlap: 100–250

### Dependências para extração de texto (recomendadas)
- Preferencial: `poppler-utils` (fornece `pdftotext` e `pdftoppm`) — mais rápido e com menor uso de memória.
- Fallback: `tesseract-ocr` (OCR por página; mais pesado e lento, usar apenas quando necessário).
- A aplicação detecta automaticamente e alerta se estiverem ausentes, sugerindo instalação.

### Estado atual e limitações
- Experimental: pode ocorrer sobrecarga de CPU/RAM e encerramento da aplicação/terminal em documentos grandes.
- Mitigações implementadas:
  - Processamento em etapas (interrompe após N páginas; execute novamente para continuar).
  - Escrita incremental de vetores/ids/metadados (sem buffers gigantes em memória).
  - Throttling entre lotes (pausa configurável) e respeito ao cancelamento.
  - Logs de estágios, métricas e progresso em tempo real na janela de indexação.
- Próximas melhorias:
  - Diagnóstico de dependências (status de `pdftotext`, `pdftoppm`, `tesseract`) na UI.
  - Fallbacks adicionais e otimizações em I/O de metadados.

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

## Como gerar um release no GitHub

Pré-requisitos:
- Ter o repositório remoto configurado (origin) e acesso de push.
- Opcional: GitHub CLI (`gh`) autenticado: `gh auth login`.

Passos sugeridos:

```bash
# 1) Garanta que CHANGELOG.md e README.md estão atualizados (ex.: 0.1.5)
git add CHANGELOG.md README.md
git commit -m "docs: atualiza changelog e readme para v0.1.5"

# 2) Versione no código se aplicável (CMakeLists.txt, headers) e commite
# Exemplo (se houve mudança de versão de build)
# git add CMakeLists.txt include/app/App.h
# git commit -m "chore(release): bump version to v0.1.5"

# 3) Crie uma tag anotada e envie
git tag -a v0.1.5 -m "v0.1.5"
git push origin v0.1.5

# 4A) Criar release via GitHub CLI (anexando notas do CHANGELOG)
gh release create v0.1.5 \
  --title "v0.1.5" \
  --notes "Consulte CHANGELOG.md para detalhes desta versão."

# 4B) Alternativa: criar release pela UI do GitHub
# - Vá em Releases > Draft a new release > Escolha a tag v0.1.5 > Preencha título/notas > Publish

# 5) (Opcional) Anexar binários
# Se você tiver artefatos em dist/, anexe com:
# gh release upload v0.1.5 dist/genai_reader-v0.1.5-linux-x86_64 dist/genai_reader-v0.1.5-linux-x86_64.tar.gz
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
