#include "ui/ChatDock.h"

#include <QWebEngineView>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QEvent>
#include <QKeyEvent>
#include <QImage>
#include <QBuffer>
#include <QByteArray>
#include <QMessageBox>
#include <QDateTime>
#include <QLabel>
#include <QPixmap>
#include <QRegularExpression>
#include <QClipboard>
#include <QGuiApplication>
#include <QTimer>
#include <QMenu>
#include <QContextMenuEvent>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include <QPageLayout>
#include <QProgressBar>
#include <cmark.h>
// Custom WebEngine helpers
#include "ui/WebPage.h"
#include "ui/WebProfile.h"

ChatDock::ChatDock(QWidget* parent)
    : QDockWidget(parent) {
    setWindowTitle(tr("Chat"));
    setObjectName("dock_chat");
    container_ = new QWidget(this);
    auto* v = new QVBoxLayout(container_);
    v->setContentsMargins(6,6,6,6);
    v->setSpacing(6);

    // History view via WebEngine: supports Markdown + MathJax + Highlight.js
    historyView_ = new QWebEngineView(container_);
    // Use shared ephemeral profile and a page that logs JS console messages
    if (historyView_) {
        auto* page = new WebPage(sharedEphemeralWebProfile(), historyView_);
        historyView_->setPage(page);
        // Allow right-click context menu on history view to offer printing
    if (historyView_) historyView_->installEventFilter(this);
    // Track container events to keep overlay in sync
    container_->installEventFilter(this);
}
    v->addWidget(historyView_, 1);
    // Scroll to bottom after each page load (connect once)
    connect(historyView_, &QWebEngineView::loadStarted, this, [this](){ pageLoading_ = true; });
    connect(historyView_, &QWebEngineView::loadFinished, this, [this](bool){
        pageLoading_ = false;
        if (historyView_ && historyView_->page()) {
            historyView_->page()->runJavaScript("window.scrollTo(0, document.body.scrollHeight);");
            // After rendering new HTML, ask MathJax to re-typeset the page
            historyView_->page()->runJavaScript("if (window.MathJax) { window.MathJax.typesetPromise(); }");
        }
        // If a rebuild was requested during load, trigger it now (debounced)
        if (rebuildPending_ && rebuildTimer_) {
            rebuildPending_ = false;
            rebuildTimer_->start();
        }
    });
    // Debounce timer to throttle rebuilds and avoid overlapping MathJax init
    rebuildTimer_ = new QTimer(this);
    rebuildTimer_->setSingleShot(true);
    rebuildTimer_->setInterval(80); // ~1 frame delay
    connect(rebuildTimer_, &QTimer::timeout, this, [this](){
        if (!historyView_) return;
        const QString doc = transcriptHtml();
        pageLoading_ = true;
        historyView_->setHtml(doc);
    });
    rebuildDocument();

    // Agentic prompt preview (collapsed by default), below history
    agenticContainer_ = new QWidget(container_);
    auto* agenticLayout = new QVBoxLayout(agenticContainer_);
    agenticLayout->setContentsMargins(0,0,0,0);
    agenticLayout->setSpacing(4);
    auto* agenticTop = new QHBoxLayout();
    agenticTop->setContentsMargins(0,0,0,0);
    agenticToggle_ = new QToolButton(agenticContainer_);
    agenticToggle_->setText(tr("Prompt Agentic"));
    agenticCopy_ = new QToolButton(agenticContainer_);
    agenticCopy_->setText(tr("Copiar"));
    agenticTop->addWidget(agenticToggle_);
    agenticTop->addStretch();
    agenticTop->addWidget(agenticCopy_);
    agenticLayout->addLayout(agenticTop);
    agenticView_ = new QPlainTextEdit(agenticContainer_);
    agenticView_->setReadOnly(true);
    agenticView_->setPlaceholderText(tr("Prompt agentic (RAG) será exibido aqui quando aplicável..."));
    agenticView_->setMinimumHeight(120);
    agenticLayout->addWidget(agenticView_);
    v->addWidget(agenticContainer_);
    agenticContainer_->setVisible(false);

    // Bottom bar with actions
    auto* bottom = new QHBoxLayout();
    btnSave_ = new QToolButton(container_);
    btnSave_->setText(tr("Salvar"));
    btnSummarize_ = new QToolButton(container_);
    btnSummarize_->setText(tr("Resumir"));
    btnPrint_ = new QToolButton(container_);
    btnPrint_->setText(tr("Imprimir"));
    btnNewChat_ = new QToolButton(container_);
    btnNewChat_->setText(tr("Novo"));
    btnHistory_ = new QToolButton(container_);
    btnHistory_->setText(tr("Histórico de Chats"));
    bottom->addWidget(btnSave_);
    bottom->addWidget(btnSummarize_);
    bottom->addWidget(btnPrint_);
    bottom->addWidget(btnNewChat_);
    bottom->addWidget(btnHistory_);
    bottom->addStretch();
    v->addLayout(bottom);

    // Pending image preview (hidden by default)
    pendingContainer_ = new QWidget(container_);
    auto* ph = new QHBoxLayout(pendingContainer_);
    ph->setContentsMargins(0,0,0,0);
    ph->setSpacing(6);
    pendingThumb_ = new QLabel(pendingContainer_);
    pendingThumb_->setMinimumHeight(64);
    pendingThumb_->setMaximumHeight(96);
    pendingThumb_->setScaledContents(true);
    pendingClear_ = new QToolButton(pendingContainer_);
    pendingClear_->setText(tr("Remover"));
    ph->addWidget(pendingThumb_, 1);
    ph->addWidget(pendingClear_, 0);
    pendingContainer_->setVisible(false);
    v->addWidget(pendingContainer_);

    // Input area
    input_ = new QPlainTextEdit(container_);
    input_->setPlaceholderText(tr("Digite sua mensagem... (Ctrl+Enter para enviar, Enter insere nova linha)"));
    input_->installEventFilter(this);
    input_->setContextMenuPolicy(Qt::DefaultContextMenu);
    v->addWidget(input_);

    // Send button
    btnSend_ = new QPushButton(tr("Enviar"), container_);
    v->addWidget(btnSend_, 0, Qt::AlignRight);

    setWidget(container_);

    connect(btnSend_, &QPushButton::clicked, this, &ChatDock::onSendClicked);
    connect(btnSave_, &QToolButton::clicked, this, &ChatDock::onSaveClicked);
    connect(btnSummarize_, &QToolButton::clicked, this, &ChatDock::onSummarizeClicked);
    connect(btnPrint_, &QToolButton::clicked, this, &ChatDock::onPrintClicked);
    connect(pendingClear_, &QToolButton::clicked, this, &ChatDock::clearPendingImage);
    connect(btnNewChat_, &QToolButton::clicked, this, [this]{ clearConversation(); });
    connect(btnHistory_, &QToolButton::clicked, this, [this]{ emit requestShowSavedChats(); });
    connect(agenticToggle_, &QToolButton::clicked, this, [this]{ showAgenticPrompt(!agenticContainer_->isVisible()); });
    connect(agenticCopy_, &QToolButton::clicked, this, [this]{
        if (!agenticView_) return; auto* cb = QGuiApplication::clipboard(); if (cb) cb->setText(agenticView_->toPlainText());
    });

    // Create busy overlay lazily on first use in setBusy()
}

