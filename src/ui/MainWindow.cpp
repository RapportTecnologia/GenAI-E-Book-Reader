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
#include <QComboBox>
#include <QDir>
#include <QPalette>
#include <QMessageBox>
#include <QFileInfo>
#include <QFile>
#include <algorithm>

#ifdef HAVE_QT_PDF
#if __has_include(<QPdfBookmarkModel>)
#include <QPdfBookmarkModel>
#define HAS_QPDF_BOOKMARK_MODEL 1
#endif
#endif

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), settings_("GenAI", "EBookReader") {
    buildUi();
    createActions();
    loadSettings();
    updateStatus();
}

void MainWindow::saveAs() {
    if (currentFilePath_.isEmpty() || !QFileInfo::exists(currentFilePath_)) {
        QMessageBox::information(this, tr("Salvar como"), tr("Nenhum arquivo aberto para salvar."));
        return;
    }
    const QString startDir = settings_.value("session/lastDir", QFileInfo(currentFilePath_).absolutePath()).toString();
    const QString suggested = QFileInfo(currentFilePath_).fileName();
    const QString target = QFileDialog::getSaveFileName(this, tr("Salvar como"), QDir(startDir).filePath(suggested), tr("E-books (*.pdf *.epub);;Todos (*.*)"));
    if (target.isEmpty()) return;

    if (QFileInfo::exists(target)) {
        const auto ret = QMessageBox::question(this, tr("Confirmar"), tr("O arquivo já existe. Deseja sobrescrever?"),
                                               QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ret != QMessageBox::Yes) return;
        if (!QFile::remove(target)) {
            QMessageBox::warning(this, tr("Erro"), tr("Não foi possível remover o arquivo de destino."));
            return;
        }
    }

    if (!QFile::copy(currentFilePath_, target)) {
        QMessageBox::warning(this, tr("Erro"), tr("Falha ao copiar para o destino."));
        return;
    }

    // Atualiza estado para o novo caminho (comportamento comum de 'Salvar como')
    currentFilePath_ = QFileInfo(target).absoluteFilePath();
    settings_.setValue("session/lastDir", QFileInfo(target).absolutePath());
    settings_.setValue("session/lastFile", currentFilePath_);
    statusBar()->showMessage(tr("Salvo em %1").arg(currentFilePath_), 3000);
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
    actSaveAs_ = new QAction(tr("Salvar como..."), this);
    actPrev_ = new QAction(tr("Anterior"), this);
    actNext_ = new QAction(tr("Próxima"), this);
    actZoomIn_ = new QAction(tr("Zoom +"), this);
    actZoomOut_ = new QAction(tr("Zoom -"), this);
    actZoomReset_ = new QAction(tr("Zoom 100%"), this);
    actToggleTheme_ = new QAction(tr("Tema claro/escuro"), this);

    actOpen_->setShortcut(QKeySequence::Open);
    actSaveAs_->setShortcut(QKeySequence::SaveAs);
    actPrev_->setShortcut(Qt::Key_Left);
    actNext_->setShortcut(Qt::Key_Right);
    actZoomIn_->setShortcut(QKeySequence::ZoomIn);
    actZoomOut_->setShortcut(QKeySequence::ZoomOut);

    connect(actOpen_, &QAction::triggered, this, &MainWindow::openFile);
    connect(actSaveAs_, &QAction::triggered, this, &MainWindow::saveAs);
    connect(actPrev_, &QAction::triggered, this, &MainWindow::prevPage);
    connect(actNext_, &QAction::triggered, this, &MainWindow::nextPage);
    connect(actZoomIn_, &QAction::triggered, this, &MainWindow::zoomIn);
    connect(actZoomOut_, &QAction::triggered, this, &MainWindow::zoomOut);
    connect(actZoomReset_, &QAction::triggered, this, &MainWindow::zoomReset);
    connect(actToggleTheme_, &QAction::triggered, this, &MainWindow::toggleTheme);

    // Menu Arquivo
    auto* menuArquivo = menuBar()->addMenu(tr("&Arquivo"));
    menuArquivo->addAction(actOpen_);
    menuArquivo->addAction(actSaveAs_);

    auto* tb = addToolBar(tr("Leitura"));
    tb->setObjectName("toolbar_reading");
    tb->addAction(actOpen_);
    tb->addAction(actSaveAs_);
    tb->addSeparator();
    tb->addAction(actPrev_);
    tb->addAction(actNext_);
    // Page combobox (RF-14)
    pageCombo_ = new QComboBox(tb);
    pageCombo_->setEditable(false);
    pageCombo_->setMinimumContentsLength(6);
    tb->addWidget(pageCombo_);
    connect(pageCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx){
        const unsigned int p = static_cast<unsigned int>(idx + 1);
        if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) vw->setCurrentPage(p);
#ifdef HAVE_QT_PDF
        if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) pv->setCurrentPage(p);
#endif
        updateStatus();
    });
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

    // Optional: reopen last file on start (RF-20)
    const QString lastFile = settings_.value("session/lastFile").toString();
    if (!lastFile.isEmpty() && QFileInfo::exists(lastFile)) {
        openPath(lastFile);
    }
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
    updatePageCombo();
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
    const QString startDir = settings_.value("session/lastDir", QDir::homePath()).toString();
    const QString file = QFileDialog::getOpenFileName(this, tr("Abrir e-book"), startDir, tr("E-books (*.pdf *.epub);;Todos (*.*)"));
    if (file.isEmpty()) return;
    openPath(file);
}

