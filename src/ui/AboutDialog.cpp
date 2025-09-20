#include "ui/AboutDialog.h"
#include "app/App.h"

#include <QApplication>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QDialogButtonBox>
#include <QStringList>
#include <QFile>
#include <QTextStream>

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent) {
    setupUi();
}

void AboutDialog::setupUi() {
    setWindowTitle(QString("Sobre %1").arg(genai::AppInfo::Name));
    setMinimumSize(600, 500);

    auto *layout = new QVBoxLayout(this);

    auto *nameLabel = new QLabel(QString("<b>%1 v%2</b>").arg(genai::AppInfo::Name).arg(genai::AppInfo::Version));
    nameLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(nameLabel);

    auto *descriptionLabel = new QLabel(genai::AppInfo::Description);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(descriptionLabel);

    // License Area
    auto *licenseLabel = new QLabel("<b>Licença:</b>");
    layout->addWidget(licenseLabel);
    auto *licenseArea = new QScrollArea();
    licenseArea->setWidgetResizable(true);
    auto *licenseText = new QTextEdit();
    licenseText->setReadOnly(true);
    
        QFile licenseFile(":/app/LICENSE");
    if (licenseFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&licenseFile);
        licenseText->setText(in.readAll());
        licenseFile.close();
    }

    licenseArea->setWidget(licenseText);
    layout->addWidget(licenseArea);

    // Authors Area
    auto *authorsLabel = new QLabel("<b>Desenvolvedores:</b>");
    layout->addWidget(authorsLabel);
    auto *authorsArea = new QScrollArea();
    authorsArea->setWidgetResizable(true);
    auto *authorsText = new QTextEdit();
    authorsText->setReadOnly(true);
    QStringList developers;
    for (const auto& dev : genai::Developers) {
        developers << QString("%1 - <a href=\"mailto:%2\">%2</a> - <a href=\"%3\">%3</a>").arg(dev.name, dev.email, dev.site);
    }
    authorsText->setHtml(developers.join("<br>"));
    authorsArea->setWidget(authorsText);
    layout->addWidget(authorsArea);

    // Close button
    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    layout->addWidget(buttonBox);

    setLayout(layout);
}
