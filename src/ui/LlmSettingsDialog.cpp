#include "ui/LlmSettingsDialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QTabWidget>
#include <QPlainTextEdit>
#include <QDialogButtonBox>
#include <QSettings>

namespace {
// Defaults aimed at: Resumos, Sinônimos, Explicações e Chat sem delírios
const char* kDefaultPromptSynonyms =
    "Atue como um dicionário de sinônimos confiável em pt-BR.\n"
    "Dado um termo ou locução, retorne de 5 a 10 sinônimos e, quando útil, pequenas notas de uso.\n"
    "Evite invenções. Seja conciso.";

const char* kDefaultPromptSummaries =
    "Resuma com precisão e objetividade em pt-BR.\n"
    "Foque ideias principais, termos-chave e estrutura.\n"
    "Evite alucinações. Seja fiel ao texto.\n"
    "Se apropriado, organize em tópicos curtos.";

const char* kDefaultPromptExplanations =
    "Explique o conteúdo em pt-BR de forma clara e didática, para um público geral.\n"
    "Defina termos importantes, traga exemplos curtos e analogias simples.\n"
    "Evite especulações: se a informação não estiver no trecho, não invente.";

const char* kDefaultPromptChat =
    "Você é um assistente que responde em pt-BR com exatidão e concisão.\n"
    "Jamais invente fatos.\n"
    "Se não tiver certeza, peça mais contexto ou informe a limitação.";
}

LlmSettingsDialog::LlmSettingsDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Configurações de LLM"));
    resize(640, 520);

    auto* root = new QVBoxLayout(this);

    // Provider/model/API form
    auto* form = new QFormLayout();
    providerCombo_ = new QComboBox(this);
    modelCombo_ = new QComboBox(this);
    baseUrlEdit_ = new QLineEdit(this);
    apiKeyEdit_ = new QLineEdit(this);
    apiKeyEdit_->setEchoMode(QLineEdit::Password);

    populateProviders();

    form->addRow(tr("Provedor"), providerCombo_);
    form->addRow(tr("Modelo"), modelCombo_);
    form->addRow(tr("Base URL (opcional)"), baseUrlEdit_);
    form->addRow(tr("API Key"), apiKeyEdit_);
    root->addLayout(form);

    // Prompts tabs
    tabs_ = new QTabWidget(this);
    promptSynonyms_ = new QPlainTextEdit(this);
    promptSummaries_ = new QPlainTextEdit(this);
    promptExplanations_ = new QPlainTextEdit(this);
    promptChat_ = new QPlainTextEdit(this);

    tabs_->addTab(promptSynonyms_, tr("Sinônimos"));
    tabs_->addTab(promptSummaries_, tr("Resumos"));
    tabs_->addTab(promptExplanations_, tr("Explicações"));
    tabs_->addTab(promptChat_, tr("Chat"));

    // Dictionary settings tab
    auto* dictWidget = new QWidget(this);
    auto* dictForm = new QFormLayout(dictWidget);
    dictApiUrlEdit_ = new QLineEdit(this);
    dictSourceLangEdit_ = new QLineEdit(this);
    dictTargetLangEdit_ = new QLineEdit(this);
    dictForm->addRow(tr("URL da API"), dictApiUrlEdit_);
    dictForm->addRow(tr("Idioma de origem (ex: en)"), dictSourceLangEdit_);
    dictForm->addRow(tr("Idioma de destino (ex: pt)"), dictTargetLangEdit_);
    tabs_->addTab(dictWidget, tr("Dicionário"));

    root->addWidget(tabs_, 1);

    // Buttons
    buttons_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    root->addWidget(buttons_);

    connect(buttons_, &QDialogButtonBox::accepted, this, &LlmSettingsDialog::accept);
    connect(buttons_, &QDialogButtonBox::rejected, this, &LlmSettingsDialog::reject);
    connect(providerCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LlmSettingsDialog::onProviderChanged);

    loadFromSettings();
}

void LlmSettingsDialog::populateProviders() {
    providerCombo_->clear();
    providerCombo_->addItem("OpenAI", "openai");
    providerCombo_->addItem("GenerAtiva", "generativa");
}

