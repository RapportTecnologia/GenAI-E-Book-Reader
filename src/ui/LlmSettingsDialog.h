#pragma once

#include <QDialog>
#include <QMap>
#include <QString>
#include <QVector>
#include <QPushButton>
#include <QCheckBox>

class QComboBox;
class QLineEdit;
class QTabWidget;
class QPlainTextEdit;
class QDialogButtonBox;
class QNetworkAccessManager;
class QLabel;

class LlmSettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit LlmSettingsDialog(QWidget* parent = nullptr);

private slots:
    void onProviderChanged(int index);
    void accept() override;
    void onApiKeyEdited(const QString&);
    void onModelFilterEdited(const QString& text);
    void onTestModel();
    void onModelChanged(int index);

private:
    // Debugging helpers
    void appendDebug(const QString& line);
    QString currentProviderId() const;

    void loadFromSettings();
    void saveToSettings();
    void populateProviders();
    void populateModelsFor(const QString& provider);
    void fetchOpenRouterModels();
    void applyModelFilter(const QString& filterText);
    QString perProviderApiKeyGroup(const QString& provider) const;
    QString loadApiKeyForProvider(const QString& provider) const;
    void saveApiKeyForProvider(const QString& provider, const QString& key);
    void migrateLegacyKeyIfNeeded(const QString& provider) const;
    // Courtesy notice for OpenRouter provider (shows embedded video when possible)
    void showOpenRouterCourtesyDialog();
    // Ollama support
    void verifyOrAssistOllama();
    void fetchOpenWebUiModels();
    void fetchGenerativaModels();
    void fetchOpenAiModels();
    void fetchOllamaModels();
    void updateFunctionCallingSupport();
    bool inferFunctionCallingSupport(const QString& provider,
                                     const QString& modelId) const;

    QComboBox* providerCombo_ {nullptr};
    QComboBox* modelCombo_ {nullptr};
    QLineEdit* baseUrlEdit_ {nullptr};
    QLineEdit* apiKeyEdit_ {nullptr};

    QTabWidget* tabs_ {nullptr};
    QPlainTextEdit* promptSynonyms_ {nullptr};
    QPlainTextEdit* promptSummaries_ {nullptr};
    QPlainTextEdit* promptExplanations_ {nullptr};
    QPlainTextEdit* promptChat_ {nullptr};
    QCheckBox* functionCallingCheck_ {nullptr};
    // Cache of capabilities obtained from providers when available
    QMap<QString, bool> modelSupportsFunctions_; // key: provider + "::" + modelId

    QDialogButtonBox* buttons_ {nullptr};

    // Test model UI
    QPushButton* testButton_ {nullptr};
    QPlainTextEdit* testOutput_ {nullptr};
    QLabel* testLabel_ {nullptr};

    // Networking and data for OpenRouter models
    QNetworkAccessManager* nam_ {nullptr};
    struct ModelEntry { QString id; QString name; };
    QVector<ModelEntry> openRouterModels_; // unfiltered, fetched from API

    // Data for Ollama models
    QVector<QString> ollamaModels_;

    // Retry counters (max 3 attempts)
    int retryOpenWebUI_ {0};
    int retryOpenRouter_ {0};
    int retryOllama_ {0};
    int retryGenerativa_ {0};

    // Reentrancy guard for model list updates/filters
    bool updatingModelList_ {false};
};

