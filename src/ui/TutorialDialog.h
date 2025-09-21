#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
class QWebEngineView;
QT_END_NAMESPACE

/**
 * @brief Diálogo que exibe o conteúdo do tutorial do aplicativo.
 *
 * Carrega o arquivo TUTORIAL.md dos recursos, converte de Markdown para HTML
 * e o exibe em um QWebEngineView para uma apresentação rica.
 */
class TutorialDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constrói o diálogo do tutorial.
     * @param parent O widget pai.
     */
    explicit TutorialDialog(QWidget *parent = nullptr);
    ~TutorialDialog();

private:
    /**
     * @brief Carrega e renderiza o conteúdo do tutorial.
     */
    void loadTutorialContent();

    QWebEngineView *view_;
};
