#pragma once

#include <QDialog>
#include <QVariantList>

class QListWidget;
class QPushButton;
class QLineEdit;

class RecentFilesDialog : public QDialog {
    Q_OBJECT

public:
    explicit RecentFilesDialog(const QVariantList& recentFiles, QWidget* parent = nullptr);
    QString selectedFile() const;

signals:
    void openFileRequested(const QString& filePath);
    void removeFileRequested(const QString& filePath);

private slots:
    void onOpenClicked();
    void onRemoveClicked();
    void onListSelectionChanged();
    void onFilterTextChanged(const QString& text);

private:
    void setupUi();
    void populateList();

    QLineEdit* filterLineEdit_;
    QListWidget* listWidget_;
    QPushButton* openButton_;
    QPushButton* removeButton_;
    QPushButton* closeButton_;

    QVariantList recentFiles_;
};
