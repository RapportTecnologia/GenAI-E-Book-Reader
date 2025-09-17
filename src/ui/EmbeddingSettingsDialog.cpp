#include "ui/EmbeddingSettingsDialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QDir>
#include <QStandardPaths>
#include <QSettings>
#include <QIntValidator>

namespace {
// Default DB path: ~/.cache/br.tec.rapport.genai-reader/
static QString defaultDbPath() {
    const QString base = QDir::home().filePath(".cache");
    return QDir(base).filePath("br.tec.rapport.genai-reader");
}
}

EmbeddingSettingsDialog::EmbeddingSettingsDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Configurações de Embeddings"));
    resize(640, 360);

    auto* root = new QVBoxLayout(this);

    // Form
    auto* form = new QFormLayout();
    providerCombo_ = new QComboBox(this);
    modelCombo_ = new QComboBox(this);
    baseUrlEdit_ = new QLineEdit(this);
    apiKeyEdit_ = new QLineEdit(this);
    apiKeyEdit_->setEchoMode(QLineEdit::Password);
    dbPathEdit_ = new QLineEdit(this);
    chunkSizeEdit_ = new QLineEdit(this);
    chunkOverlapEdit_ = new QLineEdit(this);
    batchSizeEdit_ = new QLineEdit(this);
    pagesPerStageEdit_ = new QLineEdit(this);
    pauseMsBetweenBatchesEdit_ = new QLineEdit(this);
    // validators
    chunkSizeEdit_->setValidator(new QIntValidator(1, 20000, chunkSizeEdit_));
    chunkOverlapEdit_->setValidator(new QIntValidator(0, 10000, chunkOverlapEdit_));
    batchSizeEdit_->setValidator(new QIntValidator(1, 512, batchSizeEdit_));
    pagesPerStageEdit_->setValidator(new QIntValidator(1, 100000, pagesPerStageEdit_));
    pauseMsBetweenBatchesEdit_->setValidator(new QIntValidator(0, 60000, pauseMsBetweenBatchesEdit_));
    chunkSizeEdit_->setPlaceholderText(tr("ex.: 1000"));
    chunkOverlapEdit_->setPlaceholderText(tr("ex.: 200"));
    batchSizeEdit_->setPlaceholderText(tr("ex.: 16"));
    pagesPerStageEdit_->setPlaceholderText(tr("ex.: 25 (páginas por etapa)"));
    pauseMsBetweenBatchesEdit_->setPlaceholderText(tr("ex.: 150 (ms entre lotes)"));

    populateProviders();

    form->addRow(tr("Provedor"), providerCombo_);
    form->addRow(tr("Modelo de Embeddings"), modelCombo_);
    form->addRow(tr("Base URL"), baseUrlEdit_);
    form->addRow(tr("API Key"), apiKeyEdit_);
    form->addRow(tr("Banco (ChromaDB)"), dbPathEdit_);
    form->addRow(tr("Tamanho do chunk"), chunkSizeEdit_);
    form->addRow(tr("Sobreposição do chunk"), chunkOverlapEdit_);
    form->addRow(tr("Tamanho do lote (batch)"), batchSizeEdit_);
    form->addRow(tr("Páginas por etapa"), pagesPerStageEdit_);
    form->addRow(tr("Pausa entre lotes (ms)"), pauseMsBetweenBatchesEdit_);

    root->addLayout(form);

    // Warning + Rebuild
    warningLabel_ = new QLabel(this);
    warningLabel_->setWordWrap(true);
    warningLabel_->setStyleSheet("color:#b36b00;font-weight:bold");
    warningLabel_->setText(tr("Ao mudar o modelo de embeddings, é preciso recriar os vetores de embeddings."));
    root->addWidget(warningLabel_);

    btnRebuild_ = new QPushButton(tr("Recriar Embeddings do documento aberto"), this);
    root->addWidget(btnRebuild_);

    // Buttons
    buttons_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    root->addWidget(buttons_);

    connect(buttons_, &QDialogButtonBox::accepted, this, &EmbeddingSettingsDialog::accept);
    connect(buttons_, &QDialogButtonBox::rejected, this, &EmbeddingSettingsDialog::reject);
    connect(providerCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EmbeddingSettingsDialog::onProviderChanged);
    connect(btnRebuild_, &QPushButton::clicked, this, &EmbeddingSettingsDialog::onRebuildClicked);

    loadFromSettings();
}

