#include "ui/MainWindow.h"
#include "ai/LlmClient.h"
#include "ui/SummaryDialog.h"
#include "ui/ChatDock.h"
#include "ui/PdfViewerWidget.h"
#include "ui/ViewerWidget.h"
#include "ui/OpfStore.h"

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
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

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
    const QString respLang = QSettings().value("ai/response_language", QStringLiteral("pt-BR")).toString();
    const QString sys = tr("Você é um assistente que cria um resumo executivo e estruturado de um e-book em %1.\n"
                           "Quando possível, cite páginas indicativas (entre colchetes) a partir dos trechos fornecidos.\n"
                           "Não invente fatos não suportados.").arg(respLang);
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

// ---- LLM Function Calling dispatcher and tools ----
void MainWindow::handleLlmToolCalls(const QJsonArray& toolCalls) {
    for (const auto& item : toolCalls) {
        const QJsonObject tc = item.toObject();
        const QJsonObject fn = tc.value("function").toObject();
        const QString name = fn.value("name").toString();
        const QString argsStr = fn.value("arguments").toString();
        QJsonObject args;
        if (!argsStr.isEmpty()) {
            const auto parsed = QJsonDocument::fromJson(argsStr.toUtf8());
            if (parsed.isObject()) args = parsed.object();
        }
        if (name == QLatin1String("propose_search")) {
            const QString q = args.value("query").toString();
            toolProposeSearch(q);
        } else if (name == QLatin1String("search_next")) {
            toolNextResult();
        } else if (name == QLatin1String("search_prev")) {
            toolPrevResult();
        } else if (name == QLatin1String("goto_page")) {
            const int page = args.value("page").toInt();
            toolGotoPage(page);
        } else if (name == QLatin1String("retrieve_passages")) {
            // Return additional snippets for requested pages to the chat
            QJsonArray arr = args.value("pages").toArray();
            if (arr.isEmpty()) continue;
            if (!ensurePagesTextLoaded()) continue;
            QStringList blocks;
            for (const auto& v : arr) {
                const int p = v.toInt();
                if (p <= 0) continue;
                const int idx = p - 1;
                if (idx < 0 || idx >= pagesText_.size()) continue;
                QString t = pagesText_.at(idx);
                t.replace(QRegularExpression("\\s+"), " ");
                if (t.size() > 1000) t = t.left(1000);
                const QString block = tr("[Página %1]\n%2").arg(p).arg(t.trimmed());
                blocks << block;
            }
            if (!blocks.isEmpty() && chatDock_) {
                chatDock_->appendAssistant(blocks.join("\n\n"));
            }
        } else if (name == QLatin1String("suggest_references")) {
            // The model may produce references itself; we can surface the topic intent in chat
            const QString topic = args.value("topic").toString();
            const int maxItems = args.value("max_items").toInt();
            if (chatDock_) {
                if (!topic.trimmed().isEmpty()) {
                    chatDock_->appendAssistant(tr("[referências] Tópico: %1%2").arg(topic).arg(maxItems > 0 ? tr(" (máx. %1 itens)").arg(maxItems) : QString()));
                } else {
                    chatDock_->appendAssistant(tr("[referências] Gerando sugestões adicionais de fontes e leituras."));
                }
            }
        } else if (name == QLatin1String("query_opf")) {
            // Read OPF for current document and append a concise one-line identification
            QString opfPath;
            if (!currentFilePath_.isEmpty()) {
                opfPath = OpfStore::defaultOpfPathFor(currentFilePath_);
            }
            if (opfPath.isEmpty()) continue; // silent: do not disrupt chat
            OpfData d; QString err;
            if (!OpfStore::read(opfPath, &d, &err)) continue; // silent on error
            const QString title = d.title.trimmed();
            const QString author = d.author.trimmed();
            const QString isbn = d.isbn.trimmed();
            QString line = tr("[OPF] %1%2%3")
                               .arg(!title.isEmpty() ? tr("Título: %1").arg(title) : QString())
                               .arg(!author.isEmpty() ? tr(", Autor: %1").arg(author) : QString())
                               .arg(!isbn.isEmpty() ? tr(", ISBN: %1").arg(isbn) : QString());
            if (line.trimmed().isEmpty()) continue;
            if (chatDock_) chatDock_->appendAssistant(line);
        }
    }
}

