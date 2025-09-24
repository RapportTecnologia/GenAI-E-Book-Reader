#include "ui/OpfMergeDialog.h"

#include <QTableWidget>
#include <QHeaderView>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>

namespace {
struct FieldSpec { const char* key; const char* label; };
static const FieldSpec kFields[] = {
    {"title",        "Título"},
    {"author",       "Autor"},
    {"publisher",    "Editora"},
    {"edition",      "Edição"},
    {"language",     "Idioma"},
    {"isbn",         "ISBN"},
    {"keywords",     "Palavras-chave"},
    {"description",  "Descrição"},
    {"summary",      "Resumo"},
};
static QString valueForKey(const OpfData& d, const QString& key) {
    if (key == QLatin1String("title")) return d.title;
    if (key == QLatin1String("author")) return d.author;
    if (key == QLatin1String("publisher")) return d.publisher;
    if (key == QLatin1String("edition")) return d.edition;
    if (key == QLatin1String("language")) return d.language;
    if (key == QLatin1String("isbn")) return d.isbn;
    if (key == QLatin1String("keywords")) return d.keywords;
    if (key == QLatin1String("description")) return d.description;
    if (key == QLatin1String("summary")) return d.summary;
    return {};
}
static void setValueForKey(OpfData& d, const QString& key, const QString& v) {
    if (key == QLatin1String("title")) d.title = v;
    else if (key == QLatin1String("author")) d.author = v;
    else if (key == QLatin1String("publisher")) d.publisher = v;
    else if (key == QLatin1String("edition")) d.edition = v;
    else if (key == QLatin1String("language")) d.language = v;
    else if (key == QLatin1String("isbn")) d.isbn = v;
    else if (key == QLatin1String("keywords")) d.keywords = v;
    else if (key == QLatin1String("description")) d.description = v;
    else if (key == QLatin1String("summary")) d.summary = v;
}
}

OpfMergeDialog::OpfMergeDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Selecionar metadados por provedor"));
    auto* lay = new QVBoxLayout(this);

    table_ = new QTableWidget(this);
    table_->setColumnCount(6);
    QStringList headers; headers << tr("Campo") << tr("Atual") << tr("IA") << tr("Google") << tr("Amazon") << tr("Escolha");
    table_->setHorizontalHeaderLabels(headers);
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->verticalHeader()->setVisible(false);
    table_->setSelectionMode(QAbstractItemView::NoSelection);
    lay->addWidget(table_);

    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    lay->addWidget(btns);
    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

static QString makeWrappedPreview(const QString& text, int maxLines = 3, int maxCharsPerLine = 60) {
    // Create a short multi-line preview with explicit line breaks and ellipsis if needed
    QString s = text;
    s.replace('\n', ' ');
    QStringList outLines;
    int idx = 0;
    for (int line = 0; line < maxLines && idx < s.size(); ++line) {
        const int remain = s.size() - idx;
        const int take = qMin(maxCharsPerLine, remain);
        outLines << s.mid(idx, take);
        idx += take;
    }
    QString preview = outLines.join("\n");
    if (idx < s.size()) preview += QStringLiteral("…");
    return preview;
}

void OpfMergeDialog::setRows(const QVector<Row>& rows) {
    rows_ = rows;
    table_->clearContents();
    table_->setRowCount(rows_.size());
    table_->setWordWrap(true);

    for (int r = 0; r < rows_.size(); ++r) {
        const Row& row = rows_[r];
        const bool isSummary = (row.key == QLatin1String("summary"));
        auto* itemField = new QTableWidgetItem(row.label);
        auto* itemCur   = new QTableWidgetItem(isSummary ? makeWrappedPreview(row.cur)    : row.cur);
        auto* itemAi    = new QTableWidgetItem(isSummary ? makeWrappedPreview(row.ai)     : row.ai);
        auto* itemG     = new QTableWidgetItem(isSummary ? makeWrappedPreview(row.google) : row.google);
        auto* itemA     = new QTableWidgetItem(isSummary ? makeWrappedPreview(row.amazon) : row.amazon);
        if (isSummary) {
            // Show full content on hover
            itemCur->setToolTip(row.cur);
            itemAi->setToolTip(row.ai);
            itemG->setToolTip(row.google);
            itemA->setToolTip(row.amazon);
        }
        itemField->setFlags(itemField->flags() & ~Qt::ItemIsEditable);
        itemCur->setFlags(itemCur->flags() & ~Qt::ItemIsEditable);
        itemAi->setFlags(itemAi->flags() & ~Qt::ItemIsEditable);
        itemG->setFlags(itemG->flags() & ~Qt::ItemIsEditable);
        itemA->setFlags(itemA->flags() & ~Qt::ItemIsEditable);
        table_->setItem(r, 0, itemField);
        table_->setItem(r, 1, itemCur);
        table_->setItem(r, 2, itemAi);
        table_->setItem(r, 3, itemG);
        table_->setItem(r, 4, itemA);

        auto* combo = new QComboBox(table_);
        combo->addItem(tr("Atual"), static_cast<int>(Source::Current));
        combo->addItem(tr("IA"), static_cast<int>(Source::AI));
        combo->addItem(tr("Google"), static_cast<int>(Source::Google));
        combo->addItem(tr("Amazon"), static_cast<int>(Source::Amazon));
        combo->setCurrentIndex(static_cast<int>(row.chosen));
        table_->setCellWidget(r, 5, combo);
    }

    table_->resizeColumnsToContents();
    table_->resizeRowsToContents();
}

OpfData OpfMergeDialog::selectedOpf(const OpfData& current) const {
    OpfData out = current; // preserve identifier/source/format etc.

    for (int r = 0; r < rows_.size(); ++r) {
        const Row& row = rows_[r];
        auto* combo = qobject_cast<QComboBox*>(table_->cellWidget(r, 5));
        if (!combo) continue;
        const Source s = static_cast<Source>(combo->currentData().toInt());
        QString chosenVal;
        switch (s) {
            case Source::Current: chosenVal = row.cur; break;
            case Source::AI:      chosenVal = row.ai; break;
            case Source::Google:  chosenVal = row.google; break;
            case Source::Amazon:  chosenVal = row.amazon; break;
        }
        // Only apply non-empty choice; otherwise keep current
        if (!chosenVal.trimmed().isEmpty()) setValueForKey(out, row.key, chosenVal.trimmed());
    }

    return out;
}
