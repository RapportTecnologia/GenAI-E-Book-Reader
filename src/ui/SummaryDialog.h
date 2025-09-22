#pragma once

#include <QWidget>
#include <QDialog>
#include <QString>

class QPlainTextEdit;

class SummaryDialog : public QDialog {
    Q_OBJECT
public:
    explicit SummaryDialog(QWidget* parent = nullptr);

    void setText(const QString& text);
    QString text() const;

private:
    QPlainTextEdit* edit_ {nullptr};
};
