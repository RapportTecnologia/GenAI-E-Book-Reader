#pragma once

#ifdef USE_QT
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
#ifdef HAVE_QT_NETWORK
class QNetworkAccessManager;
#endif
#ifdef HAVE_QT_PDF
class PdfViewerWidget;
#endif

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

private:
    void buildUi();
    void createActions();
    void loadSettings();
    void saveSettings();
    void updateStatus();
    void applyDarkPalette(bool enable);
    void updatePageCombo();
    bool openPath(const QString& filePath);

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
    // TOC toolbar actions
    QAction* actTocModePages_ {nullptr};
    QAction* actTocModeChapters_ {nullptr};
    QAction* actTocPrev_ {nullptr};
    QAction* actTocNext_ {nullptr};
    bool tocPagesMode_ {true};
    QComboBox* pageCombo_ {nullptr};

    QSettings settings_;
    bool darkTheme_ {false};

    QString currentFilePath_;

    genai::DummyReader reader_;

#ifdef HAVE_QT_NETWORK
    QNetworkAccessManager* netManager_ {nullptr};
#endif
};
#endif // USE_QT
