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

## 0.1.3 - Refinamento da interface de usuário (Planejado)
- com base no arquivo docs/imgs/logo-do-projeto.png para gere um icone para ser usado no app na barra de títulos
- Adicione a barra de títulos o nome do livro que está sendo lido, use os metadados do arquivo, se não estiver disponível use o nome do arquivo.
- Faça uma splash screen para mostrar o arquivo docs/imgs/logo-do-projeto.png e a versão do app. Dados do autor: Carlos Delfino <consultoria@carlosdelfino.eti.br>

## 0.2.0 – Anotações e Dicionário
- Marcação de trechos e notas com exportação JSON
- Dicionário on-click (mínimo 1 idioma)
- Busca full-text no documento

## 0.3.0 – TTS e Controles
- Reprodução TTS de seleção atual
- Controles de velocidade/pausa/retomar
- Output device selection

## 0.4.0 – IA (RAG) e Resumos
- Integração com API de IA (ex.: OpenAI)
- Resumo e explicação de trechos selecionados
- Histórico de prompts e respostas

## 0.5.0 – Estatísticas e Sessões
- Tempo ativo de leitura, progresso, sessões
- Restauração de sessão (últimos livros abertos)
- Export de métricas

## 0.6.0 – Integração Calibre e Melhorias UX
- Abrir itens da biblioteca do Calibre
- Melhorias de performance e usabilidade

## 1.x – Estabilidade e Releases
- Testes automatizados
- Empacotamento (AppImage/Flatpak)
- Internacionalização (i18n)

## Backlog Técnico
- CI (build + testes)
- Lint/format (clang-format, cmake-format)
- Benchmarks de render de páginas