void ChatDock::rebuildDocument() {
    // If a load is in progress, mark a pending rebuild
    if (pageLoading_) { rebuildPending_ = true; return; }
    // Debounce multiple quick calls
    if (rebuildTimer_) {
        rebuildTimer_->start();
        return;
    }
    // Fallback (shouldn't hit if timer exists)
    const QString doc = transcriptHtml();
    if (historyView_) { pageLoading_ = true; historyView_->setHtml(doc); }
}

void ChatDock::appendLine(const QString& who, const QString& text) {
    QString processedText = text;
    qCritical() << "text" << text;
    // Protect MathJax content from cmark parser by wrapping it in spans
    // Process display math first ($$ ... $$) to avoid conflict with inline
    processedText.replace(QStringLiteral("\\$\\$\\"), QStringLiteral("\\\\$\\\\$"));
    processedText.replace(QStringLiteral("\\$\\$\\"), QStringLiteral("\\\\$\\\\$"));
    // Use simple string replacement to avoid regex issues
    processedText.replace(QStringLiteral("\\("), QStringLiteral("\\\\("));
    processedText.replace(QStringLiteral("\\)"), QStringLiteral("\\\\)"));
    processedText.replace(QStringLiteral("\\["), QStringLiteral("\\\\["));
    processedText.replace(QStringLiteral("\\]"), QStringLiteral("\\\\]"));
    // Process inline math second ($ ... $)
    //processedText.replace(QRegularExpression("\\$([^\\$]+?)\\/$"), "<span class=\"math-inline\">\\1</span>");
    qCritical() << "processedText" << processedText;
    
    // Convert markdown to HTML using cmark
    QByteArray textUtf8 = processedText.toUtf8();
    char* html = cmark_markdown_to_html(textUtf8.constData(), textUtf8.size(), CMARK_OPT_SMART);
    qCritical() << "html" << html;
    QString renderedHtml = QString::fromUtf8(html);
    free(html);

    // Append as a block with the rendered HTML
    const QString block = QString(
        "<div class=\"msg\"><div class=\"who\"><b>%1:</b></div><div class=\"md\">%2</div></div>"
    ).arg(who.toHtmlEscaped(), renderedHtml);
    htmlBody_ += block;
    rebuildDocument();
}

