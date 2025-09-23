#pragma once

#include <QDialog>
#include <QMap>
#include <QString>
#include <QVector>

#include "ui/OpfStore.h"

class QTableWidget;

// Dialog to let the user choose per-field which provider's value to use.
// Columns: Field | Atual | IA | Google | Amazon | Escolha
class OpfMergeDialog : public QDialog {
    Q_OBJECT
public:
    enum class Source { Current, AI, Google, Amazon };

    struct Row {
        QString key;      // e.g., "title"
        QString label;    // e.g., "Título"
        QString cur;
        QString ai;
        QString google;
        QString amazon;
        Source chosen { Source::Current };
    };

    explicit OpfMergeDialog(QWidget* parent = nullptr);

    // Provide values per field for each source
    void setRows(const QVector<Row>& rows);

    // Return selected OPF data after accept()
    OpfData selectedOpf(const OpfData& current) const;

private:
    QTableWidget* table_ {nullptr};
    QVector<Row> rows_;
};
