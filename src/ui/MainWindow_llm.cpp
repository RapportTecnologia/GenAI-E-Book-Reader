#include "ui/MainWindow.h"
#include "ai/LlmClient.h"
#include "ui/SummaryDialog.h"
#include "ui/ChatDock.h"

#include <QMessageBox>
#include <QStatusBar>
#include <QMetaObject>
#include <QFileDialog>
#include <QFile>
#include <QInputDialog>
#include <QBuffer>
#include <QByteArray>
#include <QImage>
#include <QIODevice>

void MainWindow::onRequestSynonyms(const QString& wordOrLocution) {
    if (!llm_) return;
    const auto choice = QMessageBox::question(this, tr("Confirmar"), tr("Enviar o trecho selecionado à IA para obter sinônimos?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (choice != QMessageBox::Yes) return;
    statusBar()->showMessage(tr("Consultando IA (sinônimos)..."));
    llm_->synonyms(wordOrLocution, QStringLiteral("pt-BR"), [this](QString out, QString err){
        QMetaObject::invokeMethod(this, [this, out, err](){
            if (!err.isEmpty()) {
                showLongAlert(tr("Erro na IA"), err);
                statusBar()->clearMessage();
                return;
            }

            if (summaryDlg_) {
                summaryDlg_->setWindowTitle(tr("Sinônimos"));
                summaryDlg_->setText(out);
                summaryDlg_->show();
                summaryDlg_->raise();
                summaryDlg_->activateWindow();
            }
            statusBar()->clearMessage();
        });
    });
}

void MainWindow::onRequestSummarize(const QString& text) {
    if (!llm_) return;
    const auto choice = QMessageBox::question(this, tr("Confirmar"), tr("Enviar o trecho selecionado à IA para gerar um resumo?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (choice != QMessageBox::Yes) return;
    statusBar()->showMessage(tr("Consultando IA (resumo)..."));
    llm_->summarize(text, [this](QString out, QString err){
        QMetaObject::invokeMethod(this, [this, out, err](){
            if (!err.isEmpty()) {
                QMessageBox::warning(this, tr("Erro na IA"), err);
                statusBar()->clearMessage();
                return;
            }
            if (summaryDlg_) {
                summaryDlg_->setWindowTitle(tr("Resumo"));
                summaryDlg_->setText(out);
                summaryDlg_->show();
                summaryDlg_->raise();
                summaryDlg_->activateWindow();
            }
            statusBar()->clearMessage();
        });
    });
}

void MainWindow::onRequestSendToChat(const QString& text) {
    if (!llm_) return;
    const auto choice = QMessageBox::question(this, tr("Confirmar"), tr("Enviar o trecho selecionado ao chat da IA?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (choice != QMessageBox::Yes) return;
    showChatPanel();
    if (chatDock_) chatDock_->appendUser(text);
    statusBar()->showMessage(tr("Enviando ao chat da IA..."));
    llm_->chat(text, [this](QString out, QString err){
        QMetaObject::invokeMethod(this, [this, out, err](){
            if (!err.isEmpty()) {
                showLongAlert(tr("Erro na IA"), err);
                statusBar()->clearMessage();
                return;
            }
            if (chatDock_) chatDock_->appendAssistant(out);
            statusBar()->clearMessage();
        });
    });
}

// showChatPanel() is implemented in MainWindow.cpp

void MainWindow::onChatSendMessage(const QString& text) {
    if (text.trimmed().isEmpty() || !llm_) return;
    if (chatDock_) chatDock_->appendUser(text);
    statusBar()->showMessage(tr("Enviando ao chat da IA..."));
    llm_->chat(text, [this](QString out, QString err){
        QMetaObject::invokeMethod(this, [this, out, err](){
            if (!err.isEmpty()) {
                showLongAlert(tr("Erro na IA"), err);
                statusBar()->clearMessage();
                return;
            }
            if (chatDock_) chatDock_->appendAssistant(out);
            statusBar()->clearMessage();
        });
    });
}

void MainWindow::onChatSaveTranscript(const QString& text) {
    const QString target = QFileDialog::getSaveFileName(this, tr("Salvar conversa"), QDir::homePath(), tr("Texto (*.txt);;Todos (*.*)"));
    if (target.isEmpty()) return;
    QFile f(target);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Erro"), tr("Não foi possível salvar."));
        return;
    }
    f.write(text.toUtf8());
    f.close();
    statusBar()->showMessage(tr("Conversa salva em %1").arg(target), 3000);
}

void MainWindow::onChatSummarizeTranscript(const QString& text) {
    if (!llm_) return;
    statusBar()->showMessage(tr("Solicitando compilado do diálogo..."));
    // Use summarize() com todo o transcript
    llm_->summarize(text, [this](QString out, QString err){
        QMetaObject::invokeMethod(this, [this, out, err](){
            if (!err.isEmpty()) {
                showLongAlert(tr("Erro na IA"), err);
                statusBar()->clearMessage();
                return;
            }
            showChatPanel();
            if (chatDock_) chatDock_->appendAssistant(out);
            statusBar()->clearMessage();
        });
    });
}

static QString mwllm_toDataUrlPng(const QImage& img) {
    QByteArray bytes;
    QBuffer buf(&bytes);
    buf.open(QIODevice::WriteOnly);
    img.save(&buf, "PNG");
    return QString::fromLatin1("data:image/png;base64,%1").arg(QString::fromLatin1(bytes.toBase64()));
}

void MainWindow::onRequestSendImageToChat(const QImage& image) {
    if (!llm_ || image.isNull()) return;
    showChatPanel();
    if (chatDock_) chatDock_->setPendingImage(image);
    const QString edited = QInputDialog::getMultiLineText(
        this,
        tr("Enviar imagem ao chat"),
        tr("Prompt para a IA:"),
        tr("Descreva a imagem em pt-BR, detalhando elementos, relações e possíveis interpretações, sem inventar.")
    ).trimmed();
    if (edited.isEmpty()) { if (chatDock_) chatDock_->clearPendingImage(); return; }
    if (chatDock_) {
        chatDock_->appendUserImage(image);
        chatDock_->clearPendingImage();
    }
    statusBar()->showMessage(tr("Enviando imagem ao chat da IA..."));
    const QString dataUrl = mwllm_toDataUrlPng(image);
    llm_->chatWithImage(edited, dataUrl, [this](QString out, QString err){
        QMetaObject::invokeMethod(this, [this, out, err]() {
            if (!err.isEmpty()) {
                showLongAlert(tr("Erro na IA"), err);
                statusBar()->clearMessage();
                return;
            }
            if (chatDock_) chatDock_->appendAssistant(out);
            statusBar()->clearMessage();
            saveChatForCurrentFile();
        });
    });
}
