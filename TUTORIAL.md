# GenAI E-Book Reader — Tutorial

Este guia mostra como usar os principais recursos do aplicativo, desde a leitura básica até as funcionalidades avançadas de IA, como a busca semântica (RAG) e o chat com LLM.

## 1. Primeiros Passos: Abrindo e Navegando

1.  **Abrir um Livro**: Vá em `Arquivo > Abrir...` para selecionar um arquivo PDF.
2.  **Navegação**: 
    *   Use a roda do mouse para rolar as páginas.
    *   Use `Ctrl + Roda do mouse` para aplicar zoom.
3.  **Sumário (TOC)**: O painel à esquerda permite navegar pelo conteúdo. Use os botões no topo do painel para alternar entre a visualização de **Páginas** e de **Conteúdo** (capítulos).

## 2. Leitura Interativa: Seleção, Dicionário e Busca

### Selecionar Texto e Imagens

- **Selecionar Texto**: No menu `Editar`, escolha `Selecionar texto` e marque o trecho desejado.
- **Selecionar Imagem**: Escolha `Selecionar retângulo (imagem)` para capturar uma área do documento como imagem.
- **Ações**: Com o texto ou imagem selecionada, você pode copiar, salvar em um arquivo ou enviar para o chat com IA.

### Consultar o Dicionário

1.  **Configurar**: Vá em `Configurações > Dicionário`. Por padrão, o dicionário usa o LLM configurado para fornecer definições.
2.  **Usar**: Selecione uma palavra no texto e clique com o botão direito. No menu de contexto, escolha a opção para consultar o dicionário. A definição aparecerá no painel de chat.

### Buscar no Documento

A barra de busca no topo permite dois tipos de pesquisa:

- **Busca por Texto**: Digite uma palavra ou frase e clique em "Pesquisar". O aplicativo localizará as ocorrências exatas no texto.
- **Busca Semântica (RAG)**: Se a busca por texto não encontrar resultados, o aplicativo automaticamente fará uma busca semântica, que entende o significado da sua pergunta. Para que isso funcione, o documento precisa ser indexado primeiro (veja a seção sobre RAG abaixo).

## 3. Busca Semântica com RAG (Retrieval-Augmented Generation)

O RAG permite que você faça perguntas em linguagem natural sobre o conteúdo do livro. Para usá-lo, você precisa primeiro criar um "índice de embeddings" para o seu documento.

### Passo 1: Configurar os Embeddings

1.  Vá em `Configurações > Embeddings`.
2.  **Provedor**: Escolha um provedor para gerar os embeddings. `Ollama` é uma ótima opção para rodar localmente na sua máquina.
3.  **Modelo**: Selecione um modelo de embedding (ex: `nomic-embed-text`).
4.  **Parâmetros de Chunking**: Ajuste o `Tamanho do chunk` e a `Sobreposição` para controlar como o texto é dividido antes de ser processado. Valores padrão costumam funcionar bem.

### Passo 2: Indexar o Documento

1.  Com o livro aberto, clique com o botão direito no visualizador e escolha `Recriar embeddings do documento...`.
2.  Aguarde o processo terminar. Uma janela mostrará o progresso.

### Passo 3: Fazer uma Busca Semântica

- Agora, use a barra de busca para fazer perguntas como "Qual é a ideia principal do capítulo 2?" ou "Explique o conceito de X". O aplicativo usará o índice RAG para encontrar as partes mais relevantes do livro e responder à sua pergunta.


## 4. Integração com IA (LLM)

Você pode conectar o aplicativo a um serviço de LLM para usar o chat, pedir resumos, sinônimos e muito mais.

### Configurar o Acesso ao LLM

1.  Vá em `Configurações > Configurações de LLM`.
2.  **Provedor**: Escolha um provedor. Você pode usar a `OpenAI`, a `GenerAtiva` ou um serviço local como o `Ollama` (selecione `OpenAI` como provedor e informe a `Base URL` do seu Ollama, ex: `http://localhost:11434`).
3.  **API Key**: Insira sua chave de API (para serviços na nuvem).
4.  **Modelo**: Escolha o modelo que deseja usar (ex: `gpt-4o-mini`, `llama3`).
5.  **Prompts**: Personalize os prompts que a IA usará para gerar resumos, sinônimos, etc.

As configurações ficam em `QSettings` com as chaves:
- `ai/provider`: `openai` | `generativa`
- `ai/base_url`: URL base para override (opcional)
- `ai/api_key`: token secreto
- `ai/model`: nome do modelo (ex.: `gpt-4o-mini`)
- `ai/prompts/synonyms`, `ai/prompts/summaries`, `ai/prompts/explanations`, `ai/prompts/chat`

Privacidade:
- A aplicação sempre pede confirmação antes de enviar qualquer trecho do livro à IA.
- Tokens ficam armazenados nas preferências do usuário.

### Usar os Recursos de IA

- **Chat**: Abra o painel de chat para conversar com a IA. Você pode fazer perguntas, pedir explicações ou enviar trechos e imagens do livro para análise.
- **Resumos e Sinônimos**: Selecione um texto, clique com o botão direito e escolha a opção para resumir ou encontrar sinônimos.
- **Dicionário**: Selecione uma palavra e use a função de dicionário para obter sua definição no painel de chat.

Observações:
- As mensagens usam o modelo selecionado nas Configurações de LLM.
- Os prompts do diálogo podem ser personalizados nas Configurações de LLM.

## 5. Dicas e Recursos Adicionais

- **Botão de Título**: O botão no centro da barra superior mostra o nome do seu livro. Clique com o botão direito nele para acessar ações rápidas, como renomear o arquivo ou abri-lo no gerenciador de arquivos.
- **Solução de Problemas**: Se a IA não responder, verifique suas configurações de LLM (API Key, Base URL) e sua conexão com a internet.
- **Atalhos**: Use `Ctrl+Enter` para enviar mensagens no painel de chat.
