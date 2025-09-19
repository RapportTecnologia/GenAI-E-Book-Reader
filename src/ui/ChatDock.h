#pragma once

#include <QDockWidget>
#include <QVector>
#include <QString>

class QWebEngineView;
class QTextEdit; // forward declared only if used elsewhere by includes
class QPlainTextEdit;
class QPushButton;
class QToolButton;
class QVBoxLayout;
class QHBoxLayout;
class QImage;
class QLabel;
class QToolButton;

// Simple chat panel docked to the right
class ChatDock : public QDockWidget {
    Q_OBJECT
public:
    explicit ChatDock(QWidget* parent = nullptr);

    void appendUser(const QString& text);
    void appendAssistant(const QString& text);
    void appendUserImage(const QImage& img);
    void appendAssistantImage(const QImage& img);
    QString transcriptText() const; // plain text transcript (best-effort from markdown)
    QString transcriptHtml() const; // rich transcript (full HTML document)
    void setTranscriptHtml(const QString& html);

    // Agentic prompt preview panel
    void setAgenticPrompt(const QString& prompt);
    QString agenticPrompt() const;
    void showAgenticPrompt(bool show);

    // LLM conversation history (role, content) for continuous context
    QList<QPair<QString, QString>> conversationForLlm() const { return historyMsgs_; }
    void setConversationForLlm(const QList<QPair<QString, QString>>& msgs) { historyMsgs_ = msgs; }

    // Chat session management
    void clearConversation();
    QString suggestTitle() const; // auto title from latest assistant reply or first user prompt

    // Pending image preview shown above the input before sending
    void setPendingImage(const QImage& img);
    void clearPendingImage();
    bool hasPendingImage() const { return !pendingImage_.isNull(); }
    QImage pendingImage() const { return pendingImage_; }

signals:
    void sendMessageRequested(const QString& text);
    void saveTranscriptRequested(const QString& text);
    void summarizeTranscriptRequested(const QString& text);
    void requestShowSavedChats(); // ask MainWindow to open a saved chats picker
    void conversationCleared(const QString& maybeTitle, const QString& html, const QList<QPair<QString,QString>>& msgs);

private slots:
    void onSendClicked();
    void onSaveClicked();
    void onSummarizeClicked();

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override;

private:
    void appendLine(const QString& who, const QString& text);
    void appendImageLine(const QString& who, const QImage& img);
    void ensurePageLoaded();
    void rebuildDocument();

    QWidget* container_ {nullptr};
    QWebEngineView* historyView_ {nullptr};
    QPlainTextEdit* input_ {nullptr};
    QPushButton* btnSend_ {nullptr};
    QToolButton* btnSave_ {nullptr};
    QToolButton* btnSummarize_ {nullptr};
    QToolButton* btnNewChat_ {nullptr};
    QToolButton* btnHistory_ {nullptr};
    // Pending image preview UI
    QWidget* pendingContainer_ {nullptr};
    QLabel* pendingThumb_ {nullptr};
    QToolButton* pendingClear_ {nullptr};
    QImage pendingImage_ {};
    // Agentic prompt area
    QWidget* agenticContainer_ {nullptr};
    QPlainTextEdit* agenticView_ {nullptr};
    QToolButton* agenticToggle_ {nullptr};
    QToolButton* agenticCopy_ {nullptr};
    // Accumulated HTML body content (message blocks). Full document is built on the fly.
    QString htmlBody_;
    // Parallel storage for plain conversation turns: role = "user" | "assistant"; content is Markdown/plain
    QList<QPair<QString, QString>> historyMsgs_;

    // Debounce rebuilds to avoid overlapping MathJax loads and reinitializations
    QTimer* rebuildTimer_ {nullptr};
    bool pageLoading_ {false};
    bool rebuildPending_ {false};
};
