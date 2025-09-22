#include "ui/MainWindow.h"
#include "ai/LlmClient.h"
#include "ui/SummaryDialog.h"
#include "ui/ChatDock.h"
#include "ui/PdfViewerWidget.h"

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
#include <QPdfDocument>
#include <QFileInfo>
#include <QSettings>
#include <QRegularExpression>

void MainWindow::onRequestSummarizeDocument() {
    if (!llm_) return;
    if (currentFilePath_.isEmpty()) return;
    const auto choice = QMessageBox::question(
        this,
        tr("Resumo do e-book"),
        tr("Deseja enviar o e-book para a IA gerar um resumo geral?\n\nDica: os embeddings locais deste documento serão utilizados como Base de Conhecimento para buscas e conversas futuras (RAG)."),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes
    );
    if (choice != QMessageBox::Yes) return;

    // 1) Garantir que há índice de embeddings disponível para este arquivo (KB local)
    IndexPaths paths;
    const bool haveIndex = getIndexPaths(&paths);
    if (!haveIndex) {
        const auto build = QMessageBox::question(
            this,
            tr("Base de Conhecimento"),
            tr("Este e-book ainda não possui embeddings gerados.\nDeseja gerar agora para habilitar a Base de Conhecimento?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes
        );
        if (build == QMessageBox::Yes) {
            onRequestRebuildEmbeddings();
        }
    }

    statusBar()->showMessage(tr("Gerando resumo do e-book via IA..."));

    // 2) Construir um contexto enxuto, amostrando páginas de todo o documento
    QString context;
    auto* pv = qobject_cast<PdfViewerWidget*>(viewer_);
    if (pv && pv->document()) {
        // Carregar cache de texto de páginas (usa pdftotext/OCR quando disponível)
        ensurePagesTextLoaded();
        const int pageCount = pv->document()->pageCount();
        if (pageCount > 0 && !pagesText_.isEmpty()) {
            const int targetSamples = 12; // amostrar ~12 trechos distribuídos
            const int step = qMax(1, pageCount / targetSamples);
            int taken = 0;
            for (int p = 1; p <= pageCount && taken < targetSamples; p += step) {
                const int idx = p - 1;
                QString snippet;
                if (idx >= 0 && idx < pagesText_.size()) {
                    snippet = pagesText_.at(idx);
                    // Normalização: colapsar espaços e limitar tamanho por trecho
                    snippet.replace(QRegularExpression("\\s+"), " ");
                    if (snippet.size() > 800) snippet = snippet.left(800);
                }
                if (!snippet.trimmed().isEmpty()) {
                    context += tr("[Página %1]\n%2\n\n").arg(p).arg(snippet.trimmed());
                    ++taken;
                }
            }
        }
    }
    if (context.trimmed().isEmpty()) {
        // Fallback simples
        context = tr("Conteúdo não extraído. Forneça um resumo geral do e-book considerando que o texto completo não está disponível.");
    }

    // 3) Enviar para a IA montar um resumo executivo do e-book
    QList<QPair<QString,QString>> msgs;
    const QString sys = tr("Você é um assistente que cria um resumo executivo e estruturado de um e-book em português do Brasil.\n"
                           "Quando possível, cite páginas indicativas (entre colchetes) a partir dos trechos fornecidos.\n"
                           "Não invente fatos não suportados.");
    msgs.append({QStringLiteral("system"), sys});
    const QString user = tr("Gere um resumo do e-book atual com tópicos principais, objetivos, público, conceitos-chave e conclusões.\n\nTrechos amostrados do documento:\n%1").arg(context);
    msgs.append({QStringLiteral("user"), user});

    llm_->chatWithMessages(msgs, [this](QString out, QString err){
        QMetaObject::invokeMethod(this, [this, out, err](){
            if (!err.isEmpty()) {
                showLongAlert(tr("Erro na IA"), err);
                statusBar()->clearMessage();
                return;
            }
            if (summaryDlg_) {
                summaryDlg_->setWindowTitle(tr("Resumo do e-book"));
                summaryDlg_->setText(out);
                summaryDlg_->show();
                summaryDlg_->raise();
                summaryDlg_->activateWindow();
            }
            statusBar()->clearMessage();
            saveChatForCurrentFile();
        });
    });
}

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
    // Send full conversation for continuous context
    const auto msgs = chatDock_ ? chatDock_->conversationForLlm() : QList<QPair<QString, QString>>{};
    llm_->chatWithMessages(msgs, [this](QString out, QString err){
        QMetaObject::invokeMethod(this, [this, out, err](){
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

// showChatPanel() is implemented in MainWindow.cpp

void MainWindow::onChatSendMessage(const QString& text) {
    if (text.trimmed().isEmpty() || !llm_) return;
    if (chatDock_) chatDock_->appendUser(text);
    statusBar()->showMessage(tr("Enviando ao chat da IA..."));
    const auto msgs = chatDock_ ? chatDock_->conversationForLlm() : QList<QPair<QString, QString>>{};
    llm_->chatWithMessages(msgs, [this](QString out, QString err){
        QMetaObject::invokeMethod(this, [this, out, err](){
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
