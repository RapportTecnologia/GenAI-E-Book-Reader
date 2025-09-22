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
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QUrl>
#include <memory>
#include <QMessageBox>
#include <QPushButton>
#include <QDebug>
#include <QTime>
#include <QTextCursor>
#include <QSignalBlocker>
#include "ai/LlmClient.h"
#if __has_include("Config.h")
#  include "Config.h"
#else
#  define GENAI_OPENROUTER_API_KEY ""
#endif
#ifdef HAVE_QT_WEBENGINE
#include <QWebEngineView>
#endif

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

void LlmSettingsDialog::showOpenRouterCourtesyDialog() {
    // Build a simple dialog explaining the courtesy key and showing the video
    QDialog dlg(this);
    dlg.setWindowTitle(tr("OpenRouter: chave de cortesia"));
    dlg.resize(640, 420);
    auto* layout = new QVBoxLayout(&dlg);

    auto* msg = new QLabel(
        tr("Para sua conveniência, ao selecionar o provedor OpenRouter.ai, configuramos uma chave de API de cortesia como padrão.\n\n"
           "Essa chave é destinada a testes leves e uso casual. Para uso mais intenso e profissional, substitua pela sua própria chave em ‘API Key’."),
        &dlg);
    msg->setWordWrap(true);
    layout->addWidget(msg);

#ifdef HAVE_QT_WEBENGINE
    // Embed YouTube video when WebEngine is available
    auto* web = new QWebEngineView(&dlg);
    web->setUrl(QUrl("https://www.youtube.com/embed/dHggyhodAH4"));
    web->setMinimumHeight(300);
    layout->addWidget(web, 1);
#else
    // Fallback: clickable link to the video
    auto* link = new QLabel(&dlg);
    link->setText("<a href=\"https://www.youtube.com/watch?v=dHggyhodAH4\">Assista ao vídeo no YouTube</a>");
    link->setTextFormat(Qt::RichText);
    link->setTextInteractionFlags(Qt::TextBrowserInteraction);
    link->setOpenExternalLinks(true);
    layout->addWidget(link);
#endif

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok, &dlg);
    layout->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    dlg.exec();
}

