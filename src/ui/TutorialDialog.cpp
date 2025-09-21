#include "ui/TutorialDialog.h"
#include <QVBoxLayout>
#include <QWebEngineView>
#include <QFile>
#include <QTextStream>
#include <QDialogButtonBox>
#include "cmark.h"

TutorialDialog::TutorialDialog(QWidget *parent)
    : QDialog(parent)
    , view_(new QWebEngineView(this))
{
    setWindowTitle(tr("Tutorial do Aplicativo"));
    setMinimumSize(800, 600);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(view_);

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &TutorialDialog::reject);
    layout->addWidget(buttonBox);

    loadTutorialContent();
}

TutorialDialog::~TutorialDialog()
{
    // O view_ é filho do diálogo, então será deletado automaticamente.
}

void TutorialDialog::loadTutorialContent()
{
    QFile tutorialFile(":/docs/TUTORIAL.md");
    if (!tutorialFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        view_->setHtml(tr("<h1>Erro</h1><p>Não foi possível carregar o arquivo do tutorial.</p>"));
        return;
    }

    QTextStream in(&tutorialFile);
    const QString markdownContent = in.readAll();
    tutorialFile.close();

    // Converter Markdown para HTML usando cmark
    char *html = cmark_markdown_to_html(markdownContent.toUtf8().constData(), markdownContent.toUtf8().length(), CMARK_OPT_DEFAULT);
    if (html) {
        view_->setHtml(QString::fromUtf8(html));
        free(html);
    } else {
        view_->setHtml(tr("<h1>Erro</h1><p>Falha ao converter o tutorial de Markdown para HTML.</p>"));
    }
}
