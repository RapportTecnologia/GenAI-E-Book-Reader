#include "ui/OpfGenAiDialog.h"

#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QDialogButtonBox>
#include <QAbstractButton>
#include <QMovie>
#include <QLabel>

OpfGenAiDialog::OpfGenAiDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Completar metadados (IA)"));
    resize(720, 480);

    auto* lay = new QVBoxLayout(this);

    log_ = new QPlainTextEdit(this);
    log_->setReadOnly(true);
    lay->addWidget(log_, 1);

    btns_ = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(btns_, &QDialogButtonBox::rejected, this, &QDialog::reject);
    lay->addWidget(btns_);
}

void OpfGenAiDialog::appendLine(const QString& line) {
    if (!log_) return;
    log_->appendPlainText(line);
}

void OpfGenAiDialog::setBusy(bool busy) {
    if (!btns_) return;
    for (auto* b : btns_->buttons()) b->setEnabled(!busy);
}
