#pragma once

#include <QDialog>

class QTabWidget;
class QLineEdit;
class QComboBox;
class QDialogButtonBox;
class QPlainTextEdit;

class DictionarySettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit DictionarySettingsDialog(QWidget* parent = nullptr);

private slots:
    void accept() override;

private:
    void loadFromSettings();
    void saveToSettings();

    QTabWidget* tabs_ {nullptr};
    QDialogButtonBox* buttons_ {nullptr};

    // Service selection
    QComboBox* serviceCombo_ {nullptr};
    QPlainTextEdit* llmPromptEdit_ {nullptr};

    // LibreTranslate settings
    QLineEdit* libreApiUrlEdit_ {nullptr};
    QLineEdit* libreApiKeyEdit_ {nullptr};
    QLineEdit* libreSourceLangEdit_ {nullptr};
    QLineEdit* libreTargetLangEdit_ {nullptr};

    // OMW settings (initially just a placeholder for the URL)
    QLineEdit* omwApiUrlEdit_ {nullptr};
};