void LlmSettingsDialog::populateModelsFor(const QString& provider) {
    modelCombo_->clear();
    if (provider == "generativa") {
        // Sugestões compatíveis; ajuste conforme sua oferta
        modelCombo_->addItem("gpt-4o-mini (recomendado)", "gpt-4o-mini");
        modelCombo_->addItem("gpt-4o", "gpt-4o");
    } else {
        modelCombo_->addItem("gpt-4o-mini (recomendado)", "gpt-4o-mini");
        modelCombo_->addItem("gpt-4o", "gpt-4o");
        modelCombo_->addItem("gpt-3.5-turbo", "gpt-3.5-turbo");
    }
}

void LlmSettingsDialog::onProviderChanged(int) {
    const QString provider = providerCombo_->currentData().toString();
    populateModelsFor(provider);
    // Pick recommended default (first item)
    if (modelCombo_->count() > 0) modelCombo_->setCurrentIndex(0);
}

void LlmSettingsDialog::loadFromSettings() {
    QSettings s;
    const QString provider = s.value("ai/provider", "openai").toString();
    const QString model = s.value("ai/model", provider == "generativa" ? "gpt-4o-mini" : "gpt-4o-mini").toString();
    const QString baseUrl = s.value("ai/base_url").toString();
    const QString apiKey = s.value("ai/api_key").toString();

    // Provider + model
    int idx = providerCombo_->findData(provider);
    if (idx < 0) idx = 0;
    providerCombo_->setCurrentIndex(idx);
    populateModelsFor(providerCombo_->currentData().toString());
    int midx = modelCombo_->findData(model);
    if (midx < 0) midx = 0;
    modelCombo_->setCurrentIndex(midx);

    baseUrlEdit_->setText(baseUrl);
    apiKeyEdit_->setText(apiKey);

    // Prompts
    const QString pSyn = s.value("ai/prompts/synonyms", QString::fromUtf8(kDefaultPromptSynonyms)).toString();
    const QString pSum = s.value("ai/prompts/summaries", QString::fromUtf8(kDefaultPromptSummaries)).toString();
    const QString pExp = s.value("ai/prompts/explanations", QString::fromUtf8(kDefaultPromptExplanations)).toString();
    const QString pChat = s.value("ai/prompts/chat", QString::fromUtf8(kDefaultPromptChat)).toString();
    promptSynonyms_->setPlainText(pSyn);
    promptSummaries_->setPlainText(pSum);
    promptExplanations_->setPlainText(pExp);
    promptChat_->setPlainText(pChat);

    // Dictionary
    const QString dictApiUrl = s.value("dictionary/api_url", "https://libretranslate.de/translate").toString();
    const QString dictSourceLang = s.value("dictionary/source_lang", "en").toString();
    const QString dictTargetLang = s.value("dictionary/target_lang", "pt").toString();
    dictApiUrlEdit_->setText(dictApiUrl);
    dictSourceLangEdit_->setText(dictSourceLang);
    dictTargetLangEdit_->setText(dictTargetLang);
}

void LlmSettingsDialog::saveToSettings() {
    QSettings s;
    s.setValue("ai/provider", providerCombo_->currentData().toString());
    s.setValue("ai/model", modelCombo_->currentData().toString());
    s.setValue("ai/base_url", baseUrlEdit_->text().trimmed());
    s.setValue("ai/api_key", apiKeyEdit_->text());

    s.setValue("ai/prompts/synonyms", promptSynonyms_->toPlainText());
    s.setValue("ai/prompts/summaries", promptSummaries_->toPlainText());
    s.setValue("ai/prompts/explanations", promptExplanations_->toPlainText());
    s.setValue("ai/prompts/chat", promptChat_->toPlainText());

    // Dictionary
    s.setValue("dictionary/api_url", dictApiUrlEdit_->text().trimmed());
    s.setValue("dictionary/source_lang", dictSourceLangEdit_->text().trimmed());
    s.setValue("dictionary/target_lang", dictTargetLangEdit_->text().trimmed());
}

void LlmSettingsDialog::accept() {
    saveToSettings();
    QDialog::accept();
}