void LlmSettingsDialog::onTestModel() {
    appendDebug(tr("[TEST] Iniciando teste de modelo"));
    // Persist current selections so LlmClient will read them
    const QString provider = providerCombo_->currentData().toString();
    const QString model = modelCombo_->currentData().toString();
    const QString baseUrl = baseUrlEdit_->text().trimmed();
    const QString apiKey = apiKeyEdit_->text();
    appendDebug(tr("[TEST] provider=%1 model=%2 baseUrl=%3 keySet=%4")
                    .arg(provider, model, baseUrl, apiKey.isEmpty() ? "false" : "true"));

    // Save to settings using our helpers
    QSettings s;
    s.setValue("ai/provider", provider);
    s.setValue("ai/model", model);
    s.setValue("ai/base_url", baseUrl);
    saveApiKeyForProvider(provider, apiKey);

    // Run a very short test prompt
    testOutput_->setPlainText(tr("Testando modelo '%1' do provedor '%2'...\nAguarde...")
                                  .arg(model, provider));

    // Create a temporary client and test
    auto* client = new LlmClient(this);
    const QString prompt = tr("Responda em apenas UMA frase curta: Este é um teste do modelo.");
    client->chat(prompt, [this, client](QString content, QString error) {
        appendDebug(tr("[TEST] Retorno do teste recebido. erro_vazio=%1").arg(error.isEmpty() ? "true" : "false"));
        client->deleteLater();
        if (!error.isEmpty()) {
            testOutput_->setPlainText(tr("Falha no teste do modelo:\n%1").arg(error));
            return;
        }
        // Show only first line trimmed to keep it compact
        QString line = content.trimmed();
        int nl = line.indexOf('\n');
        if (nl >= 0) line = line.left(nl);
        if (line.size() > 500) line = line.left(500) + "...";
        testOutput_->setPlainText(line);
    });
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
    modelCombo_->setEditable(true); // allow typing to filter
    baseUrlEdit_ = new QLineEdit(this);
    apiKeyEdit_ = new QLineEdit(this);
    apiKeyEdit_->setEchoMode(QLineEdit::Password);

    populateProviders();
    // Reordered for a continuous flow: Provider -> Base URL -> API Key -> Model
    form->addRow(tr("Provedor"), providerCombo_);
    form->addRow(tr("Base URL (opcional)"), baseUrlEdit_);
    form->addRow(tr("API Key"), apiKeyEdit_);
    form->addRow(tr("Modelo"), modelCombo_);
    // Initially disable model selection until we have a list (or for static lists)
    modelCombo_->setEnabled(false);
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
    root->addWidget(tabs_, 1);

    // Buttons
    buttons_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    // Test model UI (button + output)
    testButton_ = new QPushButton(tr("Testar modelo"), this);
    testOutput_ = new QPlainTextEdit(this);
    testOutput_->setReadOnly(true);
    testOutput_->setPlaceholderText(tr("Resultado do teste do modelo aparecerá aqui..."));
    testOutput_->setMinimumHeight(80);

    auto* testLayout = new QVBoxLayout();
    testLayout->addWidget(testButton_);
    testLayout->addWidget(testOutput_);
    root->addLayout(testLayout);

    root->addWidget(buttons_);

    connect(buttons_, &QDialogButtonBox::accepted, this, &LlmSettingsDialog::accept);
    connect(buttons_, &QDialogButtonBox::rejected, this, &LlmSettingsDialog::reject);
    connect(providerCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LlmSettingsDialog::onProviderChanged);
    connect(apiKeyEdit_, &QLineEdit::textChanged, this, &LlmSettingsDialog::onApiKeyEdited);
    if (modelCombo_->lineEdit()) {
        connect(modelCombo_->lineEdit(), &QLineEdit::textChanged, this, &LlmSettingsDialog::onModelFilterEdited);
    } else {
        appendDebug(tr("[INIT] modelCombo_->lineEdit() ausente; filtro de modelos não será conectado"));
    }
    connect(testButton_, &QPushButton::clicked, this, &LlmSettingsDialog::onTestModel);

    // Network manager for OpenRouter
    nam_ = new QNetworkAccessManager(this);

    loadFromSettings();
    appendDebug(tr("[INIT] Dialogo carregado. provider=%1 model=%2 baseUrl=%3")
                    .arg(providerCombo_->currentData().toString(),
                         modelCombo_->currentData().toString(),
                         baseUrlEdit_->text().trimmed()));
}

void LlmSettingsDialog::populateProviders() {
    providerCombo_->clear();
    providerCombo_->addItem("GenerAtiva", "generativa");
    providerCombo_->addItem("OpenWebUI", "openwebui");
    providerCombo_->addItem("Ollama (local)", "ollama");
    providerCombo_->addItem("OpenRouter.ai", "openrouter");
    providerCombo_->addItem("OpenAI", "openai");
    appendDebug(tr("[POPULATE] Provedores carregados"));
}