void ChatDock::appendUser(const QString& text) {
    appendLine(tr("Você"), text);
    historyMsgs_.append({QStringLiteral("user"), text});
}
void ChatDock::appendAssistant(const QString& text) {
    appendLine(tr("IA"), text);
    historyMsgs_.append({QStringLiteral("assistant"), text});
}

static QString toDataUrlPng(const QImage& img) {
    QByteArray bytes;
    QBuffer buf(&bytes);
    buf.open(QIODevice::WriteOnly);
    img.save(&buf, "PNG");
    const QByteArray b64 = bytes.toBase64();
    return QString::fromLatin1("data:image/png;base64,%1").arg(QString::fromLatin1(b64));
}

void ChatDock::appendImageLine(const QString& who, const QImage& img) {
    const QString url = toDataUrlPng(img);
    const QString html = QString(
        "<div class=\"msg\"><div class=\"who\"><b>%1:</b></div><div class=\"img\"><img src=\"%2\" style=\"max-width:100%%; border-radius:6px;\"></div></div>"
    ).arg(who.toHtmlEscaped(), url);
    htmlBody_ += html;
    rebuildDocument();
}

void ChatDock::appendUserImage(const QImage& img) { appendImageLine(tr("Você"), img); }
void ChatDock::appendAssistantImage(const QImage& img) { appendImageLine(tr("IA"), img); }

QString ChatDock::transcriptText() const {
    // Naive strip of tags from our htmlBody_
    QString t = htmlBody_;
    t.remove(QRegularExpression("<[^>]*>"));
    return t;
}

QString ChatDock::transcriptHtml() const {
    // Return full HTML document (head + body + libs)
    const QString doc = QString(
        "<!DOCTYPE html><html><head>"
        "<meta charset=\"utf-8\">"
        "<script>"
        "function scrollToBottom(){ window.scrollTo(0, document.body.scrollHeight); }\n"
        "window.addEventListener('load', ()=>{\n"
        "  scrollToBottom();\n"
        "});"
        "</script>"
        "<script>\n"
        "window.MathJax = {\n"
        "  tex: {\n"
        "    inlineMath: [['$','$'], ['\\\\(','\\\\)']],\n"
        "    displayMath: [['$$','$$'], ['\\\\[','\\\\]']],\n"
        "    processEscapes: true\n"
        "  },\n"
        "  options: {\n"
        "    skipHtmlTags: ['script','noscript','style','textarea','pre','code']\n"
        "  },\n"
        "};\n"
        "</script>"
        "<script src=\"https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-mml-chtml.js\" id=\"MathJax-script\" async></script>"
        "<style>"
        "body{font-family:sans-serif;padding:8px;}"
        ".msg{margin:8px 0;} .who{color:#666;margin-bottom:4px;}"
        "/* Table styling for Markdown */"
        "table{border-collapse:collapse;width:100%;margin:8px 0;display:block;overflow-x:auto;}"
        "th,td{border:1px solid #ccc;padding:6px;vertical-align:top;}"
        "thead th{background:#f5f5f5;}"
        "code{background:#f6f8fa;border:1px solid #e1e4e8;border-radius:4px;padding:2px 4px;}"
        "pre{background:#f6f8fa;border:1px solid #e1e4e8;border-radius:4px;padding:10px;overflow:auto;}"
        "pre code{display:block;padding:0;background:transparent;border:none;}"
        "</style>"
        "</head><body>%1</body></html>"
    ).arg(htmlBody_);
    return doc;
}

