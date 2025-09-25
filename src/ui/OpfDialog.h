#pragma once

#include <QDialog>
#include <QString>
#include "ui/OpfStore.h"

class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QLabel;
class QProgressBar;

class OpfDialog : public QDialog {
    Q_OBJECT
public:
    explicit OpfDialog(QWidget* parent = nullptr);

    void setData(const OpfData& data);
    OpfData data() const;

    // Visual feedback while OPF is being generated in background
    void setBusy(bool busy);
    void setStatusMessage(const QString& msg);

signals:
    void requestCompleteWithAi();
    void requestRegenerateOpf();

private slots:
    void toggleEdit();

private:
    void setEditable(bool on);

    QLineEdit* titleEdit_ {nullptr};
    QLineEdit* authorEdit_ {nullptr};
    QLineEdit* publisherEdit_ {nullptr};
    QLineEdit* languageEdit_ {nullptr};
    QLineEdit* idEdit_ {nullptr};
    QPlainTextEdit* descriptionEdit_ {nullptr};
    QPlainTextEdit* summaryEdit_ {nullptr};
    QLineEdit* keywordsEdit_ {nullptr};
    QPushButton* editBtn_ {nullptr};
    QPushButton* aiBtn_ {nullptr};
    QPushButton* regenBtn_ {nullptr};
    QLabel* statusLabel_ {nullptr};
    QProgressBar* progress_ {nullptr};
    bool busy_ {false};
};