bool MainWindow::openPath(const QString& file) {
    const QFileInfo fi(file);

#ifdef HAVE_QT_PDF
    if (fi.suffix().compare("pdf", Qt::CaseInsensitive) == 0) {
        // Replace current viewer with a PdfViewerWidget
        auto* newViewer = new PdfViewerWidget(this);
        QString err;
        if (!newViewer->openFile(file, &err)) {
            delete newViewer;
            QMessageBox::warning(this, tr("Erro"), err.isEmpty() ? tr("Falha ao abrir PDF.") : err);
            return false;
        }

        // swap widget in splitter
        int idx = splitter_->indexOf(viewer_);
        splitter_->replaceWidget(idx, newViewer);
        viewer_->deleteLater();
        viewer_ = newViewer;

        // Build TOC: prefer PDF bookmarks (capítulos/subcapítulos) when available; fallback to all pages
        toc_->clear();
#ifdef HAS_QPDF_BOOKMARK_MODEL
        auto* bm = new QPdfBookmarkModel(toc_);
        bm->setDocument(newViewer->document());
        // recursive lambda to populate tree
        std::function<void(QTreeWidgetItem*, const QModelIndex&)> addChildren = [&](QTreeWidgetItem* parentItem, const QModelIndex& parentIdx){
            const int rows = bm->rowCount(parentIdx);
            for (int r=0; r<rows; ++r) {
                const QModelIndex idx = bm->index(r, 0, parentIdx);
                const QString title = bm->data(idx, Qt::DisplayRole).toString();
                QVariant pageVar = bm->data(idx, Qt::UserRole); // fallback role
                // Some Qt versions expose page via specific role; try common ones
                if (!pageVar.isValid()) pageVar = bm->data(idx, Qt::UserRole + 1);
                if (!pageVar.isValid()) pageVar = bm->data(idx, Qt::UserRole + 2);
                int pageZeroBased = pageVar.isValid() ? pageVar.toInt() : -1;
                auto* item = new QTreeWidgetItem(QStringList{ title.isEmpty() ? tr("Seção") : title });
                if (pageZeroBased >= 0) item->setData(0, Qt::UserRole, static_cast<unsigned int>(pageZeroBased + 1));
                if (parentItem) parentItem->addChild(item); else toc_->addTopLevelItem(item);
                addChildren(item, idx);
            }
        };
        addChildren(nullptr, QModelIndex());
        if (toc_->topLevelItemCount() == 0) {
#endif
            const unsigned int pages = newViewer->totalPages();
            for (unsigned int i = 1; i <= pages; ++i) {
                auto* item = new QTreeWidgetItem(QStringList{tr("Página %1").arg(i)});
                item->setData(0, Qt::UserRole, i);
                toc_->addTopLevelItem(item);
            }
#ifdef HAS_QPDF_BOOKMARK_MODEL
        }
#endif
        // Fit to width initially (RF-13)
        // QPdfView supports FitToWidth via ZoomMode; expose through public API if needed.
        newViewer->setZoomFactor(1.0); // ensure valid
        // Save last dir/file
        settings_.setValue("session/lastDir", fi.absolutePath());
        settings_.setValue("session/lastFile", fi.absoluteFilePath());
        currentFilePath_ = fi.absoluteFilePath();
        updateStatus();
        return true;
    }
#endif

    // Fallback to dummy reader and placeholder ViewerWidget
    auto res = reader_.open(file.toStdString());
    if (!res.ok) {
        QMessageBox::warning(this, tr("Erro"), QString::fromStdString(res.message));
        return false;
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
    settings_.setValue("session/lastDir", fi.absolutePath());
    settings_.setValue("session/lastFile", fi.absoluteFilePath());
    currentFilePath_ = fi.absoluteFilePath();
    updateStatus();
    return true;
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

void MainWindow::updatePageCombo() {
    if (!pageCombo_) return;
    unsigned int cur = 1, tot = 0;
    if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) { cur = vw->currentPage(); tot = vw->totalPages(); }
#ifdef HAVE_QT_PDF
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) { cur = pv->currentPage(); tot = pv->totalPages(); }
#endif
    const unsigned int total = tot == 0 ? 100u : tot;
    // Update items only if count mismatches to avoid flicker
    if (pageCombo_->count() != int(total)) {
        pageCombo_->blockSignals(true);
        pageCombo_->clear();
        for (unsigned int i=1; i<=total; ++i) {
            pageCombo_->addItem(QString::number(i));
        }
        pageCombo_->blockSignals(false);
    }
    const unsigned int curPage = (cur == 0 ? 1u : cur);
    const int desiredIndex = int(curPage) - 1;
    if (pageCombo_->currentIndex() != desiredIndex && desiredIndex >= 0 && desiredIndex < pageCombo_->count()) {
        pageCombo_->blockSignals(true);
        pageCombo_->setCurrentIndex(desiredIndex);
        pageCombo_->blockSignals(false);
    }
}

#endif // USE_QT
