#pragma once

/**
 * \file MainWindow.h
 * \brief Janela principal do GenAI E-Book Reader.
 *
 * Esta classe organiza a interface principal do aplicativo: barra de menus e ferramentas,
 * painel de navegação (TOC), visualizador de PDF, barra de busca e integrações com IA (chat,
 * sumarização, sinônimos e RAG). Para iniciantes, pense nela como o "cérebro" da interface,
 * que conecta os botões e menus às ações reais.
 *
 * Principais responsabilidades:
 * - Abrir, fechar e navegar por documentos (PDF).
 * - Mostrar e navegar pelo índice/TOC usando QPdfBookmarkModel.
 * - Construir e gerenciar a busca (texto simples e semântica com embeddings).
 * - Integrar com o cliente de LLM para chat, sumarização e outras ações inteligentes.
 * - Persistir preferências e histórico (via QSettings).
 */

#include <QMainWindow>
#include <QSettings>
#include <QString>
#include <QImage>
#include <QJsonArray>
#include <QJsonObject>
#include <QPixmap>
#include <QLabel>

// Forward declarations for UI types used as pointers in this header
class QToolButton;
class QSpinBox;
class QMenu;
class QAction;
class QTreeWidget;
class QTreeWidgetItem;
class QSplitter;
class QToolBar;
class QStatusBar;
class QComboBox;
class QLineEdit;
class QPushButton;
class QShortcut;
class ViewerWidget;
class QNetworkAccessManager;
class PdfViewerWidget;
class LlmClient;
class SummaryDialog;
class ChatDock;
class DictionarySettingsDialog;
class RecentFilesDialog;
class QPdfBookmarkModel;
class OpfDialog;
class OpfGenAiDialog;
class SearchProgressDialog;

#include "ui/OpfStore.h"

#include "reader/Reader.h"

/**
 * \class MainWindow
 * \brief Janela principal do aplicativo.
 *
 * Fornece ações de abertura de arquivo, navegação de páginas, zoom, alternância de tema
 * e um painel de TOC (índice) clicável. Também expõe ações de IA como resumo, sinônimos e chat.
 * Os métodos privados de utilidade cuidam do estado de UI, persistência e integração com o
 * pipeline de busca semântica (RAG).
 * \ingroup ui
 */
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    /** \brief Abre um arquivo pelo caminho absoluto. Retorna true em caso de sucesso. */
    bool openPath(const QString& filePath);
    /** \brief Abre um diálogo de arquivo para o usuário escolher um documento. */
    void openFile();
    ~MainWindow() override;

private slots:
    /** \brief Salva o documento/processado com outro nome (quando aplicável). */
    void saveAs();
    /** \brief Fecha o documento atual e limpa estados de UI. */
    void closeDocument();
    /** \brief Avança para a próxima página. */
    void nextPage();
    /** \brief Volta para a página anterior. */
    void prevPage();
    /** \brief Aumenta o zoom do visualizador. */
    void zoomIn();
    /** \brief Reduz o zoom do visualizador. */
    void zoomOut();
    /** \brief Restaura o zoom para o padrão. */
    void zoomReset();
    /** \brief Alterna entre tema claro/escuro. */
    void toggleTheme();
    /** \brief Slot chamado quando um item do TOC é ativado (clique ou Enter). */
    void onTocItemActivated(QTreeWidgetItem* item, int column);
    /** \brief Edita metadados do leitor/usuário (nome, email) usados em algumas ações. */
    void editReaderData();
    /** \brief Seleciona o item anterior no TOC. */
    void onTocPrev();
    /** \brief Seleciona o próximo item no TOC. */
    void onTocNext();
    /** \brief Coloca o TOC no modo de páginas. */
    void setTocModePages();
    /** \brief Coloca o TOC no modo de capítulos. */
    void setTocModeChapters();
    // Edit menu handlers
    /** \brief Habilita seleção de texto no PDF. */
    void enableTextSelection();
    /** \brief Habilita seleção retangular (para OCR ou captura). */
    void enableRectSelection();
    /** \brief Seleção automática (tenta usar texto, cai para retângulo quando necessário). */
    void enableAutoSelection();
    /** \brief Copia o conteúdo da seleção atual para a área de transferência. */
    void copySelection();
    /** \brief Salva a seleção como arquivo .txt. */
    void saveSelectionTxt();
    /** \brief Salva a seleção como arquivo .md (Markdown). */
    void saveSelectionMd();
    // Preferences
    /** \brief Alterna preferência de zoom pela roda do mouse. */
    void setWheelZoomPreference();
    /** \brief Abre diálogo de configuração do LLM (provider, modelo, chave). */
    void openLlmSettings();
    /** \brief Abre diálogo de configuração de embeddings/indexação. */
    void openEmbeddingSettings();
    /** \brief Abre diálogo de configuração de dicionário. */
    void openDictionarySettings();
    /** \brief Mostra/oculta o painel de chat com IA. */
    void showChatPanel();
    /** \brief Envia mensagem ao chat. */
    void onChatSendMessage(const QString& text);
    /** \brief Salva a transcrição do chat. */
    void onChatSaveTranscript(const QString& text);
    /** \brief Gera um resumo da transcrição do chat via IA. */
    void onChatSummarizeTranscript(const QString& text);
    /** \brief Exibe o diálogo Sobre. */
    void showAboutDialog();
    /** \brief Atualiza UI ao mudar a página atual. */
    void onCurrentPageChanged(int page);
    /** \brief Mostra diálogo de arquivos recentes. */
    void showRecentFilesDialog();
    /** \brief Mostra o tutorial inicial do aplicativo. */
    void showTutorialDialog();
    /** \brief Imprime o documento inteiro atual. */
    void printDocument();

    // AI actions
    /** \brief Solicita sinônimos para a seleção/palavra destacada. */
    void onRequestSynonyms(const QString& wordOrLocution);
    /** \brief Solicita sumarização do texto selecionado. */
    void onRequestSummarize(const QString& text);
    /** \brief Envia o texto selecionado para o chat (contexto). */
    void onRequestSendToChat(const QString& text);
    /** \brief Envia imagem (captura) para o chat multimodal. */
    void onRequestSendImageToChat(const QImage& image);
    /** \brief Consulta termo no dicionário. */
    void onDictionaryLookup(const QString& term);
    /** \brief Reprocessa embeddings/índice para o documento atual. */
    void onRequestRebuildEmbeddings();
    /** \brief Gera resumo do documento inteiro via IA. */
    void onRequestSummarizeDocument();
    /** \brief Abre o diálogo de metadados OPF. */
    void openOpfMetadataDialog();
    /** \brief Completa metadados OPF faltantes via IA (JSON estrito, sem inventar). */
    void completeOpfWithGenAi();
    /** \brief Recria o OPF coletando dados de múltiplas fontes (PDF, nome do arquivo, Google, Amazon, IA) e permite mesclar. */
    void regenerateOpfInteractive();