void LlmSettingsDialog::populateModelsFor(const QString& provider) {
    QSignalBlocker bCombo(*modelCombo_);
    const QLineEdit* le = modelCombo_->lineEdit();
    std::unique_ptr<QSignalBlocker> bLineEdit;
    if (le) bLineEdit.reset(new QSignalBlocker(*modelCombo_->lineEdit()));
    updatingModelList_ = true;
    modelCombo_->clear();
    // Providers that require API key to list models
    auto needsKey = [&](const QString& p){ return p == QLatin1String("generativa") || p == QLatin1String("openrouter") || p == QLatin1String("openwebui"); };
    const QString key = apiKeyEdit_->text().trimmed();
    appendDebug(tr("[MODELS] Preparando lista de modelos para provider=%1 keySet=%2")
                    .arg(provider, key.isEmpty() ? "false" : "true"));
    if (provider == "generativa") {
        if (key.isEmpty()) {
            modelCombo_->addItem(tr("Informe a API Key para listar modelos"), "");
            modelCombo_->setEnabled(false);
            modelCombo_->setEditText(QString());
            appendDebug(tr("[MODELS] GenerAtiva requer API Key"));
            return;
        }
        modelCombo_->addItem(tr("Carregando modelos da GenerAtiva..."), "");
        modelCombo_->setEditText(QString());
        fetchGenerativaModels();
    } else if (provider == "openrouter") {
        if (key.isEmpty()) {
            modelCombo_->addItem(tr("Informe a API Key para listar modelos"), "");
            modelCombo_->setEnabled(false);
            modelCombo_->setEditText(QString());
            appendDebug(tr("[MODELS] OpenRouter requer API Key"));
            return;
        }
        // Placeholder while fetching
        modelCombo_->addItem(tr("Carregando modelos do OpenRouter..."), "openrouter/auto");
        modelCombo_->setEditText(QString());
        appendDebug(tr("[MODELS] Iniciando fetchOpenRouterModels()"));
        fetchOpenRouterModels();
    } else if (provider == "openwebui") {
        if (key.isEmpty()) {
            modelCombo_->addItem(tr("Informe a API Key para listar modelos"), "");
            modelCombo_->setEnabled(false);
            modelCombo_->setEditText(QString());
            appendDebug(tr("[MODELS] OpenWebUI requer API Key"));
            return;
        }
        modelCombo_->addItem(tr("Carregando modelos do OpenWebUI..."), "");
        modelCombo_->setEditText(QString());
        fetchOpenWebUiModels();
    } else if (provider == "ollama") {
        modelCombo_->addItem(tr("Verificando Ollama..."), "");
        modelCombo_->setEditText(QString());
        verifyOrAssistOllama();
        modelCombo_->setEnabled(true);
        appendDebug(tr("[MODELS] Verificando Ollama"));
    } else {
        modelCombo_->addItem("gpt-4o-mini (recomendado)", "gpt-4o-mini");
        modelCombo_->addItem("gpt-4o", "gpt-4o");
        modelCombo_->addItem("gpt-3.5-turbo", "gpt-3.5-turbo");
        modelCombo_->setEditText(QString());
        modelCombo_->setEnabled(true);
        appendDebug(tr("[MODELS] Lista estatica (OpenAI) populada"));
    }
    updatingModelList_ = false;
}

void LlmSettingsDialog::onProviderChanged(int) {
    const QString provider = providerCombo_->currentData().toString();
    appendDebug(tr("[EVENT] onProviderChanged provider=%1").arg(provider));
    // Load per-provider API key first, then decide on listing
    migrateLegacyKeyIfNeeded(provider);
    apiKeyEdit_->setText(loadApiKeyForProvider(provider));
    // If OpenRouter selected: ensure courtesy behavior
    if (provider == QLatin1String("openrouter")) {
        if (apiKeyEdit_->text().trimmed().isEmpty()) {
            const QString courtesyKey = QString::fromUtf8(GENAI_OPENROUTER_API_KEY);
            if (!courtesyKey.trimmed().isEmpty()) {
                apiKeyEdit_->setText(courtesyKey);
                appendDebug(tr("[OPENROUTER] Usando API key definida em build (Config.h)"));
                // Persist immediately so that LlmClient and future sessions pick it up
                saveApiKeyForProvider(provider, courtesyKey);
            } else {
                appendDebug(tr("[OPENROUTER] Nenhuma API key definida em build; aguardando usuário informar"));
            }
        }
        showOpenRouterCourtesyDialog();
    }
    populateModelsFor(provider);
    // Pick recommended default (first item) if available
    if (modelCombo_->count() > 0) modelCombo_->setCurrentIndex(0);
}

