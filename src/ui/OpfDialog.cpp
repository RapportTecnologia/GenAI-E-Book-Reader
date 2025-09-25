#include "ui/OpfDialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>

OpfDialog::OpfDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Metadados do Livro (OPF)"));
    resize(700, 600);

    auto* lay = new QVBoxLayout(this);
    auto* form = new QFormLayout();

    titleEdit_ = new QLineEdit(this);
    authorEdit_ = new QLineEdit(this);
    publisherEdit_ = new QLineEdit(this);
    languageEdit_ = new QLineEdit(this);
    idEdit_ = new QLineEdit(this);
    keywordsEdit_ = new QLineEdit(this);

    descriptionEdit_ = new QPlainTextEdit(this);
    descriptionEdit_->setPlaceholderText(tr("Descrição longa (dc:description)"));
    summaryEdit_ = new QPlainTextEdit(this);
    summaryEdit_->setPlaceholderText(tr("Resumo executivo (meta name=summary)"));

    form->addRow(tr("Título:"), titleEdit_);
    form->addRow(tr("Autor:"), authorEdit_);
    form->addRow(tr("Editora:"), publisherEdit_);
    form->addRow(tr("Idioma:"), languageEdit_);
    form->addRow(tr("Identificador:"), idEdit_);
    form->addRow(tr("Palavras-chave:"), keywordsEdit_);

    lay->addLayout(form);
    lay->addWidget(descriptionEdit_, 1);
    lay->addWidget(summaryEdit_, 1);

    // Status label (busy/info messages)
    statusLabel_ = new QLabel(this);
    statusLabel_->setObjectName("opfStatusLabel");
    statusLabel_->setStyleSheet("color: #666;");
    statusLabel_->setText(QString());
    lay->addWidget(statusLabel_);

    // Indeterminate progress bar as a spinner-like indicator
    progress_ = new QProgressBar(this);
    progress_->setRange(0, 0); // indeterminate
    progress_->setTextVisible(false);
    progress_->setVisible(false);
    lay->addWidget(progress_);

    auto* btns = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Close, this);
    editBtn_ = new QPushButton(tr("Editar"), this);
    aiBtn_ = new QPushButton(tr("Completar com IA"), this);
    regenBtn_ = new QPushButton(tr("Recriar OPF"), this);
    btns->addButton(editBtn_, QDialogButtonBox::ActionRole);
    btns->addButton(aiBtn_, QDialogButtonBox::ActionRole);
    btns->addButton(regenBtn_, QDialogButtonBox::ActionRole);
    lay->addWidget(btns);

    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(editBtn_, &QPushButton::clicked, this, &OpfDialog::toggleEdit);
    connect(aiBtn_, &QPushButton::clicked, this, [this]{ emit requestCompleteWithAi(); });
    connect(regenBtn_, &QPushButton::clicked, this, [this]{ emit requestRegenerateOpf(); });

    setEditable(false);
}

void OpfDialog::setEditable(bool on) {
    const bool ro = !on;
    titleEdit_->setReadOnly(ro);
    authorEdit_->setReadOnly(ro);
    publisherEdit_->setReadOnly(ro);
    languageEdit_->setReadOnly(ro);
    idEdit_->setReadOnly(ro);
    keywordsEdit_->setReadOnly(ro);
    descriptionEdit_->setReadOnly(ro);
    summaryEdit_->setReadOnly(ro);
    editBtn_->setText(on ? tr("Bloquear edição") : tr("Editar"));
}

void OpfDialog::toggleEdit() {
    const bool nowEditable = titleEdit_->isReadOnly();
    setEditable(nowEditable);
}

void OpfDialog::setData(const OpfData& d) {
    titleEdit_->setText(d.title);
    authorEdit_->setText(d.author);
    publisherEdit_->setText(d.publisher);
    languageEdit_->setText(d.language);
    idEdit_->setText(d.identifier);
    descriptionEdit_->setPlainText(d.description);
    summaryEdit_->setPlainText(d.summary);
    keywordsEdit_->setText(d.keywords);
}

OpfData OpfDialog::data() const {
    OpfData d;
    d.title = titleEdit_->text();
    d.author = authorEdit_->text();
    d.publisher = publisherEdit_->text();
    d.language = languageEdit_->text();
    d.identifier = idEdit_->text();
    d.description = descriptionEdit_->toPlainText();
    d.summary = summaryEdit_->toPlainText();
    d.keywords = keywordsEdit_->text();
    return d;
}

void OpfDialog::setBusy(bool busy) {
    busy_ = busy;
    // Disable interactive buttons while busy and force read-only fields
    if (editBtn_) editBtn_->setEnabled(!busy);
    if (aiBtn_) aiBtn_->setEnabled(!busy);
    if (regenBtn_) regenBtn_->setEnabled(!busy);
    setEditable(!busy);
}

void OpfDialog::setStatusMessage(const QString& msg) {
    if (!statusLabel_) return;
    statusLabel_->setText(msg);
}
