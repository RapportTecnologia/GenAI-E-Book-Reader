# GenAI E-Book Reader — Tutorial

Este guia passo a passo mostra como instalar, abrir seus livros, usar os recursos de seleção/exportação, Recentes, e configurar/usar uma LLM (OpenAI/GenerAtiva) para chat, sinônimos e resumos.

## 1) Instalar e executar

- Requisitos de build: ver `README.md` na seção "Como rodar (Linux)".
- Compilar:
  ```bash
  cmake -S . -B build
  cmake --build build -j
  ./build/bin/genai_reader
  ```
- Pacote pronto (quando disponível): ver diretório `dist/`.

## 2) Abrir um PDF

- Menu `Arquivo > Abrir...` e selecione um arquivo `.pdf`.
- A área central exibe o documento. Use a roda do mouse e Ctrl para zoom.
- Preferência de granularidade do zoom pode ser ajustada em `Configurações`.

## 3) Sumário (TOC) e navegação

- O painel de TOC fica à esquerda e pode alternar entre:
  - "Páginas": lista todas as páginas.
  - "Conteúdo": capítulos e subcapítulos (quando o PDF tem bookmarks).
- Use os botões de navegação no topo do painel.
- Arraste o divisor para ajustar a largura do painel; a preferência é lembrada.

## 4) Seleção, cópia e exportação de trecho

- Modos de seleção: texto ou retângulo (imagem).
- Copie a seleção; um toast confirma a ação.
- Exportação de seleção para TXT/Markdown está disponível pelo menu `Editar`.

## 5) Recentes: localizar e abrir rapidamente

- Acesse `Arquivo > Documento > Recentes` para os últimos itens.
- `Mostrar todos...` abre o diálogo completo com busca.
- Pesquise por nome do arquivo, título, autor, resumo (subject) ou palavras‑chave.
- Os metadados são persistidos em `QSettings` quando você abre um PDF.

## 6) Configurar uma LLM (OpenAI/GenerAtiva)

A aplicação integra-se a provedores compatíveis com a API da OpenAI.

- Provedores suportados:
  - OpenAI (`https://api.openai.com`)
  - GenerAtiva (`https://generativa.rapport.tec.br`)

Passo a passo:
1. Abra o menu `Configurações > Configurações de LLM`.
2. Escolha o provedor (OpenAI ou GenerAtiva).
3. Selecione um modelo (ex.: `gpt-4o-mini`).
4. Preencha sua `API Key` do provedor escolhido.
5. (Opcional) Informe `Base URL` para usar um endpoint compatível (proxy/self-host/etc.).
6. Ajuste os prompts padrão nas abas "Sinônimos", "Resumos", "Explicações" e "Chat".
7. Confirme em OK para salvar.

As configurações ficam em `QSettings` com as chaves:
- `ai/provider`: `openai` | `generativa`
- `ai/base_url`: URL base para override (opcional)
- `ai/api_key`: token secreto
- `ai/model`: nome do modelo (ex.: `gpt-4o-mini`)
- `ai/prompts/synonyms`, `ai/prompts/summaries`, `ai/prompts/explanations`, `ai/prompts/chat`

Privacidade:
- A aplicação sempre pede confirmação antes de enviar qualquer trecho do livro à IA.
- Tokens ficam armazenados nas preferências do usuário.

## 7) Usar a IA no leitor

Há três formas principais:

- Sinônimos (trecho curto):
  1. Selecione uma palavra/locução.
  2. Ação de IA para sinônimos (menu/contexto/atalho conforme sua UI).
  3. Confirme o envio. O resultado aparece no diálogo de resultados.

- Resumo (trecho selecionado):
  1. Selecione um parágrafo ou trecho.
  2. Ação de IA para resumo.
  3. Confirme o envio. O resultado é exibido no diálogo de "Resumo".

- Chat com a IA (painel lateral):
  1. Abra o painel de chat quando desejar interagir livremente.
  2. Digite sua pergunta e clique em "Enviar" (atalho: Ctrl+Enter envia; Enter insere nova linha).
  3. Você pode:
     - Enviar um trecho selecionado do livro para o chat (com confirmação).
     - Salvar a conversa em um arquivo `.txt`.
     - Solicitar um "Compilado" do diálogo (resumo do transcript atual).

Observações:
- As mensagens usam o modelo selecionado nas Configurações de LLM.
- Os prompts do diálogo podem ser personalizados nas Configurações de LLM.

## 8) Dados do leitor (opcional, PHPList)

- `Arquivo > Leitor > Dados do leitor...` permite registrar nome, e-mail e WhatsApp.
- É possível enviar para uma lista no PHPList configurando um arquivo `.env` ao lado do executável ou na raiz do projeto:
  ```env
  PHPLIST_URL=http://seu-servidor/phplist/api/v2
  PHPLIST_USER=usuario
  PHPLIST_PASS=senha
  ```
- O envio é sempre confirmado com o usuário.

## 9) Dicas e solução de problemas

- Sem resposta da IA:
  - Verifique a conectividade de rede.
  - Confirme `API Key`, provedor e `Base URL` nas Configurações de LLM.
  - Alguns provedores exigem saldo/créditos ou permissões para o modelo escolhido.

- Erros HTTP na IA:
  - A mensagem de erro mostrará o código HTTP e um trecho do corpo da resposta.
  - Cheque se sua chave está correta e se o endpoint está ativo.

- Zoom/seleção no PDF:
  - Ajuste a granularidade do zoom em `Configurações`.
  - Alterne entre seleção de texto e retângulo conforme a sua necessidade.

## 10) Atalhos úteis

- Chat: Ctrl+Enter para enviar mensagem no painel de chat; Enter cria nova linha.
- Navegação rápida: use os botões do TOC e a combinação de zoom com Ctrl+roda do mouse.

## 11) Sobre

- Versão atual: ver badge no `README.md`.
- Licença: CC BY 4.0. Consulte `LICENSE`.
- Agradecimentos: comunidade Qt, projetos de PDF/Poppler, Calibre e contribuidores.
