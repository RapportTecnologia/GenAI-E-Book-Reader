#include "ui/SummaryDialog.h"

#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QDialogButtonBox>
#include <QPushButton>

SummaryDialog::SummaryDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Resumo / Resposta da IA"));
    resize(640, 420);

    auto* lay = new QVBoxLayout(this);
    edit_ = new QPlainTextEdit(this);
    edit_->setReadOnly(false); // allow selecting and copying with Ctrl+C
    lay->addWidget(edit_, 1);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Close, this);
    btnSend_ = new QPushButton(tr("Enviar ao chat"), this);
    box->addButton(btnSend_, QDialogButtonBox::ActionRole);
    lay->addWidget(box);

    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(btnSend_, &QPushButton::clicked, this, [this]() {
        emit sendToChatRequested(edit_->toPlainText());
    });
}

void SummaryDialog::setText(const QString& text) {
    if (edit_) edit_->setPlainText(text);
}

QString SummaryDialog::text() const {
    return edit_ ? edit_->toPlainText() : QString();
}