void MainWindow::toolProposeSearch(const QString& query) {
    if (!searchEdit_) return;
    const QString qnorm = query.trimmed();
    if (qnorm.isEmpty()) return;
    searchEdit_->setText(qnorm);
    // Trigger search to populate results, then move to the first (equiv. to Próximo)
    onSearchTriggered();
}

void MainWindow::toolNextResult() {
    onSearchNext();
}

void MainWindow::toolPrevResult() {
    onSearchPrev();
}

void MainWindow::toolGotoPage(int page) {
    if (page <= 0) return;
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
        pv->setCurrentPage(static_cast<unsigned int>(page));
        pv->flashHighlight();
        updateStatus();
    }
}

void MainWindow::onRequestSynonyms(const QString& wordOrLocution) {
    if (!llm_) return;
    const auto choice = QMessageBox::question(this, tr("Confirmar"), tr("Enviar o trecho selecionado à IA para obter sinônimos?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (choice != QMessageBox::Yes) return;
    statusBar()->showMessage(tr("Consultando IA (sinônimos)..."));
    {
        QSettings s; const QString respLang = s.value("ai/response_language", QStringLiteral("pt-BR")).toString();
        llm_->synonyms(wordOrLocution, respLang, [this](QString out, QString err){
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
    if (chatDock_) chatDock_->setBusy(true);
    // Send full conversation for continuous context
    auto msgs = chatDock_ ? chatDock_->conversationForLlm() : QList<QPair<QString, QString>>{};
    // Prepend system metadata of the current e-book (title, author, description, summary)
    const QString sysOpf = buildOpfSystemPrompt();
    if (!sysOpf.trimmed().isEmpty()) {
        msgs.prepend({QStringLiteral("system"), sysOpf});
    }
    // Precision policy: minimal metadata, never fabricate
    const QString sysPolicy = tr(
        "Política de precisão: use apenas os metadados mínimos fornecidos e o que estiver no histórico. "
        "Se uma informação não estiver disponível (por exemplo, autor), responda explicitamente que é desconhecida/não disponível. "
        "Não invente dados.");
    msgs.prepend({QStringLiteral("system"), sysPolicy});
    llm_->chatWithMessages(msgs, [this](QString out, QString err){
        QMetaObject::invokeMethod(this, [this, out, err](){
            if (!err.isEmpty()) {
                showLongAlert(tr("Erro na IA"), err);
                statusBar()->clearMessage();
                if (chatDock_) chatDock_->setBusy(false);
                return;
            }
            if (chatDock_) chatDock_->appendAssistant(out);
            statusBar()->clearMessage();
            saveChatForCurrentFile();
            if (chatDock_) chatDock_->setBusy(false);
        });
    });
}

// showChatPanel() is implemented in MainWindow.cpp

void MainWindow::onChatSendMessage(const QString& text) {
    if (text.trimmed().isEmpty() || !llm_) return;
    // If it's a slash command, keep legacy tool-based flow
    if (text.startsWith('/')) {
        if (chatDock_) chatDock_->appendUser(text);
        statusBar()->showMessage(tr("Enviando ao chat da IA..."));
        if (chatDock_) chatDock_->setBusy(true);
        auto msgs = chatDock_ ? chatDock_->conversationForLlm() : QList<QPair<QString, QString>>{};
        const QString sysOpf = buildOpfSystemPrompt();
        if (!sysOpf.trimmed().isEmpty()) { msgs.prepend({QStringLiteral("system"), sysOpf}); }
        QSettings s; const QString respLang = s.value("ai/response_language", QStringLiteral("pt-BR")).toString();
        QJsonArray tools;
        {
            QJsonObject tool; tool["type"] = "function";
            QJsonObject fn; fn["name"] = "propose_search";
            QJsonObject params; params["type"] = "object";
            QJsonObject props; QJsonObject q; q["type"] = "string"; q["description"] = tr("Reescreva/normalize a consulta a ser pesquisada no documento (%1)").arg(respLang);
            props["query"] = q; params["properties"] = props; QJsonArray req; req.append("query"); params["required"] = req;
            fn["parameters"] = params; tool["function"] = fn; tools.append(tool);
        }
        { QJsonObject tool; tool["type"] = "function"; QJsonObject fn; fn["name"] = "search_next"; QJsonObject params; params["type"] = "object"; params["properties"] = QJsonObject(); fn["parameters"] = params; tool["function"] = fn; tools.append(tool); }
        { QJsonObject tool; tool["type"] = "function"; QJsonObject fn; fn["name"] = "search_prev"; QJsonObject params; params["type"] = "object"; params["properties"] = QJsonObject(); fn["parameters"] = params; tool["function"] = fn; tools.append(tool); }
        {
            QJsonObject tool; tool["type"] = "function";
            QJsonObject fn; fn["name"] = "goto_page";
            QJsonObject params; params["type"] = "object";
            QJsonObject props; QJsonObject p; p["type"] = "integer"; p["minimum"] = 1; p["description"] = tr("Número da página (1-based)"); props["page"] = p;
            params["properties"] = props; QJsonArray req; req.append("page"); params["required"] = req;
            fn["parameters"] = params; tool["function"] = fn; tools.append(tool);
        }
        llm_->chatWithMessagesTools(msgs, tools, [this](QString out, QJsonArray toolCalls, QString err){
            QMetaObject::invokeMethod(this, [this, out, toolCalls, err](){
                if (!err.isEmpty()) { showLongAlert(tr("Erro na IA"), err); statusBar()->clearMessage(); return; }
                if (!toolCalls.isEmpty()) { handleLlmToolCalls(toolCalls); }
                if (!out.trimmed().isEmpty() && chatDock_) chatDock_->appendAssistant(out);
                statusBar()->clearMessage();
                saveChatForCurrentFile();
                if (chatDock_) chatDock_->setBusy(false);
            });
        });
        return;
    }

    // Default: use RAG to select page and answer grounded on the book
    if (chatDock_) chatDock_->appendUser(text);
    statusBar()->showMessage(tr("Respondendo com base no livro (RAG)..."));
    if (chatDock_) chatDock_->setBusy(true);
    answerQuestionWithRag(text);
}

QString MainWindow::buildOpfSystemPrompt() const {
    if (currentFilePath_.isEmpty()) return QString();
    const QString opfPath = OpfStore::defaultOpfPathFor(currentFilePath_);
    if (opfPath.isEmpty()) return QString();
    OpfData d; QString err;
    bool haveOpf = OpfStore::read(opfPath, &d, &err);
    // Fallback: if OPF missing or largely empty, try to infer from PDF metadata
    if (!haveOpf || (d.title.trimmed().isEmpty() && d.author.trimmed().isEmpty())) {
        OpfData inferred = buildOpfFromPdfMeta(currentFilePath_);
        // Only fill missing fields to avoid overwriting present OPF data
        if (d.title.trimmed().isEmpty()) d.title = inferred.title;
        if (d.author.trimmed().isEmpty()) d.author = inferred.author;
        if (d.isbn.trimmed().isEmpty()) d.isbn = inferred.isbn;
    }

    QStringList lines;
    lines << tr("Arquivo: %1").arg(QFileInfo(currentFilePath_).fileName());
    if (!d.title.trimmed().isEmpty()) lines << tr("Título: %1").arg(d.title);
    if (!d.author.trimmed().isEmpty()) lines << tr("Autor: %1").arg(d.author);
    if (!d.isbn.trimmed().isEmpty()) lines << tr("ISBN: %1").arg(d.isbn);

    // If still only filename present, avoid adding an empty system prompt
    const int nonTrivial = lines.size();
    if (nonTrivial <= 1) return QString();

    const QString header = tr("Metadados mínimos de identificação do e-book atual. Use apenas para contextualizar a conversa, sem inferências adicionais:");
    return header + "\n" + lines.join("\n");
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
        tr("Descreva a imagem em %1, detalhando elementos, relações e possíveis interpretações, sem inventar.").arg(QSettings().value("ai/response_language", QStringLiteral("pt-BR")).toString())
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
