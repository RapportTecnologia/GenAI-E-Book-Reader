#ifdef USE_QT
#ifdef HAVE_QT_PDF
#include "ui/PdfViewerWidget.h"

#include <QPdfDocument>
#include <QPdfView>
#include <QPdfPageNavigator>
#include <QPointF>
#include <QVBoxLayout>
#include <QFileInfo>

PdfViewerWidget::PdfViewerWidget(QWidget* parent)
    : QWidget(parent), doc_(new QPdfDocument(this)), view_(new QPdfView(this)) {
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0,0,0,0);
    lay->addWidget(view_);
    view_->setDocument(doc_);
    view_->setPageMode(QPdfView::PageMode::SinglePage);
    view_->setZoomMode(QPdfView::ZoomMode::Custom);
    navigation_ = view_->pageNavigator();
}

PdfViewerWidget::~PdfViewerWidget() {
}

bool PdfViewerWidget::openFile(const QString& path, QString* errorOut) {
    const auto status = doc_->load(path);
    if (status != static_cast<QPdfDocument::Error>(0)) {
        if (errorOut) *errorOut = QString::fromLatin1("Falha ao abrir PDF (%1)").arg(static_cast<int>(status));
        return false;
    }
    view_->setPageMode(QPdfView::PageMode::SinglePage);
    if (navigation_) navigation_->jump(0, QPointF(), 0);
    return true;
}

void PdfViewerWidget::setZoomFactor(double factor) {
    if (!view_) return;
    if (factor < 0.25) factor = 0.25;
    if (factor > 4.0) factor = 4.0;
    view_->setZoomFactor(factor);
}

double PdfViewerWidget::zoomFactor() const {
    return view_ ? view_->zoomFactor() : 1.0;
}

void PdfViewerWidget::nextPage() {
    if (!navigation_ || !doc_) return;
    if (navigation_->currentPage() + 1 < doc_->pageCount()) {
        navigation_->jump(navigation_->currentPage() + 1, QPointF(), 0);
    }
}

void PdfViewerWidget::prevPage() {
    if (!navigation_) return;
    if (navigation_->currentPage() > 0) {
        navigation_->jump(navigation_->currentPage() - 1, QPointF(), 0);
    }
}

unsigned int PdfViewerWidget::totalPages() const {
    return doc_ ? static_cast<unsigned int>(doc_->pageCount()) : 0u;
}

unsigned int PdfViewerWidget::currentPage() const {
    if (!navigation_) return 0u;
    return static_cast<unsigned int>(navigation_->currentPage() + 1); // 1-based for UI
}

void PdfViewerWidget::setCurrentPage(unsigned int page) {
    if (!navigation_ || !doc_) return;
    if (page == 0) page = 1;
    const int idx = static_cast<int>(page) - 1;
    if (idx >= 0 && doc_ && idx < doc_->pageCount()) {
        navigation_->jump(idx, QPointF(), 0);
    }
}

#endif // HAVE_QT_PDF
#endif // USE_QT

