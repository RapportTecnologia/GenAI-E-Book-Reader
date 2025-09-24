#pragma once

#include <QDialog>

class QPlainTextEdit;
class QDialogButtonBox;

class OpfGenAiDialog : public QDialog {
    Q_OBJECT
public:
    explicit OpfGenAiDialog(QWidget* parent = nullptr);

    void appendLine(const QString& line);
    void setBusy(bool busy);

private:
    QPlainTextEdit* log_ {nullptr};
    QDialogButtonBox* btns_ {nullptr};
};
