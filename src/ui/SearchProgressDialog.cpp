#include "ui/SearchProgressDialog.h"

#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QDialogButtonBox>
#include <QAbstractButton>

SearchProgressDialog::SearchProgressDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Buscando..."));
    setModal(true);
    resize(720, 360);

    auto* lay = new QVBoxLayout(this);

    log_ = new QPlainTextEdit(this);
    log_->setReadOnly(true);
    lay->addWidget(log_, 1);

    btns_ = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(btns_, &QDialogButtonBox::rejected, this, &QDialog::reject);
    lay->addWidget(btns_);
}

void SearchProgressDialog::appendLine(const QString& line) {
    if (!log_) return;
    log_->appendPlainText(line);
}

void SearchProgressDialog::setBusy(bool busy) {
    if (!btns_) return;
    for (auto* b : btns_->buttons()) b->setEnabled(!busy);
}