private:
    void buildUi();
    void createActions();
    void loadSettings();
    void saveSettings();
    void updateStatus();
    void focusSearchBar();
    void onSearchTriggered();
    void onSearchNext();
    void onSearchPrev();
    void onSearchMetricCosine();
    void onSearchMetricDot();
    void onSearchMetricL2();
    void onSearchTopKChanged(int value);
    void applyDarkPalette(bool enable);
    void updatePageCombo();
    void showLongAlert(const QString& title, const QString& longText);
    // Recent files helpers
    void addRecentFile(const QString& absPath);
    void rebuildRecentMenu();

    // Validation and integration helpers
    bool validateReaderInputs(const QString& name, const QString& email, QString* errorMsg) const;
    void applyDefaultSplitterSizesIfNeeded();
    // Chat persistence helpers
    void saveChatForCurrentFile();
    void loadChatForFile(const QString& absPath);
    // Chat sessions (history) helpers
    QJsonArray readChatSessions(const QString& filePath);
    void writeChatSessions(const QString& filePath, const QJsonArray& sessions);
    void showSavedChatsPicker();
    // LLM function calling dispatcher
    void handleLlmToolCalls(const QJsonArray& toolCalls);
    // Tool implementations
    void toolProposeSearch(const QString& query);
    void toolNextResult();
    void toolPrevResult();
    void toolGotoPage(int page);

    // Search progress modal helpers
    void beginSearchProgress(const QString& title, const QString& firstLine = QString());
    void logSearchProgress(const QString& line);
    void endSearchProgress();

