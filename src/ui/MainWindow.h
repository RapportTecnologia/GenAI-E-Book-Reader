#pragma once

#include <QMainWindow>
#include <QSettings>
#include <QString>
class QTreeWidget;
class QTreeWidgetItem;
class QSplitter;
class QAction;
class QToolBar;
class QStatusBar;
class QComboBox;
class ViewerWidget;
class QNetworkAccessManager;
class PdfViewerWidget;

#include "reader/Reader.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void openFile();
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

private:
    void buildUi();
    void createActions();
    void loadSettings();
    void saveSettings();
    void updateStatus();
    void applyDarkPalette(bool enable);
    void updatePageCombo();
    bool openPath(const QString& filePath);
    // Recent files helpers
    void addRecentFile(const QString& absPath);
    void rebuildRecentMenu();

    // Validation and integration helpers
    bool validateReaderInputs(const QString& name, const QString& email, QString* errorMsg) const;
    QMap<QString, QString> loadEnvConfig() const; // reads .env near executable or CWD
    void submitReaderDataToPhpList(const QString& name, const QString& email, const QString& whatsapp);
    void applyDefaultSplitterSizesIfNeeded();

private:
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
    // TOC toolbar actions
    QAction* actTocModePages_ {nullptr};
    QAction* actTocModeChapters_ {nullptr};
    QAction* actTocPrev_ {nullptr};
    QAction* actTocNext_ {nullptr};
    bool tocPagesMode_ {true};
    QComboBox* pageCombo_ {nullptr};

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

    genai::DummyReader reader_;

    QNetworkAccessManager* netManager_ {nullptr};
};