void LlmSettingsDialog::loadFromSettings() {
    QSettings s;
    const QString provider = s.value("ai/provider", "openai").toString();
    const QString model = s.value("ai/model", provider == "openrouter" ? "openrouter/auto" : (provider == "generativa" ? "gpt-4o-mini" : "gpt-4o-mini")).toString();
    const QString baseUrl = s.value("ai/base_url").toString();
    migrateLegacyKeyIfNeeded(provider);
    const QString apiKey = loadApiKeyForProvider(provider);
    appendDebug(tr("[SETTINGS] provider=%1 model=%2 baseUrl=%3 keySet=%4")
                    .arg(provider, model, baseUrl, apiKey.isEmpty() ? "false" : "true"));

    // Provider + model
    int idx = providerCombo_->findData(provider);
    if (idx < 0) idx = 0;
    providerCombo_->setCurrentIndex(idx);
    baseUrlEdit_->setText(baseUrl);
    apiKeyEdit_->setText(apiKey);
    populateModelsFor(providerCombo_->currentData().toString());
    int midx = modelCombo_->findData(model);
    if (midx < 0) midx = 0;
    if (modelCombo_->count() > 0)
        modelCombo_->setCurrentIndex(midx);

    // Prompts
    const QString pSyn = s.value("ai/prompts/synonyms", QString::fromUtf8(kDefaultPromptSynonyms)).toString();
    const QString pSum = s.value("ai/prompts/summaries", QString::fromUtf8(kDefaultPromptSummaries)).toString();
    const QString pExp = s.value("ai/prompts/explanations", QString::fromUtf8(kDefaultPromptExplanations)).toString();
    const QString pChat = s.value("ai/prompts/chat", QString::fromUtf8(kDefaultPromptChat)).toString();
    promptSynonyms_->setPlainText(pSyn);
    promptSummaries_->setPlainText(pSum);
    promptExplanations_->setPlainText(pExp);
    promptChat_->setPlainText(pChat);
}

void LlmSettingsDialog::saveToSettings() {
    QSettings s;
    s.setValue("ai/provider", providerCombo_->currentData().toString());
    s.setValue("ai/model", modelCombo_->currentData().toString());
    s.setValue("ai/base_url", baseUrlEdit_->text().trimmed());
    // Save API key bound to the selected provider
    saveApiKeyForProvider(providerCombo_->currentData().toString(), apiKeyEdit_->text());

    s.setValue("ai/prompts/synonyms", promptSynonyms_->toPlainText());
    s.setValue("ai/prompts/summaries", promptSummaries_->toPlainText());
    s.setValue("ai/prompts/explanations", promptExplanations_->toPlainText());
    s.setValue("ai/prompts/chat", promptChat_->toPlainText());
}

void LlmSettingsDialog::accept() {
    saveToSettings();
    QDialog::accept();
}

void LlmSettingsDialog::onApiKeyEdited(const QString&) {
    const QString provider = providerCombo_->currentData().toString();
    const QString key = apiKeyEdit_->text().trimmed();
    appendDebug(tr("[EVENT] onApiKeyEdited provider=%1 keyLen=%2")
                    .arg(provider)
                    .arg(QString::number(key.size())));
    // For providers that require key to list, fetch only when key is provided
    if (provider == QLatin1String("openrouter") || provider == QLatin1String("openwebui") || provider == QLatin1String("generativa")) {
        if (key.isEmpty()) {
            QSignalBlocker bCombo(*modelCombo_);
            std::unique_ptr<QSignalBlocker> bLineEdit;
            if (modelCombo_->lineEdit()) bLineEdit.reset(new QSignalBlocker(*modelCombo_->lineEdit()));
            updatingModelList_ = true;
            modelCombo_->clear();
            modelCombo_->addItem(tr("Informe a API Key para listar modelos"), "");
            modelCombo_->setEnabled(false);
            updatingModelList_ = false;
        } else {
            populateModelsFor(provider);
        }
    }
}

void LlmSettingsDialog::onModelFilterEdited(const QString& text) {
    applyModelFilter(text);
}

