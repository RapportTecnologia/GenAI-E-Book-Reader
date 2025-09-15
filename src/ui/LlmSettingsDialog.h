#pragma once

#include <QDialog>
#include <QMap>
#include <QString>

class QComboBox;
class QLineEdit;
class QTabWidget;
class QPlainTextEdit;
class QDialogButtonBox;

class LlmSettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit LlmSettingsDialog(QWidget* parent = nullptr);

private slots:
    void onProviderChanged(int index);
    void accept() override;

private:
    void loadFromSettings();
    void saveToSettings();
    void populateProviders();
    void populateModelsFor(const QString& provider);

    QComboBox* providerCombo_ {nullptr};
    QComboBox* modelCombo_ {nullptr};
    QLineEdit* baseUrlEdit_ {nullptr};
    QLineEdit* apiKeyEdit_ {nullptr};

    QTabWidget* tabs_ {nullptr};
    QPlainTextEdit* promptSynonyms_ {nullptr};
    QPlainTextEdit* promptSummaries_ {nullptr};
    QPlainTextEdit* promptExplanations_ {nullptr};
    QPlainTextEdit* promptChat_ {nullptr};

    QDialogButtonBox* buttons_ {nullptr};
};
