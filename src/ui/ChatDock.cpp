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
    v->addWidget(historyView_, 1);
    // Scroll to bottom after each page load (connect once)
    connect(historyView_, &QWebEngineView::loadFinished, this, [this](bool){
        if (historyView_ && historyView_->page()) {
            historyView_->page()->runJavaScript("window.scrollTo(0, document.body.scrollHeight);");
        }
    });
    rebuildDocument();

    // Bottom bar with actions
    auto* bottom = new QHBoxLayout();
    btnSave_ = new QToolButton(container_);
    btnSave_->setText(tr("Salvar"));
    btnSummarize_ = new QToolButton(container_);
    btnSummarize_->setText(tr("Compilar"));
    btnNewChat_ = new QToolButton(container_);
    btnNewChat_->setText(tr("Novo"));
    btnHistory_ = new QToolButton(container_);
    btnHistory_->setText(tr("Histórico"));
    bottom->addWidget(btnSave_);
    bottom->addWidget(btnSummarize_);
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
    v->addWidget(input_);

    // Send button
    btnSend_ = new QPushButton(tr("Enviar"), container_);
    v->addWidget(btnSend_, 0, Qt::AlignRight);

    setWidget(container_);

    connect(btnSend_, &QPushButton::clicked, this, &ChatDock::onSendClicked);
    connect(btnSave_, &QToolButton::clicked, this, &ChatDock::onSaveClicked);
    connect(btnSummarize_, &QToolButton::clicked, this, &ChatDock::onSummarizeClicked);
    connect(pendingClear_, &QToolButton::clicked, this, &ChatDock::clearPendingImage);
    connect(btnNewChat_, &QToolButton::clicked, this, [this]{ clearConversation(); });
    connect(btnHistory_, &QToolButton::clicked, this, [this]{ emit requestShowSavedChats(); });
}

void ChatDock::rebuildDocument() {
    const QString doc = transcriptHtml();
    if (historyView_) {
        historyView_->setHtml(doc);
    }
}

void ChatDock::appendLine(const QString& who, const QString& text) {
    // Append as markdown block (rendered in WebEngine)
    const QString block = QString(
        "<div class=\"msg\"><div class=\"who\"><b>%1:</b></div><div class=\"md\">%2</div></div>"
    ).arg(who.toHtmlEscaped(), text.toHtmlEscaped());
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
        "<script src=\"https://cdn.jsdelivr.net/npm/marked/marked.min.js\"></script>"
        "<link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/highlight.js@11.9.0/styles/github.min.css\">"
        "<script src=\"https://cdn.jsdelivr.net/npm/highlight.js@11.9.0/lib/highlight.min.js\"></script>"
        "<script>"
        "// Configure marked to support GFM (including tables) and line breaks\n"
        "marked.setOptions({ gfm: true, tables: true, breaks: true, headerIds: false, mangle: false, smartLists: true });\n"
        "function renderAll(){\n"
        "  document.querySelectorAll('.md').forEach(n=>{ n.innerHTML = marked.parse(n.textContent || ''); });\n"
        "  document.querySelectorAll('pre code').forEach((el)=>{ try{ hljs.highlightElement(el); }catch(e){} });\n"
        "}\n"
        "function scrollToBottom(){ window.scrollTo(0, document.body.scrollHeight); }\n"
        "window.addEventListener('load', async ()=>{\n"
        "  renderAll();\n"
        "  if(window.MathJax && MathJax.typesetPromise){ try { await MathJax.typesetPromise(); } catch(e){} }\n"
        "  scrollToBottom();\n"
        "});"
        "</script>"
        "<script>\n"
        "window.MathJax = {\n"
        "  tex: {\n"
        "    inlineMath: [['$','$'], ['\\\\(','\\\\)']],\n"
        "    displayMath: [['$$','$$'], ['\\\\[','\\\\]']],\n"
        "    packages: { '[+]': ['noerrors','noundefined'] }\n"
        "  },\n"
        "  options: {\n"
        "    skipHtmlTags: ['script','noscript','style','textarea','pre','code'],\n"
        "    ignoreHtmlClass: 'tex2jax_ignore',\n"
        "    processHtmlClass: 'tex2jax_process'\n"
        "  }\n"
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
    QString t = source;
    // Remove fenced code blocks
    t.remove(QRegularExpression("```[\n\s\S]*?```"));
    // Remove inline code
    t.remove(QRegularExpression("`[^`]*`"));
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
    const QString text = input_->toPlainText().trimmed();
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

bool ChatDock::eventFilter(QObject* obj, QEvent* ev) {
    if (obj == input_ && ev->type() == QEvent::KeyPress) {
        auto* ke = static_cast<QKeyEvent*>(ev);
        if ((ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) && (ke->modifiers() & Qt::ControlModifier)) {
            onSendClicked();
            return true;
        }
    }
    return QDockWidget::eventFilter(obj, ev);
}
