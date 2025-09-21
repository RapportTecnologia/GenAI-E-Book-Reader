#pragma once

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
class QLabel;
class ViewerWidget;
class QNetworkAccessManager;
class PdfViewerWidget;
class LlmClient;
class SummaryDialog;
class ChatDock;
class DictionarySettingsDialog;

#include "reader/Reader.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    bool openPath(const QString& filePath);
    void openFile();
    ~MainWindow() override;

private slots:
    void saveAs();
    void closeDocument();
    void nextPage();
    void prevPage();
    void zoomIn();
    void zoomOut();
    void zoomReset();
    void toggleTheme();
    void onTocItemActivated(QTreeWidgetItem* item, int column);
    void editReaderData();
    void onTocPrev();
    void onTocNext();
    void setTocModePages();
    void setTocModeChapters();
    // Edit menu handlers
    void enableTextSelection();
    void enableRectSelection();
    void enableAutoSelection();
    void copySelection();
    void saveSelectionTxt();
    void saveSelectionMd();
    // Preferences
    void setWheelZoomPreference();
    void openLlmSettings();
    void openEmbeddingSettings();
    void openDictionarySettings();
    void showChatPanel();
    void onChatSendMessage(const QString& text);
    void onChatSaveTranscript(const QString& text);
    void onChatSummarizeTranscript(const QString& text);
    void showAboutDialog();
    void onCurrentPageChanged(int page);
    void showTutorialDialog();

    // AI actions
    void onRequestSynonyms(const QString& wordOrLocution);
    void onRequestSummarize(const QString& text);
    void onRequestSendToChat(const QString& text);
    void onRequestSendImageToChat(const QImage& image);
    void onDictionaryLookup(const QString& term);
    void onRequestRebuildEmbeddings();

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
    QMap<QString, QString> loadEnvConfig() const; // reads .env near executable or CWD
    void submitReaderDataToPhpList(const QString& name, const QString& email, const QString& whatsapp);
    void applyDefaultSplitterSizesIfNeeded();
    // Chat persistence helpers
    void saveChatForCurrentFile();
    void loadChatForFile(const QString& absPath);
    // Chat sessions (history) helpers
    QJsonArray readChatSessions(const QString& filePath);
    void writeChatSessions(const QString& filePath, const QJsonArray& sessions);
    void showSavedChatsPicker();

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

    QWidget* viewer_ {nullptr}; // can be ViewerWidget or PdfViewerWidget
    QTreeWidget* toc_ {nullptr};
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
    enum { MaxRecentMenuItems = 6 };
    QAction* recentActs_[MaxRecentMenuItems] {};

private slots:
    // Recent files slots
    void openRecentFile();
    void showRecentDialog();
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
    ChatDock* chatDock_ {nullptr};

    // Cache for plain text pages of current PDF
    QStringList pagesText_;
    bool pagesTextLoaded_ {false};

    // Search results state
    QList<int> searchResultsPages_;
    int searchResultIdx_ {-1};

    // Pending state for async RAG
    QString pendingRagQuery_;
};
