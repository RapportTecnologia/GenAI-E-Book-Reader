#include "WelcomeDialog.h"
#include "ui/AboutDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

WelcomeDialog::WelcomeDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle("Bem-vindo ao GenAI E-Book Reader");

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *welcomeLabel = new QLabel(this);
    welcomeLabel->setTextFormat(Qt::RichText);
    welcomeLabel->setWordWrap(true);
    welcomeLabel->setText(
        "<p>Parabéns pela sua escolha! É muito importante para nós que você tenha escolhido o nosso software.</p>" \
        "<p>Esperamos que você se beneficie com ele, e que os seus resultados lhe gerem sucesso e aprendizado de qualidade. " \
        "Estamos muito felizes por ter você aqui!</p>" \
        "<p>Queremos te ouvir e saber o que gostaria de encontrar em nosso software. Para isso, envie um e-mail para " \
        "<a href='mailto:consultoria@carlosdelfino.com?subject=[GER]%20Dicas,%20Sugestões%20e%20Agradecimentos'>" \
        "consultoria@carlosdelfino.com</a> com o assunto <b>[GER] Dicas, Sugestões e Agradecimentos</b>. " \
        "Iremos buscar implementar suas sugestões o mais rápido possível.</p>"
    );

    layout->addWidget(welcomeLabel);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    QPushButton *aboutButton = buttonBox->addButton("Sobre", QDialogButtonBox::ActionRole);
    QPushButton *continueButton = buttonBox->addButton("Continuar", QDialogButtonBox::AcceptRole);

    connect(aboutButton, &QPushButton::clicked, this, &WelcomeDialog::on_aboutButton_clicked);
    connect(continueButton, &QPushButton::clicked, this, &QDialog::accept);

    layout->addWidget(buttonBox);

    setLayout(layout);
    setMinimumWidth(500);
}

WelcomeDialog::~WelcomeDialog()
{
}

void WelcomeDialog::on_aboutButton_clicked()
{
    AboutDialog aboutDialog(this);
    aboutDialog.exec();
}
