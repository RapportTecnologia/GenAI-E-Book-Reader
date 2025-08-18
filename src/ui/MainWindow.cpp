#ifdef USE_QT
#include "ui/MainWindow.h"
#include "ui/ViewerWidget.h"
#ifdef HAVE_QT_PDF
#include "ui/PdfViewerWidget.h"
#endif

#include <QApplication>
#include <QAction>
#include <QToolBar>
#include <QMenuBar>
#include <QStatusBar>
#include <QTreeWidget>
#include <QSplitter>
#include <QFileDialog>
#include <QPalette>
#include <QMessageBox>
#include <QFileInfo>
#include <algorithm>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), settings_("GenAI", "EBookReader") {
    buildUi();
    createActions();
    loadSettings();
    updateStatus();
}

MainWindow::~MainWindow() {
    saveSettings();
}

void MainWindow::buildUi() {
    viewer_ = new ViewerWidget(this);
    toc_ = new QTreeWidget(this);
    toc_->setHeaderHidden(true);

    splitter_ = new QSplitter(this);
    splitter_->addWidget(toc_);
    splitter_->addWidget(viewer_);
    splitter_->setStretchFactor(1, 1);

    setCentralWidget(splitter_);
    statusBar();
}

void MainWindow::createActions() {
    actOpen_ = new QAction(tr("Abrir"), this);
    actPrev_ = new QAction(tr("Anterior"), this);
    actNext_ = new QAction(tr("Próxima"), this);
    actZoomIn_ = new QAction(tr("Zoom +"), this);
    actZoomOut_ = new QAction(tr("Zoom -"), this);
    actZoomReset_ = new QAction(tr("Zoom 100%"), this);
    actToggleTheme_ = new QAction(tr("Tema claro/escuro"), this);

    actOpen_->setShortcut(QKeySequence::Open);
    actPrev_->setShortcut(Qt::Key_Left);
    actNext_->setShortcut(Qt::Key_Right);
    actZoomIn_->setShortcut(QKeySequence::ZoomIn);
    actZoomOut_->setShortcut(QKeySequence::ZoomOut);

    connect(actOpen_, &QAction::triggered, this, &MainWindow::openFile);
    connect(actPrev_, &QAction::triggered, this, &MainWindow::prevPage);
    connect(actNext_, &QAction::triggered, this, &MainWindow::nextPage);
    connect(actZoomIn_, &QAction::triggered, this, &MainWindow::zoomIn);
    connect(actZoomOut_, &QAction::triggered, this, &MainWindow::zoomOut);
    connect(actZoomReset_, &QAction::triggered, this, &MainWindow::zoomReset);
    connect(actToggleTheme_, &QAction::triggered, this, &MainWindow::toggleTheme);

    auto* tb = addToolBar(tr("Leitura"));
    tb->setObjectName("toolbar_reading");
    tb->addAction(actOpen_);
    tb->addSeparator();
    tb->addAction(actPrev_);
    tb->addAction(actNext_);
    tb->addSeparator();
    tb->addAction(actZoomOut_);
    tb->addAction(actZoomIn_);
    tb->addAction(actZoomReset_);
    tb->addSeparator();
    tb->addAction(actToggleTheme_);

    connect(toc_, &QTreeWidget::itemActivated, this, &MainWindow::onTocItemActivated);
}

void MainWindow::loadSettings() {
    restoreGeometry(settings_.value("ui/geometry").toByteArray());
    restoreState(settings_.value("ui/state").toByteArray());
    darkTheme_ = settings_.value("ui/dark", false).toBool();
    // apply zoom if viewer supports it (initial viewer_ is ViewerWidget)
    if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) {
        vw->setZoomFactor(settings_.value("view/zoom", 1.0).toDouble());
    }
    if (darkTheme_) applyDarkPalette(true);
}

void MainWindow::saveSettings() {
    settings_.setValue("ui/geometry", saveGeometry());
    settings_.setValue("ui/state", saveState());
    settings_.setValue("ui/dark", darkTheme_);
    double z = 1.0;
    if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) {
        z = vw->zoomFactor();
    }
#ifdef HAVE_QT_PDF
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
        z = pv->zoomFactor();
    }
#endif
    settings_.setValue("view/zoom", z);
}

void MainWindow::updateStatus() {
    unsigned int cur = 1, tot = 0; double z = 1.0;
    if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) {
        cur = vw->currentPage() == 0 ? 1u : vw->currentPage();
        tot = vw->totalPages();
        z = vw->zoomFactor();
    }
#ifdef HAVE_QT_PDF
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
        cur = pv->currentPage() == 0 ? 1u : pv->currentPage();
        tot = pv->totalPages();
        z = pv->zoomFactor();
    }
#endif
    statusBar()->showMessage(tr("Página %1 de %2 | Zoom %3%")
        .arg(cur)
        .arg(tot == 0 ? 100 : tot)
        .arg(int(z*100)));
}

