#pragma once

#ifdef USE_QT
#include <QMainWindow>
#include <QSettings>
class QTreeWidget;
class QTreeWidgetItem;
class QSplitter;
class QAction;
class QToolBar;
class QStatusBar;
class QComboBox;
class ViewerWidget;
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
    void nextPage();
    void prevPage();
    void zoomIn();
    void zoomOut();
    void zoomReset();
    void toggleTheme();
    void onTocItemActivated(QTreeWidgetItem* item, int column);

private:
    void buildUi();
    void createActions();
    void loadSettings();
    void saveSettings();
    void updateStatus();
    void applyDarkPalette(bool enable);
    void updatePageCombo();
    bool openPath(const QString& filePath);

private:
    QWidget* viewer_ {nullptr}; // can be ViewerWidget or PdfViewerWidget
    QTreeWidget* toc_ {nullptr};
    QSplitter* splitter_ {nullptr};

    QAction* actOpen_ {nullptr};
    QAction* actPrev_ {nullptr};
    QAction* actNext_ {nullptr};
    QAction* actZoomIn_ {nullptr};
    QAction* actZoomOut_ {nullptr};
    QAction* actZoomReset_ {nullptr};
    QAction* actToggleTheme_ {nullptr};
    QComboBox* pageCombo_ {nullptr};

    QSettings settings_;
    bool darkTheme_ {false};

    genai::DummyReader reader_;
};
#endif // USE_QT
