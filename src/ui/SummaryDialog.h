#pragma once

#include <QWidget>
#include <QDialog>
#include <QString>

class QPlainTextEdit;
class QPushButton;

class SummaryDialog : public QDialog {
    Q_OBJECT
public:
    explicit SummaryDialog(QWidget* parent = nullptr);

    void setText(const QString& text);
    QString text() const;

signals:
    void sendToChatRequested(const QString& text);

private:
    QPlainTextEdit* edit_ {nullptr};
    QPushButton* btnSend_ {nullptr};
};