void EmbeddingSettingsDialog::populateProviders() {
    providerCombo_->clear();
    // Map visible label -> provider key
    providerCombo_->addItem(tr("GenerAtiva API"), "generativa");
    providerCombo_->addItem(tr("OpenAI"), "openai");
    providerCombo_->addItem(tr("Ollama local"), "ollama");
    providerCombo_->addItem(tr("OpenWebUI (retrieval/ef)"), "openwebui");
    providerCombo_->addItem(tr("SentenceTransformers (local)"), "sentence_transformers");
    // Trigger initial model list
    onProviderChanged(providerCombo_->currentIndex());
}

void EmbeddingSettingsDialog::populateModelsFor(const QString& provider) {
    modelCombo_->clear();
    if (provider == QLatin1String("generativa")) {
        // Model is internally defined by the provider
        modelCombo_->addItem(QStringLiteral("(definido pelo provedor)"), QString());
    } else if (provider == QLatin1String("openai")) {
        // Only embedding-capable OpenAI models
        modelCombo_->addItem(QStringLiteral("text-embedding-3-large"), QStringLiteral("text-embedding-3-large"));
        modelCombo_->addItem(QStringLiteral("text-embedding-3-small"), QStringLiteral("text-embedding-3-small"));
        modelCombo_->addItem(QStringLiteral("text-embedding-ada-002"), QStringLiteral("text-embedding-ada-002"));
    } else if (provider == QLatin1String("ollama")) {
        modelCombo_->addItem(QStringLiteral("nomic-embed-text:latest"), QStringLiteral("nomic-embed-text:latest"));
    } else if (provider == QLatin1String("openwebui")) {
        // Model is internally defined by the provider
        modelCombo_->addItem(QStringLiteral("(definido pelo provedor)"), QString());
    } else { // sentence_transformers
        // Offer a few common local models
        modelCombo_->addItem(QStringLiteral("all-MiniLM-L6-v2"), QStringLiteral("sentence-transformers/all-MiniLM-L6-v2"));
        modelCombo_->addItem(QStringLiteral("multi-qa-MiniLM-L6-cos-v1"), QStringLiteral("sentence-transformers/multi-qa-MiniLM-L6-cos-v1"));
    }
    if (modelCombo_->count() > 0) modelCombo_->setCurrentIndex(0);
}

void EmbeddingSettingsDialog::onProviderChanged(int) {
    const QString provider = providerCombo_->currentData().toString();
    populateModelsFor(provider);
    // Tweak field visibility/enabled based on provider
    const bool needsHttp = (provider == QLatin1String("generativa") || provider == QLatin1String("openai") || provider == QLatin1String("openwebui"));
    baseUrlEdit_->setEnabled(needsHttp);
    apiKeyEdit_->setEnabled(needsHttp);
    // Model selection is only meaningful for OpenAI, Ollama, and sentence_transformers
    const bool usesModel = (provider == QLatin1String("openai") || provider == QLatin1String("ollama") || provider == QLatin1String("sentence_transformers"));
    modelCombo_->setEnabled(usesModel);
    modelCombo_->setToolTip(usesModel ? QString() : tr("O modelo é definido internamente pelo provedor."));
    if (provider == QLatin1String("openai")) {
        if (baseUrlEdit_->text().trimmed().isEmpty()) {
            baseUrlEdit_->setText(QStringLiteral("https://api.openai.com/v1"));
        }
    } else if (provider == QLatin1String("openwebui")) {
        if (baseUrlEdit_->text().trimmed().isEmpty()) {
            baseUrlEdit_->setText(QStringLiteral("http://localhost:8080"));
        }
    }
}

