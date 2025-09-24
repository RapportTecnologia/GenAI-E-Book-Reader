#pragma once

#include <QDialog>
#include <QWidget>
#include <QString>

class QPlainTextEdit;
class QDialogButtonBox;

// Simple modal dialog to display search progress and logs of AI interactions
class SearchProgressDialog : public QDialog {
    Q_OBJECT
public:
    explicit SearchProgressDialog(QWidget* parent = nullptr);

    void appendLine(const QString& line);
    void setBusy(bool busy);

private:
    QPlainTextEdit* log_ {nullptr};
    QDialogButtonBox* btns_ {nullptr};
};
