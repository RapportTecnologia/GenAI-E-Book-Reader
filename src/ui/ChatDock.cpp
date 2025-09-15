#include "ui/ChatDock.h"

#include <QTextEdit>
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
#include <QLabel>

ChatDock::ChatDock(QWidget* parent)
    : QDockWidget(parent) {
    setWindowTitle(tr("Chat"));
    setObjectName("dock_chat");
    container_ = new QWidget(this);
    auto* v = new QVBoxLayout(container_);
    v->setContentsMargins(6,6,6,6);
    v->setSpacing(6);

    // History view (rich text for readability)
    history_ = new QTextEdit(container_);
    history_->setReadOnly(true);
    v->addWidget(history_, 1);

    // Bottom bar with actions
    auto* bottom = new QHBoxLayout();
    btnSave_ = new QToolButton(container_);
    btnSave_->setText(tr("Salvar"));
    btnSummarize_ = new QToolButton(container_);
    btnSummarize_->setText(tr("Compilar"));
    bottom->addWidget(btnSave_);
    bottom->addWidget(btnSummarize_);
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
}

void ChatDock::appendLine(const QString& who, const QString& text) {
    history_->append(QString("<b>%1:</b> %2").arg(who, text.toHtmlEscaped()));
}

void ChatDock::appendUser(const QString& text) { appendLine(tr("Você"), text); }
void ChatDock::appendAssistant(const QString& text) { appendLine(tr("IA"), text); }

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
    const QString html = QString("<b>%1:</b><br><img src=\"%2\" style=\"max-width:100%%; border-radius:6px;\"><br>")
                             .arg(who.toHtmlEscaped(), url);
    history_->append(html);
}

void ChatDock::appendUserImage(const QImage& img) { appendImageLine(tr("Você"), img); }
void ChatDock::appendAssistantImage(const QImage& img) { appendImageLine(tr("IA"), img); }

QString ChatDock::transcriptText() const {
    // Simple plain text export of the conversation
    return history_->toPlainText();
}

QString ChatDock::transcriptHtml() const {
    return history_->toHtml();
}

void ChatDock::setTranscriptHtml(const QString& html) {
    history_->setHtml(html);
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
