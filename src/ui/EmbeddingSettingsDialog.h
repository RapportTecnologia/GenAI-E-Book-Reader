#pragma once

#include <QDialog>
#include <QString>

class QComboBox;
class QLineEdit;
class QLabel;
class QDialogButtonBox;
class QPushButton;

class EmbeddingSettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit EmbeddingSettingsDialog(QWidget* parent = nullptr);

signals:
    void rebuildRequested(); // Emitted when user clicks "Recriar Embeddings"

private slots:
    void onProviderChanged(int index);
    void onRebuildClicked();
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
    QLineEdit* dbPathEdit_ {nullptr};
    QLineEdit* chunkSizeEdit_ {nullptr};
    QLineEdit* chunkOverlapEdit_ {nullptr};
    QLineEdit* batchSizeEdit_ {nullptr};
    QLineEdit* pagesPerStageEdit_ {nullptr};
    QLineEdit* pauseMsBetweenBatchesEdit_ {nullptr};
    // Retrieval params
    QComboBox* similarityCombo_ {nullptr};
    QLineEdit* topKEdit_ {nullptr};

    QLabel* warningLabel_ {nullptr};
    QPushButton* btnRebuild_ {nullptr};
    QDialogButtonBox* buttons_ {nullptr};
};