void EmbeddingSettingsDialog::loadFromSettings() {
    QSettings s;
    const QString provider = s.value("emb/provider", "generativa").toString();
    const QString model = s.value("emb/model", "nomic-embed-text:latest").toString();
    const QString baseUrl = s.value("emb/base_url", "https://generativa.rapport.tec.br").toString();
    const QString apiKey = s.value("emb/api_key").toString();
    const QString dbPath = s.value("emb/db_path", defaultDbPath()).toString();
    const int chunkSize = s.value("emb/chunk_size", 1000).toInt();
    const int chunkOverlap = s.value("emb/chunk_overlap", 200).toInt();
    const int batchSize = s.value("emb/batch_size", 16).toInt();
    const int pagesPerStage = s.value("emb/pages_per_stage", -1).toInt();
    const int pauseMsBetweenBatches = s.value("emb/pause_ms_between_batches", 0).toInt();

    int pidx = providerCombo_->findData(provider);
    if (pidx < 0) pidx = 0;
    providerCombo_->setCurrentIndex(pidx);
    populateModelsFor(providerCombo_->currentData().toString());

    int midx = modelCombo_->findData(model);
    if (midx < 0) midx = 0;
    modelCombo_->setCurrentIndex(midx);

    baseUrlEdit_->setText(baseUrl);
    apiKeyEdit_->setText(apiKey);
    dbPathEdit_->setText(dbPath);
    chunkSizeEdit_->setText(QString::number(chunkSize));
    chunkOverlapEdit_->setText(QString::number(chunkOverlap));
    batchSizeEdit_->setText(QString::number(batchSize));
    if (pagesPerStage > 0) pagesPerStageEdit_->setText(QString::number(pagesPerStage)); else pagesPerStageEdit_->clear();
    pauseMsBetweenBatchesEdit_->setText(QString::number(pauseMsBetweenBatches));
}

void EmbeddingSettingsDialog::saveToSettings() {
    QSettings s;
    s.setValue("emb/provider", providerCombo_->currentData().toString());
    s.setValue("emb/model", modelCombo_->currentData().toString());
    s.setValue("emb/base_url", baseUrlEdit_->text().trimmed());
    s.setValue("emb/api_key", apiKeyEdit_->text());
    s.setValue("emb/db_path", dbPathEdit_->text().trimmed().isEmpty() ? defaultDbPath() : dbPathEdit_->text().trimmed());
    bool ok1=false, ok2=false, ok3=false;
    const int chunkSize = chunkSizeEdit_->text().toInt(&ok1);
    const int chunkOverlap = chunkOverlapEdit_->text().toInt(&ok2);
    const int batchSize = batchSizeEdit_->text().toInt(&ok3);
    s.setValue("emb/chunk_size", ok1 && chunkSize>0 ? chunkSize : 1000);
    s.setValue("emb/chunk_overlap", ok2 && chunkOverlap>=0 ? chunkOverlap : 200);
    s.setValue("emb/batch_size", ok3 && batchSize>0 ? batchSize : 16);
    bool ok4=false, ok5=false;
    const int pagesPerStage = pagesPerStageEdit_->text().toInt(&ok4);
    const int pauseMsBetweenBatches = pauseMsBetweenBatchesEdit_->text().toInt(&ok5);
    s.setValue("emb/pages_per_stage", ok4 && pagesPerStage>0 ? pagesPerStage : -1);
    s.setValue("emb/pause_ms_between_batches", ok5 && pauseMsBetweenBatches>=0 ? pauseMsBetweenBatches : 0);
}

void EmbeddingSettingsDialog::onRebuildClicked() {
    // Persist first so listeners use updated config
    saveToSettings();
    emit rebuildRequested();
}

void EmbeddingSettingsDialog::accept() {
    saveToSettings();
    QDialog::accept();
}
