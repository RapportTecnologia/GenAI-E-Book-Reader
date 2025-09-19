# GenAI E-Book Reader — Plano de Desenvolvimento (baseado em REQUISITOS.md)

Documento de planejamento derivado de `REQUISITOS.md`. Toda decisão e escopo aqui devem manter rastreabilidade aos RF/RNF/CA descritos em `REQUISITOS.md`.

## 1) Estratégia e Escopo do MVP
- Objetivo: entregar os Critérios de Aceitação CA-01 a CA-08 (ver `REQUISITOS.md`, seção 12).
- Plataformas: Linux (principal). Windows: preparar portabilidade (sem entrega no MVP).
- Stack: C/C++ + Qt; CMake; Doxygen; persistência local (JSON ou SQLite — decidir no início).

## 2) Roadmap por Fases
- Fase 1 — MVP (alinha com CA-01..CA-08):
  - Leitura PDF/EPUB com navegação básica e tema (RF-01..RF-03, CA-01).
  - Marcações e anotações com painel lateral e exportação JSON (RF-04..RF-06, CA-02).
  - Dicionário mínimo funcional (1 idioma) (RF-08, CA-03).
  - TTS básico em trechos selecionados (RF-10..RF-11, CA-04).
  - IA para resumo/explicação com RAG básico (RF-12..RF-15, CA-05).
  - Estatísticas de tempo ativo e progresso (RF-16..RF-19, CA-06).
  - Restauração de sessão (RF-20..RF-21, CA-07).
  - Integração Calibre: abrir livro da biblioteca (RF-22, CA-08).

- Fase 2 — Consolidação e Conversões:
  - Conversões EPUB/MOBI/PDF e fila (RF-24..RF-25).
  - Dicionários multilíngues e configuráveis (RF-09).
  - Atalhos de teclado e usabilidade (RNF-02).
  - v0.1.2 (concluída em 2025-08-18): "Salvar como" (RF-28) e pequenos aprimoramentos de leitura.
  - v0.1.3 (concluída em 2025-08-18): Refinamento de UI — ícone do app (RF-29), título com nome do livro (RF-30) e splash screen (RF-31). Painel de Sumário (TOC) remodelado com barra de ferramentas (alternância "Páginas"/"Conteúdo") e botões de navegação; tamanho padrão do splitter ~10%/90% na primeira execução.

- Fase 3 — IA avançada e Exportações:
  - RAG otimizado; sumários por capítulo; geração de flashcards (RF-12..RF-15).
  - Exportações avançadas de marcações/anotações (Markdown/PDF).
  - Sincronização opcional (fora do MVP; requer design de privacidade).

- Fase 4 — Portabilidade e Acessibilidade:
  - Build/instalador Windows (RNF-03).
  - i18n completo; acessibilidade ampliada.

## 3) Sprints Sugeridos (Fase 1 — 4 sprints de ~2 semanas)
- Sprint 1: Fundações e Leitura
  - Estrutura do projeto (CMake, Doxygen, CI local mínima).
  - Decisão Qt: Widgets vs QML. Prototipar ambas rapidamente.
  - Render PDF/EPUB e navegação básica; tema claro/escuro (RF-01..RF-03).
  - Persistência inicial (JSON/SQLite — PoC e escolha).

- Sprint 2: Marcações/Anotações e Dicionário
  - Realce + criação/edição de anotações; painel lateral (RF-04..RF-05).
  - Exportar anotações (JSON) (RF-06).
  - Dicionário on-click básico (RF-08) e configuração simples (RF-09 parcial).

- Sprint 3: TTS, IA (RAG), Estatísticas
  - Integração TTS e controles básicos (RF-10..RF-11).
  - OpenAI: resumo/explicação com contexto (RAG básico) (RF-12..RF-14).
  - Logs locais de metadados (RF-15) e políticas de privacidade (RNF-06).
  - Tempo ativo + progresso + painel de estatísticas (RF-16..RF-19).

- Sprint 4: Sessão, Calibre e Hardening
  - Restauração de sessão (RF-20..RF-21).
  - Integração Calibre para abrir livro (RF-22, CA-08).
  - Tela de Configurações (RF-26..RF-27).
  - Testes de aceitação CA-01..CA-08; estabilização e documentação.

## 4) Entregas e Critérios de Aceitação (DoD)
- Cada história deve ter:
  - Implementação funcional.
  - Testes manuais guiados e, quando possível, testes automatizados.
  - Documentação mínima (Doxygen/README). 
  - Registro em CHANGELOG.
- Encerramento do MVP: todos CA-01..CA-08 verificados em Linux.