void LlmSettingsDialog::applyModelFilter(const QString& filterText) {
    const QString provider = providerCombo_->currentData().toString();
    if (provider != QLatin1String("openrouter")) return; // only dynamic list for openrouter here
    if (updatingModelList_) {
        appendDebug(tr("[FILTER] Ignorado (atualização em andamento)"));
        return;
    }
    const QString t = filterText.trimmed();
    appendDebug(tr("[FILTER] Texto='%1' modelosCarregados=%2")
                    .arg(t)
                    .arg(QString::number(openRouterModels_.size())));
    updatingModelList_ = true;
    QSignalBlocker bCombo(*modelCombo_);
    std::unique_ptr<QSignalBlocker> bLineEdit;
    if (modelCombo_->lineEdit()) bLineEdit.reset(new QSignalBlocker(*modelCombo_->lineEdit()));
    modelCombo_->clear();

    // Partition: free first, then others, applying filter if any
    auto matches = [&](const ModelEntry& m){
        if (t.isEmpty()) return true;
        const QString hay = (m.id + " " + m.name).toLower();
        return hay.contains(t.toLower());
    };
    QVector<ModelEntry> freeList;
    QVector<ModelEntry> otherList;
    for (const auto& m : openRouterModels_) {
        if (!matches(m)) continue;
        const QString idLower = m.id.toLower();
        const QString nameLower = m.name.toLower();
        const bool isFree = idLower.contains("free") || nameLower.contains("free");
        if (isFree) freeList.push_back(m); else otherList.push_back(m);
    }
    auto addEntries = [&](const QVector<ModelEntry>& list){
        for (const auto& m : list) {
            const QString label = m.name.isEmpty() ? m.id : QString("%1 (%2)").arg(m.name, m.id);
            modelCombo_->addItem(label, m.id);
        }
    };
    addEntries(freeList);
    addEntries(otherList);
    if (modelCombo_->count() == 0) {
        // Fallback option
        modelCombo_->addItem("openrouter/auto", "openrouter/auto");
    }
    modelCombo_->setEnabled(true);
    updatingModelList_ = false;
}

