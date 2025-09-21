#include "PdfViewerWidget.h"

#include <QWidget>
#include <QPdfDocument>
#include <QPdfView>
#include <QVBoxLayout>
#include <QPdfPageNavigator>
// Try to include QPdfLinkModel if available (Qt 6.4+)
// For Qt 5.15, we don't have QPdfLinkModel, so we'll implement a different approach
#include <QGuiApplication>
#include <QClipboard>
#include <QScrollBar>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QImageWriter>
#include <QProcess>
#include <QStandardPaths>
#include <QDir>
#include <QUuid>
#include <QPixmap>
#include <QStringList>
#include <QVariantAnimation>
#include <QEasingCurve>
#include <QToolTip>
#include <QCursor>
#include <QLabel>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include <QMenu>
#include <QRegularExpression>
#if __has_include(<QtPdf/QPdfSelection>)
#  include <QtPdf/QPdfSelection>
#  define HAS_QPDF_SELECTION 1
#elif __has_include(<QPdfSelection>)
#  include <QPdfSelection>
#  define HAS_QPDF_SELECTION 1
#endif

PdfViewerWidget::PdfViewerWidget(QWidget* parent)
    : QWidget(parent), doc_(new QPdfDocument(this)), view_(new QPdfView(this)) {
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0,0,0,0);
    lay->addWidget(view_);
    view_->setDocument(doc_);
    // Enable continuous multi-page viewing for seamless vertical scrolling
    view_->setPageMode(QPdfView::PageMode::MultiPage);
    view_->setZoomMode(QPdfView::ZoomMode::Custom);
    navigation_ = view_->pageNavigator();
    // Ensure the widget and internal view can receive keyboard focus
    setFocusPolicy(Qt::StrongFocus);
    view_->setFocusPolicy(Qt::StrongFocus);
    // Intercept key events before QPdfView handles them
    view_->installEventFilter(this);
    if (view_->viewport()) view_->viewport()->installEventFilter(this);
    if (auto* vbar = view_->verticalScrollBar()) {
        connect(vbar, &QScrollBar::valueChanged, this, [this](int v){ emit scrollChanged(v); });
    }
    if (navigation_) {
        connect(navigation_, &QPdfPageNavigator::currentPageChanged, this, [this](int page) {
            // O navegador é 0-based, a UI é 1-based
            emit pageChanged(page + 1);
        });
    }
    rubber_ = new QRubberBand(QRubberBand::Rectangle, view_->viewport());
}

QString PdfViewerWidget::selectionText(bool* ok) {
    if (ok) *ok = false;
    QString out;
    // Priority 1: Native text selection (for Auto and Text modes)
    if (selMode_ == SelectionMode::Auto || selMode_ == SelectionMode::Text) {
        if (!selectedText_.trimmed().isEmpty()) {
            out = selectedText_.trimmed();
            if (ok) *ok = true;
            return out;
        }
    }

    // Priority 2: OCR from rectangular selection (for all modes if no text was found)
    if (selRect_.isValid() && !selRect_.isEmpty()) {
        bool ocrOk = false;
        out = ocrSelectionText(&ocrOk).trimmed();
        if (ok) *ok = ocrOk && !out.isEmpty();
        return out;
    }
    return out;
}

PdfViewerWidget::~PdfViewerWidget() {
}

bool PdfViewerWidget::openFile(const QString& path, QString* errorOut) {
    const auto status = doc_->load(path);
    if (status != static_cast<QPdfDocument::Error>(0)) {
        if (errorOut) *errorOut = QString::fromLatin1("Falha ao abrir PDF (%1)").arg(static_cast<int>(status));
        return false;
    }
    // Keep multi-page mode active after load
    view_->setPageMode(QPdfView::PageMode::MultiPage);
    if (navigation_) navigation_->jump(0, QPointF(), 0);

    return true;
}

