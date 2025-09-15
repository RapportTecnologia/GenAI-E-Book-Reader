#pragma once

#include <QDockWidget>
#include <QVector>
#include <QString>

class QTextEdit;
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
    QString transcriptText() const; // plain text transcript
    QString transcriptHtml() const; // rich transcript
    void setTranscriptHtml(const QString& html);

    // Pending image preview shown above the input before sending
    void setPendingImage(const QImage& img);
    void clearPendingImage();
    bool hasPendingImage() const { return !pendingImage_.isNull(); }
    QImage pendingImage() const { return pendingImage_; }

signals:
    void sendMessageRequested(const QString& text);
    void saveTranscriptRequested(const QString& text);
    void summarizeTranscriptRequested(const QString& text);

private slots:
    void onSendClicked();
    void onSaveClicked();
    void onSummarizeClicked();

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override;

private:
    void appendLine(const QString& who, const QString& text);
    void appendImageLine(const QString& who, const QImage& img);

    QWidget* container_ {nullptr};
    QTextEdit* history_ {nullptr};
    QPlainTextEdit* input_ {nullptr};
    QPushButton* btnSend_ {nullptr};
    QToolButton* btnSave_ {nullptr};
    QToolButton* btnSummarize_ {nullptr};
    // Pending image preview UI
    QWidget* pendingContainer_ {nullptr};
    QLabel* pendingThumb_ {nullptr};
    QToolButton* pendingClear_ {nullptr};
    QImage pendingImage_ {};
};