void LlmSettingsDialog::fetchOpenRouterModels() {
    // Clear current list but keep editable text
    {
        QSignalBlocker bCombo(*modelCombo_);
        std::unique_ptr<QSignalBlocker> bLineEdit;
        if (modelCombo_->lineEdit()) bLineEdit.reset(new QSignalBlocker(*modelCombo_->lineEdit()));
        updatingModelList_ = true;
        openRouterModels_.clear();
        modelCombo_->clear();
        modelCombo_->addItem(tr("Carregando modelos do OpenRouter..."), "openrouter/auto");
        updatingModelList_ = false;
    }

    // Build request
    QUrl url("https://openrouter.ai/api/v1/models");
    const QString baseOverride = baseUrlEdit_->text().trimmed();
    if (!baseOverride.isEmpty()) {
        // Ensure it ends with /v1/models if a custom base is set
        const QString b = baseOverride.endsWith('/') ? baseOverride : (baseOverride + "/");
        url = QUrl(b + "v1/models");
    }
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    const QString key = apiKeyEdit_->text().trimmed();
    if (!key.isEmpty()) req.setRawHeader("Authorization", QByteArray("Bearer ") + key.toUtf8());
    req.setRawHeader("HTTP-Referer", QByteArray("https://rapport.tec.br/genai-reader"));
    const QByteArray title = QCoreApplication::applicationName().isEmpty() ? QByteArray("GenAI E-Book Reader") : QCoreApplication::applicationName().toUtf8();
    req.setRawHeader("X-Title", title);
    appendDebug(tr("[HTTP] GET %1 (Authorization=%2)")
                    .arg(url.toString(), key.isEmpty() ? "vazio" : "definido"));

    auto* reply = nam_->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        appendDebug(tr("[HTTP] Resposta recebida do OpenRouter. erro=%1")
                        .arg(reply->error() == QNetworkReply::NoError ? "NoError" : reply->errorString()));
        const auto guard = std::unique_ptr<QNetworkReply, void(*)(QNetworkReply*)>(reply, [](QNetworkReply* r){ r->deleteLater(); });
        if (reply->error() != QNetworkReply::NoError) {
            const QString errText = tr("Não foi possível obter a lista de modelos do OpenRouter.\n\nErro: %1\n\nDeseja verificar sua conexão e tentar novamente?").arg(reply->errorString());
            auto btn = QMessageBox::question(this, tr("Falha ao listar modelos"), errText,
                                             QMessageBox::Retry | QMessageBox::Cancel, QMessageBox::Retry);
            if (btn == QMessageBox::Retry && retryOpenRouter_ < 3) {
                ++retryOpenRouter_;
                appendDebug(tr("[HTTP] Retry %1/3").arg(QString::number(retryOpenRouter_))); 
                fetchOpenRouterModels();
            } else {
                modelCombo_->clear();
                modelCombo_->addItem(tr("Falha ao listar modelos do OpenRouter"), "openrouter/auto");
                retryOpenRouter_ = 0;
            }
            return;
        }
        const QByteArray raw = reply->readAll();
        appendDebug(tr("[HTTP] Tamanho da resposta: %1 bytes").arg(QString::number(raw.size())));
        const QJsonDocument doc = QJsonDocument::fromJson(raw);
        if (!doc.isObject()) {
            const QString errText = tr("Resposta inválida do OpenRouter.\n\nDeseja tentar novamente?");
            auto btn = QMessageBox::question(this, tr("Dados inválidos"), errText,
                                             QMessageBox::Retry | QMessageBox::Cancel, QMessageBox::Retry);
            if (btn == QMessageBox::Retry && retryOpenRouter_ < 3) {
                ++retryOpenRouter_;
                appendDebug(tr("[HTTP] Retry %1/3 (json invalido)").arg(QString::number(retryOpenRouter_))); 
                fetchOpenRouterModels();
            } else {
                modelCombo_->clear();
                modelCombo_->addItem(tr("Não foi possível carregar modelos do OpenRouter"), "openrouter/auto");
                retryOpenRouter_ = 0;
            }
            return;
        }
        const QJsonObject obj = doc.object();
        const QJsonArray data = obj.value("data").toArray();
        openRouterModels_.clear();
        for (const auto& it : data) {
            const QJsonObject m = it.toObject();
            ModelEntry e; e.id = m.value("id").toString();
            // some entries include 'name' or nested org meta, prefer name if available
            e.name = m.value("name").toString();
            if (!e.id.isEmpty()) openRouterModels_.push_back(e);
        }
        appendDebug(tr("[MODELS] OpenRouter carregou %1 modelos").arg(QString::number(openRouterModels_.size())));
        // Populate with free-first ordering
        const QString currentFilter = modelCombo_->lineEdit() ? modelCombo_->lineEdit()->text() : QString();
        appendDebug(tr("[MODELS] Aplicando filtro atual='%1'").arg(currentFilter));
        applyModelFilter(currentFilter);
        // Try to keep previously selected model if present in settings
        QSettings s; const QString currentModel = s.value("ai/model").toString();
        int idx = modelCombo_->findData(currentModel);
        if (idx >= 0) {
            QSignalBlocker bCombo(*modelCombo_);
            std::unique_ptr<QSignalBlocker> bLineEdit;
            if (modelCombo_->lineEdit()) bLineEdit.reset(new QSignalBlocker(*modelCombo_->lineEdit()));
            modelCombo_->setCurrentIndex(idx);
        }
        retryOpenRouter_ = 0; // success resets counter
        modelCombo_->setEnabled(true);
    });
}

// ---- Debug helpers ----
void LlmSettingsDialog::appendDebug(const QString& line) {
    const QString msg = QString("%1 %2")
                            .arg(QTime::currentTime().toString("HH:mm:ss"))
                            .arg(line);
    qInfo().noquote() << "[LlmSettingsDialog]" << msg;
    if (testOutput_) {
        auto cursor = testOutput_->textCursor();
        cursor.movePosition(QTextCursor::End);
        testOutput_->setTextCursor(cursor);
        testOutput_->appendPlainText(msg);
    }
}

QString LlmSettingsDialog::currentProviderId() const {
    return providerCombo_ ? providerCombo_->currentData().toString() : QString();
}