void PdfViewerWidget::setZoomFactor(double factor) {
    if (!view_) return;
    if (factor < 0.25) factor = 0.25;
    if (factor > 4.0) factor = 4.0;
    // Switch to Custom mode so factor takes effect even after FitToWidth
    view_->setZoomMode(QPdfView::ZoomMode::Custom);
    view_->setZoomFactor(factor);
    emit zoomFactorChanged(view_->zoomFactor());
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

void PdfViewerWidget::fitToWidth() {
    if (!view_) return;
    view_->setZoomMode(QPdfView::ZoomMode::FitToWidth);
    emit zoomFactorChanged(view_->zoomFactor());
}

void PdfViewerWidget::keyPressEvent(QKeyEvent* event) {
    if (!view_) { QWidget::keyPressEvent(event); return; }
    const int key = event->key();
    auto* vbar = view_->verticalScrollBar();
    if (!vbar) { QWidget::keyPressEvent(event); return; }

    // Smooth scroll by nearly one viewport height
    const int pageStep = view_->viewport() ? view_->viewport()->height() - 32 : vbar->pageStep();
    const Qt::KeyboardModifiers mods = event->modifiers();
    // Ctrl + PageUp/PageDown for zoom
    if (mods.testFlag(Qt::ControlModifier) && (key == Qt::Key_PageUp || key == Qt::Key_PageDown)) {
        const double cur = view_->zoomFactor();
        const double next = (key == Qt::Key_PageUp) ? cur * 1.1 : cur / 1.1;
        setZoomFactor(next);
        event->accept();
        return;
    }
    switch (key) {
        case Qt::Key_PageDown:
            vbar->setValue(vbar->value() + pageStep);
            event->accept();
            return;
        case Qt::Key_PageUp:
            vbar->setValue(vbar->value() - pageStep);
            event->accept();
            return;
        default:
            break;
    }
    QWidget::keyPressEvent(event);
}

bool PdfViewerWidget::eventFilter(QObject* watched, QEvent* event) {
    if (watched == view_ || watched == view_->viewport()) {
        if (event->type() == QEvent::KeyPress) {
            auto* ke = static_cast<QKeyEvent*>(event);
            keyPressEvent(ke);
            if (ke->isAccepted()) return true;
        } else if (event->type() == QEvent::Wheel) {
            auto* we = static_cast<QWheelEvent*>(event);
            if (we->modifiers().testFlag(Qt::ControlModifier)) {
                // Zoom in/out by wheel notches
                const QPoint numDegrees = we->angleDelta();
                const int dy = numDegrees.y();
                if (dy != 0) {
                    const double cur = view_ ? view_->zoomFactor() : 1.0;
                    const double step = (dy > 0) ? wheelZoomStep_ : (1.0 / wheelZoomStep_);
                    setZoomAnimated(cur * step);
                    event->accept();
                    return true;
                }
            }
        } else if (event->type() == QEvent::MouseButtonPress) {
            auto* me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::RightButton) {
                // Do NOT alter current selection on right-click; just show context menu if any selection exists
                bool ok = false;
                const QString text = selectionText(&ok);
                const bool hasImageSelection = selRect_.isValid() && !selRect_.isEmpty();
                QMenu menu(this);
                if (ok && !text.trimmed().isEmpty()) {
                    const int wordCount = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
                    if (wordCount <= 3 && text.size() <= 80) {
                        QAction* actSyn = menu.addAction(tr("Obter sinônimos"));
                        QObject::connect(actSyn, &QAction::triggered, this, [this, text]() { emit requestSynonyms(text.trimmed()); });

                        QAction* actDict = menu.addAction(tr("Consultar dicionário"));
                        QObject::connect(actDict, &QAction::triggered, this, [this, text]() { emit requestDictionaryLookup(text.trimmed()); });
                    }
                    if (wordCount >= 10 || text.contains('\n')) {
                        QAction* actSum = menu.addAction(tr("Gerar resumo"));
                        QObject::connect(actSum, &QAction::triggered, this, [this, text]() { emit requestSummarize(text); });
                    }
                    QAction* actChat = menu.addAction(tr("Enviar ao chat (IA)"));
                    QObject::connect(actChat, &QAction::triggered, this, [this, text]() { emit requestSendToChat(text); });
                    // If there is an image selection rectangle too, offer image sending as well
                    if (hasImageSelection) {
                        menu.addSeparator();
                        QPoint tl = view_->viewport()->mapTo(view_, selRect_.topLeft());
                        QRect inView(tl, selRect_.size());
                        QPixmap pm = view_->grab(inView);
                        if (!pm.isNull()) {
                            QAction* actChatImg = menu.addAction(tr("Enviar imagem ao chat (IA)"));
                            QObject::connect(actChatImg, &QAction::triggered, this, [this, pm]() {
                                emit requestSendImageToChat(pm.toImage());
                            });
                        }
                    }
                    menu.addSeparator();
                    QAction* actCopy = menu.addAction(tr("Copiar (Ctrl+C)"));
                    QObject::connect(actCopy, &QAction::triggered, this, [this]() { copySelection(); });
                } else if (hasImageSelection) {
                    // Build image from current rectangle selection (viewport coords to view coords)
                    QPoint tl = view_->viewport()->mapTo(view_, selRect_.topLeft());
                    QRect inView(tl, selRect_.size());
                    QPixmap pm = view_->grab(inView);
                    if (!pm.isNull()) {
                        QAction* actChatImg = menu.addAction(tr("Enviar imagem ao chat (IA)"));
                        QObject::connect(actChatImg, &QAction::triggered, this, [this, pm]() {
                            emit requestSendImageToChat(pm.toImage());
                        });
                        menu.addSeparator();
                        QAction* actCopy = menu.addAction(tr("Copiar (Ctrl+C)"));
                        QObject::connect(actCopy, &QAction::triggered, this, [this]() { copySelection(); });
                    } else {
                        QAction* actCancel = menu.addAction(tr("Falha ao capturar imagem"));
                        actCancel->setEnabled(false);
                    }
                } else {
                    // Sem seleção válida
                    QAction* actCancel = menu.addAction(tr("Sem seleção válida"));
                    actCancel->setEnabled(false);
                }
                // Always offer to rebuild embeddings
                menu.addSeparator();
                QAction* actRebuild = menu.addAction(tr("Recriar embeddings do documento..."));
                QObject::connect(actRebuild, &QAction::triggered, this, [this]() { emit requestRebuildEmbeddings(); });
                menu.exec(view_->viewport()->mapToGlobal(me->pos()));
                event->accept();
                return true;
            }
            if (me->button() == Qt::LeftButton) {
                // Start a new selection only on left-click
                selecting_ = true;
                const QPoint start = (watched == view_->viewport()) ? me->pos() : view_->viewport()->mapFrom(view_, me->pos());
                selStart_ = start;
                selRect_ = QRect(selStart_, QSize());
                if (rubber_) {
                    rubber_->setGeometry(selRect_);
                    rubber_->show();
                }
                event->accept();
                return true;
            }
            
        } else if (event->type() == QEvent::MouseMove) {
            auto* me = static_cast<QMouseEvent*>(event);
            if (selecting_ && (me->buttons() & Qt::LeftButton) && selMode_ != SelectionMode::None) {
                const QPoint cur = (watched == view_->viewport()) ? me->pos() : view_->viewport()->mapFrom(view_, me->pos());
                selRect_ = QRect(selStart_, cur).normalized();
                if (rubber_) rubber_->setGeometry(selRect_);
                event->accept();
                return true;
            }
        } else if (event->type() == QEvent::MouseButtonRelease) {
            auto* me = static_cast<QMouseEvent*>(event);
            if (selecting_ && me->button() == Qt::LeftButton) {
                selecting_ = false;
                event->accept();
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void PdfViewerWidget::setWheelZoomStep(double step) {
    if (step < 1.01) step = 1.01;
    if (step > 1.5) step = 1.5;
    wheelZoomStep_ = step;
}

void PdfViewerWidget::setZoomAnimated(double target) {
    if (!view_) return;
    if (target < 0.25) target = 0.25;
    if (target > 4.0) target = 4.0;
    const double start = view_->zoomFactor();
    if (qFuzzyCompare(start, target)) {
        // ensure custom mode
        view_->setZoomMode(QPdfView::ZoomMode::Custom);
        view_->setZoomFactor(target);
        return;
    }
    if (!zoomAnim_) {
        zoomAnim_ = new QVariantAnimation(this);
        zoomAnim_->setEasingCurve(QEasingCurve::InOutCubic);
        connect(zoomAnim_, &QVariantAnimation::valueChanged, this, [this](const QVariant& v){
            if (!view_) return;
            view_->setZoomMode(QPdfView::ZoomMode::Custom);
            view_->setZoomFactor(v.toDouble());
            emit zoomFactorChanged(view_->zoomFactor());
        });
    }
    if (zoomAnim_->state() == QVariantAnimation::Running) zoomAnim_->stop();
    zoomAnim_->setStartValue(start);
    zoomAnim_->setEndValue(target);
    zoomAnim_->setDuration(120);
    zoomAnim_->start();
}

void PdfViewerWidget::setSelectionMode(SelectionMode mode) {
    selMode_ = mode;
    clearSelection();
}

bool PdfViewerWidget::hasSelection() const {
    if (selMode_ == SelectionMode::Rect) return selRect_.isValid() && !selRect_.isEmpty();
    if (selMode_ == SelectionMode::Text) return !selectedText_.isEmpty();
    return false;
}

void PdfViewerWidget::clearSelection() {
    selRect_ = QRect();
    selectedText_.clear();
    if (rubber_) rubber_->hide();
}

static QRect normalizedRect(const QPoint& a, const QPoint& b) {
    return QRect(a, b).normalized();
}

void PdfViewerWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        // First check for hyperlinks at click position
#ifdef HAS_QPDF_LINK_MODEL
        if (linkModel_ && doc_ && navigation_) {
            const QPoint viewportPos = view_->viewport()->mapFrom(this, event->pos());
            // Note: QPdfView doesn't have mapToScene, we need to calculate position differently
            // For now, we'll use a simpler approach with page coordinates
            
            // Set the current page for link detection
            const int currentPageIndex = navigation_->currentPage();
            linkModel_->setPage(currentPageIndex);
            
            // Convert viewport position to document coordinates (simplified)
            // This is a basic implementation - may need refinement for accuracy
            const QPointF documentPos = QPointF(viewportPos);
            
            // Check if there's a link at this position
            const QPdfLink link = linkModel_->linkAt(documentPos);
            if (link.isValid()) {
                // Handle the link navigation
                if (link.page() >= 0) {
                    // Internal link to another page
                    navigation_->jump(link.page(), link.location(), link.zoom());
                    event->accept();
                    return;
                }
                // Could also handle external URLs here if needed
                // For now, just handle internal page links
            }
        }
#endif
        
        // If no link was clicked, proceed with selection logic
        if (selMode_ != SelectionMode::None) {
            selecting_ = true;
            selStart_ = view_->viewport()->mapFrom(this, event->pos());
            selRect_ = QRect(selStart_, QSize());
            if (rubber_) {
                rubber_->setGeometry(selRect_);
                rubber_->show();
            }
            event->accept();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void PdfViewerWidget::mouseMoveEvent(QMouseEvent* event) {
    if (selecting_ && (event->buttons() & Qt::LeftButton) && selMode_ != SelectionMode::None) {
        const QPoint cur = view_->viewport()->mapFrom(this, event->pos());
        selRect_ = normalizedRect(selStart_, cur);
        if (rubber_) rubber_->setGeometry(selRect_);
        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void PdfViewerWidget::mouseReleaseEvent(QMouseEvent* event) {
if (selecting_ && event->button() == Qt::LeftButton) {
        selecting_ = false;
        // For text mode, attempt immediate extraction on current page if API is available
#ifdef HAS_QPDF_SELECTION
        if (selMode_ == SelectionMode::Text && doc_ && navigation_) {
            const int pageIdx = navigation_->currentPage();
            // Heuristic: map viewport rect to page area by rendering scale approximation is complex;
            // as etapa 1, keep it simple: we attempt to use full page width proportionally is not reliable,
            // so leave extraction for copy time instead.
        }
#endif
        event->accept();
        return;
    }
QWidget::mouseReleaseEvent(event);
}

void PdfViewerWidget::copySelection() {
    if (!view_) return;
    QClipboard* cb = QGuiApplication::clipboard();
    bool copied = false;
    if (selMode_ == SelectionMode::Rect) {
        if (!selRect_.isValid() || selRect_.isEmpty()) {
            // nothing to copy
        } else {
            // Map rubber (viewport) rect to view coordinates for grabbing
            QPoint tl = view_->viewport()->mapTo(view_, selRect_.topLeft());
            QRect inView(tl, selRect_.size());
            QPixmap pm = view_->grab(inView);
            if (!pm.isNull()) { cb->setImage(pm.toImage()); copied = true; }
        }
    }
    if (selMode_ == SelectionMode::Text) {
#ifdef HAS_QPDF_SELECTION
        // Best-effort text extraction on current page
        const int pageIdx = navigation_ ? navigation_->currentPage() : 0;
        // Fallback: copy previously extracted text if available
        if (!selectedText_.isEmpty()) { cb->setText(selectedText_); copied = true; }
        // If we cannot map to page coordinates, copy nothing and inform user via status would be nicer
        // but for etapa 1 we'll at least avoid crash.
        if (copied) { showCopyToast(); }
#else
        // Not supported on this Qt
        if (!selectedText_.isEmpty()) { cb->setText(selectedText_); copied = true; }
#endif
        // Etapa 2: tentar OCR
        bool ok = false;
        const QString ocr = ocrSelectionText(&ok);
        if (ok && !ocr.trimmed().isEmpty()) { cb->setText(ocr.trimmed()); copied = true; }
    }

    if (copied) { showCopyToast(); }
}

void PdfViewerWidget::showToast(const QString& message) {
    QWidget* host = view_ && view_->viewport() ? view_->viewport() : this;
    if (!toastLabel_) {
        toastLabel_ = new QLabel(host);
        toastLabel_->setWordWrap(true);
        toastLabel_->setAlignment(Qt::AlignCenter);
        toastLabel_->setText(message);
        toastLabel_->setStyleSheet("QLabel{background-color: rgba(40,40,40,180); color: white; border-radius: 8px; padding: 10px 14px; font-weight: 500;}");
        toastLabel_->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        toastOpacity_ = new QGraphicsOpacityEffect(toastLabel_);
        toastLabel_->setGraphicsEffect(toastOpacity_);
        toastOpacity_->setOpacity(0.0);
        toastHideTimer_ = new QTimer(this);
        toastHideTimer_->setSingleShot(true);
        connect(toastHideTimer_, &QTimer::timeout, this, [this]() {
            // fade out
            auto* anim = new QVariantAnimation(this);
            anim->setStartValue(1.0);
            anim->setEndValue(0.0);
            anim->setDuration(250);
            anim->setEasingCurve(QEasingCurve::OutCubic);
            connect(anim, &QVariantAnimation::valueChanged, this, [this](const QVariant& v){ if (toastOpacity_) toastOpacity_->setOpacity(v.toDouble()); });
            connect(anim, &QVariantAnimation::finished, this, [this, anim]() { if (toastLabel_) toastLabel_->hide(); anim->deleteLater(); });
            anim->start();
        });
    }
    toastLabel_->setText(message);
    // Size to content with a max width relative to host
    int maxW = host->width() * 0.6;
    toastLabel_->setMaximumWidth(maxW);
    toastLabel_->adjustSize();
    const int x = (host->width() - toastLabel_->width())/2;
    const int y = (host->height() - toastLabel_->height())/2;
    toastLabel_->move(x, y);
    toastLabel_->show();
    // fade in
    auto* animIn = new QVariantAnimation(this);
    animIn->setStartValue(0.0);
    animIn->setEndValue(1.0);
    animIn->setDuration(180);
    animIn->setEasingCurve(QEasingCurve::OutCubic);
    connect(animIn, &QVariantAnimation::valueChanged, this, [this](const QVariant& v){ if (toastOpacity_) toastOpacity_->setOpacity(v.toDouble()); });
    connect(animIn, &QVariantAnimation::finished, this, [this, animIn]() {
        animIn->deleteLater();
        if (toastHideTimer_) toastHideTimer_->start(900);
    });
    animIn->start();
}

void PdfViewerWidget::showCopyToast() {
    showToast(tr("Seleção copiada para a área de transferência."));
}

void PdfViewerWidget::flashHighlight() {
    QWidget* host = view_ && view_->viewport() ? view_->viewport() : this;
    if (!highlightOverlay_) {
        highlightOverlay_ = new QLabel(host);
        highlightOverlay_->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        highlightOverlay_->setStyleSheet("QLabel{background-color: rgba(255, 230, 0, 90); border: 2px solid rgba(255,200,0,180);}");
        highlightOverlay_->hide();
    }
    // Cover the viewport with small margins
    const int margin = 6;
    const QRect r(margin, margin, qMax(0, host->width() - 2*margin), qMax(0, host->height() - 2*margin));
    highlightOverlay_->setGeometry(r);
    highlightOverlay_->show();
    // Auto-hide shortly
    if (!highlightTimer_) {
        highlightTimer_ = new QTimer(this);
        highlightTimer_->setSingleShot(true);
        connect(highlightTimer_, &QTimer::timeout, this, [this]() {
            if (highlightOverlay_) highlightOverlay_->hide();
        });
    }
    highlightTimer_->start(600);
}

void PdfViewerWidget::saveSelectionAsTxt() {
    if (selMode_ == SelectionMode::Rect) {
        const QString path = QFileDialog::getSaveFileName(this, tr("Salvar seleção"), QString(), tr("Texto (*.txt)"));
        if (path.isEmpty()) return;
        // Save note and paired image
        const QString imgPath = QFileInfo(path).completeBaseName() + ".png";
        QPoint tl = view_->viewport()->mapTo(view_, selRect_.topLeft());
        QRect inView(tl, selRect_.size());
        QPixmap pm = view_->grab(inView);
        if (!pm.isNull()) pm.save(imgPath, "PNG");
        QFile f(path);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream ts(&f);
            ts << tr("Imagem salva em ") << imgPath << "\n";
        }
        return;
    }
    if (selMode_ == SelectionMode::Text) {
        const QString path = QFileDialog::getSaveFileName(this, tr("Salvar seleção"), QString(), tr("Texto (*.txt)"));
        if (path.isEmpty()) return;
        QFile f(path);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream ts(&f);
            QString out = selectedText_;
            if (out.trimmed().isEmpty()) {
                bool ok = false; out = ocrSelectionText(&ok);
            }
            ts << out;
        }
    }
}

void PdfViewerWidget::saveSelectionAsMarkdown() {
    if (selMode_ == SelectionMode::Rect) {
        const QString path = QFileDialog::getSaveFileName(this, tr("Salvar seleção"), QString(), tr("Markdown (*.md)"));
        if (path.isEmpty()) return;
        const QString base = QFileInfo(path).completeBaseName();
        const QString imgPath = base + ".png";
        QPoint tl = view_->viewport()->mapTo(view_, selRect_.topLeft());
        QRect inView(tl, selRect_.size());
        QPixmap pm = view_->grab(inView);
        if (!pm.isNull()) pm.save(imgPath, "PNG");
        QFile f(path);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream ts(&f);
            ts << "![](" << imgPath << ")\n";
        }
        return;
    }
    if (selMode_ == SelectionMode::Text) {
        const QString path = QFileDialog::getSaveFileName(this, tr("Salvar seleção"), QString(), tr("Markdown (*.md)"));
        if (path.isEmpty()) return;
        QFile f(path);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream ts(&f);
            QString out = selectedText_;
            if (out.trimmed().isEmpty()) {
                bool ok = false; out = ocrSelectionText(&ok);
            }
            ts << out << "\n";
        }
    }
}

QString PdfViewerWidget::ocrSelectionText(bool* ok) {
    if (ok) *ok = false;
    if (!view_ || !selRect_.isValid() || selRect_.isEmpty()) return {};

    // Grab selection as image
    QPoint tl = view_->viewport()->mapTo(view_, selRect_.topLeft());
    QRect inView(tl, selRect_.size());
    QPixmap pm = view_->grab(inView);
    if (pm.isNull()) return {};

    // Save to temporary PNG
    const QString tmpDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QDir().mkpath(tmpDir);
    const QString tmpBase = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const QString tmpPng = QDir(tmpDir).filePath(tmpBase + ".png");
    if (!pm.save(tmpPng, "PNG")) return {};

    // Run tesseract: tesseract <img> stdout -l por+eng --psm 6
    QProcess proc;
    QStringList args;
    args << tmpPng << "stdout" << "-l" << "por+eng" << "--psm" << "6";
    proc.start("tesseract", args);
    if (!proc.waitForStarted(3000)) return {};
    proc.waitForFinished(20000);
    const QByteArray out = proc.readAllStandardOutput();
    const QByteArray err = proc.readAllStandardError();
    Q_UNUSED(err);
    const QString text = QString::fromUtf8(out).trimmed();
    if (ok) *ok = !text.isEmpty();
    return text;
}

void PdfViewerWidget::startRectSelection(QMouseEvent* event, QObject* watched) {
    selecting_ = true;
    const QPoint start = (watched == view_->viewport()) ? event->pos() : view_->viewport()->mapFrom(view_, event->pos());
    selStart_ = start;
    selRect_ = QRect(selStart_, QSize());
    if (rubber_) {
        rubber_->setGeometry(selRect_);
        rubber_->show();
    }
}

QString PdfViewerWidget::extractTextFromSelectionNative() {
#ifdef HAS_QPDF_SELECTION
    // Best-effort: use current page selection API if available in this Qt build.
    if (!doc_ || !navigation_) return {};
    const int pageIdx = navigation_->currentPage();
    Q_UNUSED(pageIdx);
    // NOTE: Mapping viewport rect to page coordinates varies by Qt version.
    // To keep this build-safe across Qt variants, we do not attempt coordinate mapping here.
    // Hook point for future enhancement when a stable API is guaranteed.
#endif
    return {};
}