## 5) Matriz de Rastreabilidade (resumo)
- Leitura: RF-01..03 → CA-01 → Sprints 1–2.
- Marcações/Anotações: RF-04..06 → CA-02 → Sprint 2.
- Dicionário: RF-08..09 → CA-03 → Sprint 2.
- TTS: RF-10..11 → CA-04 → Sprint 3.
- IA/RAG: RF-12..15 → CA-05 → Sprint 3.
- Estatísticas: RF-16..19 → CA-06 → Sprint 3.
- Sessão: RF-20..21 → CA-07 → Sprint 4.
- Calibre: RF-22 → CA-08 → Sprint 4.
- Configurações: RF-26..27 → suporte geral → Sprint 4.
 - Operações de Arquivo: RF-28 (Salvar como) → release menor v0.1.2.
 - UI/Branding: RF-29 (ícone), RF-30 (título com nome do livro), RF-31 (splash) → release menor v0.1.3.

## 6) Decisões Técnicas Iniciais (a validar no Sprint 1)
- UI Qt: Widgets (maior maturidade e estabilidade) vs QML (flexibilidade/anim.)
- PDF: Poppler/Qt PDF; EPUB/MOBI: avaliar libs com licença compatível.
- Persistência: SQLite (transações/robustez) vs JSON (simplicidade). Sugestão: SQLite.
- Integração Calibre: CLI vs acesso ao DB; começar pela CLI para reduzir acoplamento.

## 7) Estrutura Inicial do Repositório
```
/ (raiz)
  CMakeLists.txt
  /docs
    REQUISITOS.md
    PLANO-DE-DESENVOLVIMENTO.md
    CHANGELOG.md
  /src
    app/ (bootstrap Qt)
    ui/ (widgets/qml)
    reader/ (pdf, epub, mobi)
    notes/ (marcações, anotações)
    tts/
    ai/
    stats/
    session/
    calibre/
    config/
  /include
  /third_party
  /assets
  /tests
```

## 8) Riscos e Mitigações (prioritários)
- EPUB/MOBI libs: risco de licença/qualidade → PoC e revisão legal no Sprint 1.
- TTS disponibilidade: heterogêneo no Linux → fallback e configuração clara.
- Custo/latência OpenAI: cache de contexto local; opção de desativar; permitir endpoint alternativo.
- Respeito à privacidade: opt-in para enviar conteúdo à IA; armazenamento seguro do token.

## 9) Métricas de Projeto
- Velocidade por sprint; bugs por entrega; tempo médio de leitura de arquivos grandes; uso de memória; tempo de cold start.

## 10) Cronograma Base (exemplo)
- 8 semanas para MVP (4 sprints x 2 semanas). Ajustar conforme disponibilidade.

## 11) Próximos Passos
 - Decidir Qt (Widgets vs QML) após protótipo rápido.
  - Escolher bibliotecas para PDF/EPUB/MOBI e validar licença.
  - Definir persistência (preferência: SQLite) e esquema inicial.
  - Montar skeleton do projeto com CMake e estrutura acima.
  - Próximos releases: acompanhar `ROADMAP.md`.
  - Próxima: iniciar 0.2.0 (Anotações e Dicionário) conforme `ROADMAP.md`.

### Notas de Progresso Recentes
- v0.1.6 (interno): Otimização da renderização do chat. A conversão de Markdown para HTML foi movida do front-end (JavaScript, `marked.js`) para o back-end (C++, biblioteca `cmark`), simplificando o código do cliente e centralizando a lógica de formatação.
- v0.1.2: "Salvar como" (RF-28) implementado; refinamentos de leitura.
- v0.1.1: Navegação por combobox cobrindo todas as páginas de PDFs e TOC simples para todas as páginas; restauração de sessão; melhorias de status e zoom.
 - v0.1.3: TOC aprimorado com toolbar (modos "Páginas"/"Conteúdo") e navegação context-aware; splitter com tamanho inicial ~10% (TOC) / ~90% (visualizador). Branding/UI entregue: ícone do app (RF-48), título com nome do livro (RF-49) e splash screen (RF-50). Preferência de granularidade do zoom (Ctrl+roda) adicionada. Modos de seleção no PDF (texto/retângulo), cópia com toast e exportação de seleção para TXT/Markdown. Diálogo "Dados do leitor" com envio opcional para PHPList via `.env`.

## 12) Instruções de Build e Documentação

### Build da Aplicação (Linux)
- Pré-requisitos: CMake (>=3.16), compilador C++17, Qt6 (Widgets, PdfWidgets, Network), Doxygen (opcional).
- Comandos:
```bash
cmake -S . -B build
cmake --build build -j
# Executável gerado em: build/bin/genai_reader
```

### Geração da Documentação (Doxygen)
- Via target CMake:
```bash
cmake --build build --target docs
```
- Ou diretamente:
```bash
doxygen docs/Doxyfile
```
- Saída HTML em: `docs/build/html/index.html` (com treeview à esquerda).
