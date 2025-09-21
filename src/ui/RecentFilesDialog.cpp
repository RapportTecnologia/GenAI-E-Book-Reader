#include "RecentFilesDialog.h"

#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QVariantMap>
#include <QLineEdit>

RecentFilesDialog::RecentFilesDialog(const QVariantList& recentFiles, QWidget* parent)
    : QDialog(parent), recentFiles_(recentFiles) {
    setupUi();
    populateList();
    connect(openButton_, &QPushButton::clicked, this, &RecentFilesDialog::onOpenClicked);
    connect(removeButton_, &QPushButton::clicked, this, &RecentFilesDialog::onRemoveClicked);
    connect(closeButton_, &QPushButton::clicked, this, &RecentFilesDialog::reject);
    connect(listWidget_, &QListWidget::itemSelectionChanged, this, &RecentFilesDialog::onListSelectionChanged);
    connect(listWidget_, &QListWidget::itemDoubleClicked, this, &RecentFilesDialog::onOpenClicked);
    connect(filterLineEdit_, &QLineEdit::textChanged, this, &RecentFilesDialog::onFilterTextChanged);

    onListSelectionChanged(); // Initial state
}

QString RecentFilesDialog::selectedFile() const {
    if (listWidget_->currentItem()) {
        return listWidget_->currentItem()->data(Qt::UserRole).toString();
    }
    return QString();
}

void RecentFilesDialog::setupUi() {
    setWindowTitle(tr("Arquivos Recentes"));
    setMinimumSize(500, 300);

    filterLineEdit_ = new QLineEdit(this);
    filterLineEdit_->setPlaceholderText(tr("Pesquisar..."));

    listWidget_ = new QListWidget(this);

    openButton_ = new QPushButton(tr("Abrir"), this);
    removeButton_ = new QPushButton(tr("Remover da Lista"), this);
    closeButton_ = new QPushButton(tr("Fechar"), this);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(openButton_);
    buttonLayout->addWidget(removeButton_);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton_);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(filterLineEdit_);
    mainLayout->addWidget(listWidget_);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
}

void RecentFilesDialog::populateList() {
    listWidget_->clear();
    for (const QVariant& entry : recentFiles_) {
        QVariantMap map = entry.toMap();
        QString path = map.value("path").toString();
        if (QFileInfo::exists(path)) {
            QListWidgetItem* item = new QListWidgetItem(QFileInfo(path).fileName());
            item->setData(Qt::UserRole, path);
            item->setToolTip(path);
            listWidget_->addItem(item);
        }
    }
}

void RecentFilesDialog::onOpenClicked() {
    if (listWidget_->currentItem()) {
        emit openFileRequested(selectedFile());
        accept();
    }
}

void RecentFilesDialog::onRemoveClicked() {
    if (listWidget_->currentItem()) {
        QString fileToRemove = selectedFile();
        emit removeFileRequested(fileToRemove);
        // Remove from view
        delete listWidget_->takeItem(listWidget_->currentRow());
    }
}

void RecentFilesDialog::onFilterTextChanged(const QString& text) {
    const QStringList searchTerms = text.split(' ', Qt::SkipEmptyParts);
    for (int i = 0; i < listWidget_->count(); ++i) {
        QListWidgetItem* item = listWidget_->item(i);
        const QString fileName = item->text().toLower();
        bool match = true;
        for (const QString& term : searchTerms) {
            if (!fileName.contains(term.toLower())) {
                match = false;
                break;
            }
        }
        item->setHidden(!match);
    }
}

void RecentFilesDialog::onListSelectionChanged() {
    bool hasSelection = listWidget_->currentItem() != nullptr;
    openButton_->setEnabled(hasSelection);
    removeButton_->setEnabled(hasSelection);
}
