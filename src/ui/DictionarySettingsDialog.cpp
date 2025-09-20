#include "ui/DictionarySettingsDialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QSettings>
#include <QLabel>
#include <QCheckBox>

DictionarySettingsDialog::DictionarySettingsDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Configurações de Dicionário"));
    resize(560, 320);

    auto* root = new QVBoxLayout(this);
    auto* form = new QFormLayout();

    useLlmCheckBox_ = new QCheckBox(tr("Usar LLM para pesquisa de palavras"), this);
    form->addRow(useLlmCheckBox_);

    llmPromptEdit_ = new QLineEdit(this);
    form->addRow(tr("Prompt da LLM para Dicionário"), llmPromptEdit_);

    serviceCombo_ = new QComboBox(this);
    serviceCombo_->addItem("LibreTranslate", "libre");
    serviceCombo_->addItem("Open Multilingual Wordnet", "omw");
    form->addRow(tr("Serviço de Dicionário"), serviceCombo_);
    root->addLayout(form);

    tabs_ = new QTabWidget(this);

    // LibreTranslate Tab
    auto* libreWidget = new QWidget(this);
    auto* libreForm = new QFormLayout(libreWidget);
    libreApiUrlEdit_ = new QLineEdit(this);
    libreApiKeyEdit_ = new QLineEdit(this);
    libreApiKeyEdit_->setEchoMode(QLineEdit::Password);
    libreSourceLangEdit_ = new QLineEdit(this);
    libreTargetLangEdit_ = new QLineEdit(this);
    libreForm->addRow(tr("URL da API"), libreApiUrlEdit_);
    libreForm->addRow(tr("Chave da API (opcional)"), libreApiKeyEdit_);
    libreForm->addRow(tr("Idioma de Origem"), libreSourceLangEdit_);
    libreForm->addRow(tr("Idioma de Destino"), libreTargetLangEdit_);
    tabs_->addTab(libreWidget, "LibreTranslate");

    // OMW Tab
    auto* omwWidget = new QWidget(this);
    auto* omwForm = new QFormLayout(omwWidget);
    omwApiUrlEdit_ = new QLineEdit(this);
    omwForm->addRow(tr("URL da API (se aplicável)"), omwApiUrlEdit_);
    tabs_->addTab(omwWidget, "OMW");

    root->addWidget(tabs_);

    buttons_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    root->addWidget(buttons_);

    connect(buttons_, &QDialogButtonBox::accepted, this, &DictionarySettingsDialog::accept);
    connect(buttons_, &QDialogButtonBox::rejected, this, &DictionarySettingsDialog::reject);
    connect(serviceCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), tabs_, &QTabWidget::setCurrentIndex);
    connect(tabs_, &QTabWidget::currentChanged, this, [this](int index){
        serviceCombo_->setCurrentIndex(index);
    });

    connect(useLlmCheckBox_, &QCheckBox::toggled, this, [this](bool checked) {
        serviceCombo_->setEnabled(!checked);
        tabs_->setEnabled(!checked);
        llmPromptEdit_->setVisible(checked);
    });

    loadFromSettings();
}

void DictionarySettingsDialog::loadFromSettings() {
    QSettings s;
    useLlmCheckBox_->setChecked(s.value("dictionary/use_llm", true).toBool());
    llmPromptEdit_->setText(s.value("dictionary/llm_prompt", tr("Forneça o significado, a etimologia e os sinônimos da palavra: {palavra}")).toString());
    const QString service = s.value("dictionary/service", "libre").toString();
    int idx = serviceCombo_->findData(service);
    if (idx < 0) idx = 0;
    serviceCombo_->setCurrentIndex(idx);
    tabs_->setCurrentIndex(idx);

    // Trigger the enable/disable logic based on the loaded setting
    serviceCombo_->setEnabled(!useLlmCheckBox_->isChecked());
    tabs_->setEnabled(!useLlmCheckBox_->isChecked());

    // LibreTranslate
    libreApiUrlEdit_->setText(s.value("dictionary/libre/api_url", "https://libretranslate.de/translate").toString());
    libreApiKeyEdit_->setText(s.value("dictionary/libre/api_key").toString());
    libreSourceLangEdit_->setText(s.value("dictionary/libre/source_lang", "en").toString());
    libreTargetLangEdit_->setText(s.value("dictionary/libre/target_lang", "pt").toString());

    // OMW
    omwApiUrlEdit_->setText(s.value("dictionary/omw/api_url").toString());
}

void DictionarySettingsDialog::saveToSettings() {
    QSettings s;
    s.setValue("dictionary/use_llm", useLlmCheckBox_->isChecked());
    s.setValue("dictionary/llm_prompt", llmPromptEdit_->text());
    s.setValue("dictionary/service", serviceCombo_->currentData().toString());

    // LibreTranslate
    s.setValue("dictionary/libre/api_url", libreApiUrlEdit_->text().trimmed());
    s.setValue("dictionary/libre/api_key", libreApiKeyEdit_->text());
    s.setValue("dictionary/libre/source_lang", libreSourceLangEdit_->text().trimmed());
    s.setValue("dictionary/libre/target_lang", libreTargetLangEdit_->text().trimmed());

    // OMW
    s.setValue("dictionary/omw/api_url", omwApiUrlEdit_->text().trimmed());
}

void DictionarySettingsDialog::accept() {
    saveToSettings();
    QDialog::accept();
}