void ChatDock::setTranscriptHtml(const QString& html) {
    // Load provided HTML and also store its body best-effort
    htmlBody_ = html;
    historyView_->setHtml(html);
}

void ChatDock::setAgenticPrompt(const QString& prompt) {
    if (agenticView_) agenticView_->setPlainText(prompt);
}

QString ChatDock::agenticPrompt() const {
    return agenticView_ ? agenticView_->toPlainText() : QString();
}

void ChatDock::showAgenticPrompt(bool show) {
    if (agenticContainer_) agenticContainer_->setVisible(show);
}

void ChatDock::clearConversation() {
    const auto ret = QMessageBox::question(this, tr("Novo chat"),
                                           tr("Deseja iniciar um novo chat?\n\nSalvar o chat atual no histórico?"),
                                           QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                           QMessageBox::Yes);
    if (ret == QMessageBox::Cancel) return;
    if (ret == QMessageBox::Yes) {
        const QString title = suggestTitle();
        emit conversationCleared(title, transcriptHtml(), historyMsgs_);
    }
    htmlBody_.clear();
    historyMsgs_.clear();
    rebuildDocument();
    // Notify listeners that a brand-new chat session has started so they can
    // avoid reloading previous history automatically.
    emit newChatStarted();
}

QString ChatDock::suggestTitle() const {
    QString source;
    for (int i = historyMsgs_.size() - 1; i >= 0; --i) {
        if (historyMsgs_[i].first == QLatin1String("assistant")) { source = historyMsgs_[i].second; break; }
    }
    if (source.isEmpty()) {
        for (const auto& p : historyMsgs_) { if (p.first == QLatin1String("user")) { source = p.second; break; } }
    }
    if (source.isEmpty()) source = tr("Novo chat %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm"));
    // Create a temporary copy for manipulation
    QString t = source;
    // Remove fenced code blocks
    t.remove(QRegularExpression("```[\\s\\S]*?```"));
    // Remove inline code
    t.remove(QRegularExpression("`[^`]*`"));
    // Remove MathJax delimiters so they don't linger in the title
    t.remove(QRegularExpression("\\$[^\\$]*\\$"));
    // Remove HTML tags
    t.remove(QRegularExpression("<[^>]*>"));
    t.replace('\n', ' ');
    t = t.simplified();
    if (t.size() > 60) t = t.left(60) + QLatin1String("…");
    return t;
}

void ChatDock::setPendingImage(const QImage& img) {
    pendingImage_ = img;
    if (!img.isNull()) {
        // Create a scaled pixmap for thumbnail
        const int h = 96;
        QPixmap pm = QPixmap::fromImage(img).scaledToHeight(h, Qt::SmoothTransformation);
        pendingThumb_->setPixmap(pm);
        pendingContainer_->setVisible(true);
    } else {
        clearPendingImage();
    }
}

void ChatDock::clearPendingImage() {
    pendingImage_ = QImage();
    pendingThumb_->clear();
    pendingContainer_->setVisible(false);
}

void ChatDock::onSendClicked() {
    // Preserve exact formatting (do not trim) to avoid breaking Markdown/LaTeX blocks
    const QString text = input_->toPlainText();
    if (text.isEmpty()) return;
    emit sendMessageRequested(text);
    input_->clear();
    input_->setFocus();
}

void ChatDock::onSaveClicked() {
    emit saveTranscriptRequested(transcriptText());
}

void ChatDock::onSummarizeClicked() {
    emit summarizeTranscriptRequested(transcriptText());
}