void LlmSettingsDialog::fetchOpenWebUiModels() {
    // Default base URL for OpenWebUI if empty
    QUrl url("http://localhost:3000/api/v1/models");
    const QString baseOverride = baseUrlEdit_->text().trimmed();
    if (!baseOverride.isEmpty()) {
        const QString b = baseOverride.endsWith('/') ? baseOverride : (baseOverride + "/");
        url = QUrl(b + "v1/models");
    }
    modelCombo_->clear();
    modelCombo_->addItem(tr("Carregando modelos do OpenWebUI..."), "");

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    const QString key = apiKeyEdit_->text().trimmed();
    if (!key.isEmpty()) req.setRawHeader("Authorization", QByteArray("Bearer ") + key.toUtf8());
    auto* reply = nam_->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const auto guard = std::unique_ptr<QNetworkReply, void(*)(QNetworkReply*)>(reply, [](QNetworkReply* r){ r->deleteLater(); });
        modelCombo_->clear();
        if (reply->error() != QNetworkReply::NoError) {
            const QString errText = tr("Não foi possível obter a lista de modelos do OpenWebUI.\n\nErro: %1\n\nDeseja verificar sua conexão e tentar novamente?")
                                      .arg(reply->errorString());
            auto btn = QMessageBox::question(this, tr("Falha ao listar modelos"), errText,
                                             QMessageBox::Retry | QMessageBox::Cancel, QMessageBox::Retry);
            if (btn == QMessageBox::Retry && retryOpenWebUI_ < 3) {
                ++retryOpenWebUI_;
                fetchOpenWebUiModels();
            } else {
                modelCombo_->addItem(tr("Falha ao listar modelos do OpenWebUI"), "");
                retryOpenWebUI_ = 0;
            }
            return;
        }
        const QByteArray raw = reply->readAll();
        const QJsonDocument doc = QJsonDocument::fromJson(raw);
        const QJsonObject obj = doc.object();
        const QJsonArray data = obj.value("data").toArray();
        for (const auto& it : data) {
            const QJsonObject m = it.toObject();
            const QString id = m.value("id").toString();
            if (!id.isEmpty()) modelCombo_->addItem(id, id);
        }
        if (modelCombo_->count() == 0) {
            modelCombo_->addItem("gpt-4o-mini", "gpt-4o-mini");
            modelCombo_->addItem("gpt-4o", "gpt-4o");
        }
        modelCombo_->setCurrentIndex(0);
        retryOpenWebUI_ = 0; // success resets counter
        modelCombo_->setEnabled(true);
    });
}

void LlmSettingsDialog::verifyOrAssistOllama() {
    // Guide the user to install and pull models
    QMessageBox::StandardButton btn = QMessageBox::information(
        this,
        tr("Ollama não encontrado"),
                tr("Não foi possível conectar ao Ollama em 127.0.0.1:11434.\n\n"
                   "Passos sugeridos:\n"
                   "1) Instale o Ollama: https://ollama.com/download\n"
                   "2) Inicie o serviço do Ollama (daemon).\n"
                   "3) Faça o pull de um modelo recomendado:\n"
                   "   - granite3.3:8b (recomendado) ou\n"
                   "   - granite3.3:2b (mais leve)\n\n"
                   "Exemplo de comandos:\n"
                   "  ollama pull granite3.3:8b\n"
                   "  ou\n"
                   "  ollama pull granite3.3:2b\n\n"
                   "Após concluir, clique em 'Repetir' para atualizar a lista de modelos."),
        QMessageBox::Retry | QMessageBox::Cancel,
        QMessageBox::Retry);
    if (btn == QMessageBox::Retry && retryOllama_ < 3) {
        ++retryOllama_;
        verifyOrAssistOllama();
    } else {
        modelCombo_->clear();
        modelCombo_->addItem(tr("Ollama indisponível"), "");
        retryOllama_ = 0;
    }
}