private:
    // Search helpers
    bool ensurePagesTextLoaded();
    QList<int> plainTextSearchPages(const QString& needle, int maxResults = 20);
    QList<int> semanticSearchPages(const QString& query, int k = 5);
    QString sha1(const QString& s) const;
    struct IndexPaths { QString base; QString binPath; QString idsPath; QString metaPath; };
    bool getIndexPaths(IndexPaths* out) const;
    bool computeIndexPathsFor(const QString& filePath, IndexPaths* out) const;
    void loadSearchOptionsFromSettings();
    void saveSearchOptionsToSettings(const QString& metricKey, int topK);
    // RAG pipeline helpers
    void startRagSearch(const QString& userQuery);
    void ensureIndexAvailableThen(const QString& translatedQuery);
    void continueRagAfterEnsureIndex(const QString& translatedQuery);
    QString detectDocumentLanguageSample() const;
    void detectDocumentLanguageAsync(std::function<void(QString)> onLang);
    void translateQueryIfNeededAsync(const QString& query, const QString& docLang, std::function<void(QString)> onReady);
    void maybeAskGenerateOpf();
    void generateOpfWithLlmAsync(const QString& absPdfPath);
    OpfData buildOpfFromPdfMeta(const QString& absPdfPath) const;
    // UI helper: after writing OPF, reload the dialog if it's open so fields are not stale
    void reloadOpfDialogFromDiskIfOpen(const QString& opfPath);
    // Apply current OPF generation status to dialog if open
    void applyOpfBusyStatusToDialogIfOpen();

    // Build a system message including the current e-book metadata (title, author, description, summary)
    // to be prepended to chat conversations with the LLM.
    QString buildOpfSystemPrompt() const;

    QWidget* viewer_ {nullptr}; // can be ViewerWidget or PdfViewerWidget
    QTreeWidget* toc_ {nullptr};
    QPdfBookmarkModel* tocModel_ {nullptr};
    QWidget* tocPanel_ {nullptr};
    QToolBar* tocToolBar_ {nullptr};
    QSplitter* splitter_ {nullptr};

    QAction* actOpen_ {nullptr};
    QAction* actSaveAs_ {nullptr};
    QAction* actClose_ {nullptr};
    QAction* actPrev_ {nullptr};
    QAction* actNext_ {nullptr};
    QAction* actZoomIn_ {nullptr};
    QAction* actZoomOut_ {nullptr};
    QAction* actZoomReset_ {nullptr};
    QAction* actToggleTheme_ {nullptr};
    QAction* actQuit_ {nullptr};
    QAction* actReaderData_ {nullptr};
    QAction* actChat_ {nullptr};
    QAction* actPrintDoc_ {nullptr};
    // Edit actions
    QAction* actSelText_ {nullptr};
    QAction* actSelRect_ {nullptr};
    QAction* actSelAuto_ {nullptr};
    QAction* actSelCopy_ {nullptr};
    QAction* actSelSaveTxt_ {nullptr};
    QAction* actSelSaveMd_ {nullptr};
    // Preferences
    QAction* actWheelZoomPref_ {nullptr};
    QAction* actRecentConfig_ {nullptr};
    QAction* actLlmSettings_ {nullptr};
    QAction* actEmbeddingSettings_ {nullptr};
    QAction* actDictionarySettings_ {nullptr};
    QAction* actAbout_ {nullptr};
    QAction* actTutorial_ {nullptr};
    QAction* actOpfDialog_ {nullptr};
    // TOC toolbar actions
    QAction* actTocModePages_ {nullptr};
    QAction* actTocModeChapters_ {nullptr};
    QAction* actTocPrev_ {nullptr};
    QAction* actTocNext_ {nullptr};
    bool tocPagesMode_ {true};
    QComboBox* pageCombo_ {nullptr};
    QLabel* totalPagesLabel_ {nullptr};

    // Search UI
    QToolBar* searchToolBar_ {nullptr};
    QLineEdit* searchEdit_ {nullptr};
    QPushButton* searchButton_ {nullptr};
    QPushButton* searchPrevButton_ {nullptr};
    QPushButton* searchNextButton_ {nullptr};
    QToolButton* searchOptionsButton_ {nullptr};
    QMenu* searchOptionsMenu_ {nullptr};
    QAction* actMetricCosine_ {nullptr};
    QAction* actMetricDot_ {nullptr};
    QAction* actMetricL2_ {nullptr};
    QSpinBox* topKSpin_ {nullptr};
    QShortcut* slashShortcut_ {nullptr};
    // Threshold controls
    class QDoubleSpinBox* simThresholdSpin_ {nullptr}; // for cosine/dot: minimum similarity
    class QDoubleSpinBox* l2MaxDistSpin_ {nullptr};    // for L2: maximum distance

    QSettings settings_;
    bool darkTheme_ {false};
    // Recent files menu
    QMenu* menuRecent_ {nullptr};
    QAction* actRecentDialog_ {nullptr};

private slots:
    // Recent files slots
    void openRecentFile();
    void configureRecentDialogCount();

private:
    QString currentFilePath_;

    // Title/button UI and actions
    QToolButton* titleButton_ {nullptr};
    QMenu* titleMenu_ {nullptr};
    QAction* actTitleOpenDir_ {nullptr};
    QAction* actTitleAddToCalibre_ {nullptr};
    QAction* actTitleRename_ {nullptr};
    void updateTitleWidget();
    void showCurrentPathInfo();
    void onTitleOpenDir();
    void onTitleAddToCalibre();
    void onTitleRenameFile();
    bool migrateEmbeddingsForPathChange(const QString& oldPath, const QString& newPath, QString* errorMsg);

    genai::DummyReader reader_;

    QNetworkAccessManager* netManager_ {nullptr};

    // AI integration
    LlmClient* llm_ {nullptr};
    SummaryDialog* summaryDlg_ {nullptr};
    OpfDialog* opfDialog_ {nullptr};
    ChatDock* chatDock_ {nullptr};
    SearchProgressDialog* searchDlg_ {nullptr};

    // Cache for plain text pages of current PDF
    QStringList pagesText_;
    bool pagesTextLoaded_ {false};

    // Search results state
    QList<int> searchResultsPages_;
    int searchResultIdx_ {-1};

    // Pending state for async RAG
    QString pendingRagQuery_;

    // OPF generation status (for UI feedback in OpfDialog)
    bool opfGenInProgress_ {false};
    QString opfGenStatus_;
};
