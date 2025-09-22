#pragma once

#include <QDialog>
#include "ui/OpfStore.h"

class QLineEdit;
class QPlainTextEdit;
class QPushButton;

class OpfDialog : public QDialog {
    Q_OBJECT
public:
    explicit OpfDialog(QWidget* parent = nullptr);

    void setData(const OpfData& data);
    OpfData data() const;

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
};