void ChatDock::onPrintClicked() {
#ifdef HAVE_QT_WEBENGINE
    if (!historyView_) return;
    QPrinter printer(QPrinter::HighResolution);
    printer.setDocName(tr("Chat - GenAI Reader"));
    QPrintDialog dlg(&printer, this);
    dlg.setWindowTitle(tr("Imprimir chat"));
    if (dlg.exec() != QDialog::Accepted) return;

    // Print the current chat view snapshot fitted to a page
    QPixmap pm = historyView_->grab();
    if (pm.isNull()) { QMessageBox::warning(this, tr("Imprimir"), tr("Falha ao capturar conteúdo do chat.")); return; }
    QPainter painter(&printer);
    if (!painter.isActive()) { QMessageBox::warning(this, tr("Imprimir"), tr("Falha ao iniciar impressora.")); return; }
    const QRect pageRect = printer.pageLayout().paintRectPixels(printer.resolution());
    QPixmap scaled = pm.scaled(pageRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    const int x = pageRect.x() + (pageRect.width() - scaled.width())/2;
    const int y = pageRect.y() + (pageRect.height() - scaled.height())/2;
    painter.drawPixmap(QPoint(x, y), scaled);
    painter.end();
#endif
}

bool ChatDock::eventFilter(QObject* obj, QEvent* ev) {
    const bool isHist = (obj == historyView_);
    const bool isCont = (obj == container_);
    if ((isHist || isCont) && overlay_) {
        if (ev->type() == QEvent::Resize || ev->type() == QEvent::Show || ev->type() == QEvent::Move) {
            // Position overlay to cover the historyView_ area within container_
            if (historyView_ && overlay_->parentWidget() == container_) {
                overlay_->setGeometry(historyView_->geometry());
            }
            if (overlayInner_) {
                const int w = overlayInner_->width();
                const int h = overlayInner_->height();
                overlayInner_->move(overlay_->rect().center() - QPoint(w/2, h/2));
            }
            overlay_->raise();
        }
    }
    if (obj == input_ && ev->type() == QEvent::KeyPress) {
        auto* ke = static_cast<QKeyEvent*>(ev);
        if ((ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) && (ke->modifiers() & Qt::ControlModifier)) {
            onSendClicked();
            return true;
        }
    }
    // Context menu on history or input: add Print option
    if ((obj == historyView_ || obj == input_) && ev->type() == QEvent::ContextMenu) {
        auto* ce = static_cast<QContextMenuEvent*>(ev);
        QMenu menu(this);
        QAction* actPrint = menu.addAction(tr("Imprimir chat..."));
        connect(actPrint, &QAction::triggered, this, &ChatDock::onPrintClicked);
        menu.exec(ce->globalPos());
        return true;
    }
    return QDockWidget::eventFilter(obj, ev);
}

void ChatDock::setBusy(bool on) {
    busy_ = on;
    if (input_) input_->setDisabled(on);
    if (btnSend_) btnSend_->setDisabled(on);
    if (!historyView_) return;
    // Lazy create overlay
    if (!overlay_) {
        // Parent to container_ so it can be stacked above QWebEngineView reliably
        overlay_ = new QWidget(container_);
        overlay_->setAttribute(Qt::WA_NoSystemBackground, false);
        overlay_->setAutoFillBackground(true);
        QPalette pal = overlay_->palette();
        pal.setColor(QPalette::Window, QColor(0,0,0,90));
        overlay_->setPalette(pal);
        if (historyView_) overlay_->setGeometry(historyView_->geometry());

        overlayInner_ = new QWidget(overlay_);
        overlayInner_->setObjectName("overlayInner");
        overlayInner_->setStyleSheet("#overlayInner { background: #ffffff; border: 1px solid rgba(0,0,0,0.15); box-shadow: 0px 6px 18px rgba(0,0,0,0.15); border-radius: 8px; }");
        auto* innerLayout = new QVBoxLayout(overlayInner_);
        innerLayout->setContentsMargins(16,16,16,16);
        innerLayout->setSpacing(8);
        overlayLabel_ = new QLabel(tr("Pensando"), overlayInner_);
        overlayLabel_->setStyleSheet("color:#333; font-size:14px; font-weight:600;");
        overlayLabel_->setAlignment(Qt::AlignCenter);
        overlayBar_ = new QProgressBar(overlayInner_);
        overlayBar_->setRange(0,0); // indeterminate
        overlayBar_->setTextVisible(true);
        overlayBar_->setFormat(tr("Pensando"));
        overlayBar_->setStyleSheet("QProgressBar { text-align:center; color:#333; } QProgressBar::chunk { background-color:#3b82f6; }");
        innerLayout->addWidget(overlayLabel_);
        innerLayout->addWidget(overlayBar_);

        // Center the inner box
        const int w = 260, h = 96;
        overlayInner_->setFixedSize(w, h);
        overlayInner_->move(overlay_->rect().center() - QPoint(w/2, h/2));

        overlay_->hide();
    }
    overlay_->setVisible(on);
    overlay_->raise();
    if (overlayInner_) {
        const int w = overlayInner_->width();
        const int h = overlayInner_->height();
        overlayInner_->move(overlay_->rect().center() - QPoint(w/2, h/2));
    }
}