void MainWindow::applyDarkPalette(bool enable) {
    QPalette pal;
    if (enable) {
        pal.setColor(QPalette::Window, QColor(37,37,38));
        pal.setColor(QPalette::WindowText, Qt::white);
        pal.setColor(QPalette::Base, QColor(30,30,30));
        pal.setColor(QPalette::AlternateBase, QColor(45,45,48));
        pal.setColor(QPalette::ToolTipBase, Qt::white);
        pal.setColor(QPalette::ToolTipText, Qt::white);
        pal.setColor(QPalette::Text, Qt::white);
        pal.setColor(QPalette::Button, QColor(45,45,48));
        pal.setColor(QPalette::ButtonText, Qt::white);
        pal.setColor(QPalette::BrightText, Qt::red);
        pal.setColor(QPalette::Highlight, QColor(38,79,120));
        pal.setColor(QPalette::HighlightedText, Qt::white);
    }
    QApplication::setPalette(pal);
}

void MainWindow::openFile() {
    const QString file = QFileDialog::getOpenFileName(this, tr("Abrir e-book"), QString(), tr("E-books (*.pdf *.epub);;Todos (*.*)"));
    if (file.isEmpty()) return;

    const QFileInfo fi(file);

#ifdef HAVE_QT_PDF
    if (fi.suffix().compare("pdf", Qt::CaseInsensitive) == 0) {
        // Replace current viewer with a PdfViewerWidget
        auto* newViewer = new PdfViewerWidget(this);
        QString err;
        if (!newViewer->openFile(file, &err)) {
            delete newViewer;
            QMessageBox::warning(this, tr("Erro"), err.isEmpty() ? tr("Falha ao abrir PDF.") : err);
            return;
        }

        // swap widget in splitter
        int idx = splitter_->indexOf(viewer_);
        splitter_->replaceWidget(idx, newViewer);
        viewer_->deleteLater();
        viewer_ = newViewer;

        // Build a simple TOC placeholder if no outline available (future: parse outline)
        toc_->clear();
        const unsigned int pages = newViewer->totalPages();
        for (unsigned int i = 1; i <= std::min(pages, 10u); ++i) {
            auto* item = new QTreeWidgetItem(QStringList{tr("Página %1").arg(i)});
            item->setData(0, Qt::UserRole, i);
            toc_->addTopLevelItem(item);
        }
        updateStatus();
        return;
    }
#endif

    // Fallback to dummy reader and placeholder ViewerWidget
    auto res = reader_.open(file.toStdString());
    if (!res.ok) {
        QMessageBox::warning(this, tr("Erro"), QString::fromStdString(res.message));
        return;
    }
    if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) {
        vw->setTotalPages(res.totalPages);
        vw->setCurrentPage(1);
    }
    toc_->clear();
    for (unsigned int i = 1; i <= 10; ++i) {
        auto* item = new QTreeWidgetItem(QStringList{tr("Capítulo %1").arg(i)});
        item->setData(0, Qt::UserRole, i*10u);
        toc_->addTopLevelItem(item);
    }
    updateStatus();
}

void MainWindow::nextPage() {
    unsigned int cur = 1, tot = 0;
    if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) {
        cur = vw->currentPage(); tot = vw->totalPages();
        if (cur < tot) vw->setCurrentPage(cur + 1);
    }
#ifdef HAVE_QT_PDF
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
        cur = pv->currentPage(); tot = pv->totalPages();
        if (cur < tot) pv->setCurrentPage(cur + 1);
    }
#endif
    updateStatus();
}

void MainWindow::prevPage() {
    if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) {
        if (vw->currentPage() > 1) vw->setCurrentPage(vw->currentPage() - 1);
    }
#ifdef HAVE_QT_PDF
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
        if (pv->currentPage() > 1) pv->setCurrentPage(pv->currentPage() - 1);
    }
#endif
    updateStatus();
}

void MainWindow::zoomIn() {
    if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) {
        vw->setZoomFactor(vw->zoomFactor() * 1.1);
    }
#ifdef HAVE_QT_PDF
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
        pv->setZoomFactor(pv->zoomFactor() * 1.1);
    }
#endif
    updateStatus();
}

void MainWindow::zoomOut() {
    if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) {
        vw->setZoomFactor(vw->zoomFactor() / 1.1);
    }
#ifdef HAVE_QT_PDF
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
        pv->setZoomFactor(pv->zoomFactor() / 1.1);
    }
#endif
    updateStatus();
}

void MainWindow::zoomReset() {
    if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) {
        vw->setZoomFactor(1.0);
    }
#ifdef HAVE_QT_PDF
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
        pv->setZoomFactor(1.0);
    }
#endif
    updateStatus();
}

void MainWindow::toggleTheme() {
    darkTheme_ = !darkTheme_;
    applyDarkPalette(darkTheme_);
}

void MainWindow::onTocItemActivated(QTreeWidgetItem* item, int) {
    bool ok = false;
    const unsigned int page = item->data(0, Qt::UserRole).toUInt(&ok);
    if (ok) {
        if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) {
            vw->setCurrentPage(page);
        }
#ifdef HAVE_QT_PDF
        if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
            pv->setCurrentPage(page);
        }
#endif
        updateStatus();
    }
}
#endif // USE_QT