void LlmSettingsDialog::fetchGenerativaModels() {
    // Fetch GenerAtiva models similar to OpenWebUI (OpenAI-style list endpoint)
    // Default base from settings or use known default
    QUrl url("https://generativa.rapport.tec.br/api/v1/models");
    const QString baseOverride = baseUrlEdit_->text().trimmed();
    if (!baseOverride.isEmpty()) {
        const QString b = baseOverride.endsWith('/') ? baseOverride : (baseOverride + "/");
        url = QUrl(b + "v1/models");
    }
    modelCombo_->clear();
    modelCombo_->addItem(tr("Carregando modelos da GenerAtiva..."), "");

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    const QString key = apiKeyEdit_->text().trimmed();
    if (!key.isEmpty()) req.setRawHeader("Authorization", QByteArray("Bearer ") + key.toUtf8());
    auto* reply = nam_->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const auto guard = std::unique_ptr<QNetworkReply, void(*)(QNetworkReply*)>(reply, [](QNetworkReply* r){ r->deleteLater(); });
        modelCombo_->clear();
        if (reply->error() != QNetworkReply::NoError) {
            const QString errText = tr("Não foi possível obter a lista de modelos da GenerAtiva.\n\nErro: %1\n\nDeseja verificar sua conexão e tentar novamente?")
                                      .arg(reply->errorString());
            auto btn = QMessageBox::question(this, tr("Falha ao listar modelos"), errText,
                                             QMessageBox::Retry | QMessageBox::Cancel, QMessageBox::Retry);
            if (btn == QMessageBox::Retry && retryGenerativa_ < 3) {
                ++retryGenerativa_;
                fetchGenerativaModels();
            } else {
                modelCombo_->addItem(tr("Falha ao listar modelos da GenerAtiva"), "");
                retryGenerativa_ = 0;
            }
            return;
        }
        const QByteArray raw = reply->readAll();
        const QJsonDocument doc = QJsonDocument::fromJson(raw);
        const QJsonObject obj = doc.object();
        const QJsonArray data = obj.value("data").toArray();
        // Prioritize Granite3.3 variants if present
        QStringList models;
        for (const auto& it : data) {
            const QJsonObject m = it.toObject();
            const QString id = m.value("id").toString();
            if (!id.isEmpty()) models << id;
        }
        // Show preferred first
        QStringList preferred;
        if (models.contains("granite3.3:8b")) preferred << "granite3.3:8b";
        if (models.contains("granite3.3:2b")) preferred << "granite3.3:2b";
        for (const auto& p : preferred) modelCombo_->addItem(p, p);
        for (const auto& id : models) { if (!preferred.contains(id)) modelCombo_->addItem(id, id); }
        if (modelCombo_->count() == 0) {
            modelCombo_->addItem(tr("Nenhum modelo encontrado na GenerAtiva"), "");
        } else {
            // Default selection: Granite3.3:8b or 2b if present, else first
            int idx = -1;
            idx = modelCombo_->findData("granite3.3:8b");
            if (idx < 0) idx = modelCombo_->findData("granite3.3:2b");
            if (idx < 0) idx = 0;
            modelCombo_->setCurrentIndex(idx);
        }
        retryGenerativa_ = 0; // success resets counter
        modelCombo_->setEnabled(true);
    });
}

void LlmSettingsDialog::fetchOpenAiModels() {
    // OpenAI: static list (we can expand later or fetch with API if desired)
    modelCombo_->clear();
    modelCombo_->addItem("gpt-4o-mini (recomendado)", "gpt-4o-mini");
    modelCombo_->addItem("gpt-4o", "gpt-4o");
    modelCombo_->addItem("gpt-3.5-turbo", "gpt-3.5-turbo");
    modelCombo_->setCurrentIndex(0);
}

// ---- Per-provider API key helpers ----
QString LlmSettingsDialog::perProviderApiKeyGroup(const QString& provider) const {
    return QStringLiteral("ai/api_keys/%1").arg(provider);
}

QString LlmSettingsDialog::loadApiKeyForProvider(const QString& provider) const {
    QSettings s;
    return s.value(perProviderApiKeyGroup(provider)).toString();
}

void LlmSettingsDialog::saveApiKeyForProvider(const QString& provider, const QString& key) {
    QSettings s;
    s.setValue(perProviderApiKeyGroup(provider), key);
}

void LlmSettingsDialog::migrateLegacyKeyIfNeeded(const QString& provider) const {
    QSettings s;
    const QString dst = perProviderApiKeyGroup(provider);
    if (s.contains(QStringLiteral("ai/api_key")) && !s.contains(dst)) {
        const QString legacy = s.value(QStringLiteral("ai/api_key")).toString();
        if (!legacy.isEmpty()) {
            s.setValue(dst, legacy);
            s.remove(QStringLiteral("ai/api_key"));
        }
    }
}
