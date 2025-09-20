#include "ui/MainWindow.h"
#include "ui/ViewerWidget.h"
#include "ui/PdfViewerWidget.h"
#include "ai/LlmClient.h"
#include "ui/SummaryDialog.h"
#include "ui/LlmSettingsDialog.h"
#include "ui/EmbeddingSettingsDialog.h"
#include "ui/DictionarySettingsDialog.h"
#include "ui/ChatDock.h"
#include "ui/AboutDialog.h"
#include <QApplication>
#include <QCoreApplication>
#include <QAction>
#include <QToolBar>
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include <QTreeWidget>
#include <QSplitter>
#include <QVBoxLayout>
#include <QActionGroup>
#include <QSize>
#include <QFileDialog>
#include <QComboBox>
#include <QDir>
#include <QPalette>
#include <QMessageBox>
#include <QFileInfo>
#include <QFile>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QMap>
#include <QSizePolicy>
#include <QStyle>
#include <QInputDialog>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProgressDialog>
#include <QProgressBar>
#include <QTimer>
#include <QProcess>
#include <QThread>
#include <QShortcut>
#include <QStandardPaths>
#include <QToolButton>
#include <QMenu>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QWidgetAction>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <algorithm>
#include <functional>
#include <QModelIndex>
#include <QVariant>
#include <QPushButton>
#include <QSettings>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <QDesktopServices>
#include <QClipboard>
#include <QGuiApplication>

#include "app/App.h"
#include "ai/EmbeddingIndexer.h"
#include "ai/EmbeddingProvider.h"
#include "ai/VectorIndex.h"

#include <QPdfDocument>
#include <QPdfBookmarkModel>
#include <QCryptographicHash>

namespace {
// Load recent entries as a list of QVariantMap with keys:
// path, title, author, publisher, isbn, summary, keywords
QVariantList loadRecentEntries(QSettings& settings) {
    QVariantList entries = settings.value("recent/entries").toList();
    if (!entries.isEmpty()) return entries;
    // Backwards compatibility: migrate from recent/files (QStringList)
    const QStringList files = settings.value("recent/files").toStringList();
    if (files.isEmpty()) return entries;
    for (const QString& p : files) {
        QVariantMap m; m["path"] = p; m["title"] = QFileInfo(p).completeBaseName();
        m["author"] = QString(); m["publisher"] = QString(); m["isbn"] = QString();
        m["summary"] = QString(); m["keywords"] = QString();
        entries.append(m);
    }
    settings.setValue("recent/entries", entries);
    return entries;
}

} // end anonymous namespace

void MainWindow::showAboutDialog() {
    AboutDialog dlg(this);
    dlg.exec();
}

void MainWindow::updateTitleWidget() {
    if (!titleButton_) return;
    QString label = tr("Leitor");
    QString tip;
    if (!currentFilePath_.isEmpty()) {
        // Prefer PDF metadata title if available, else filename
        QString title = QFileInfo(currentFilePath_).completeBaseName();
        if (QFileInfo(currentFilePath_).suffix().compare("pdf", Qt::CaseInsensitive) == 0) {
            QPdfDocument doc;
            if (doc.load(currentFilePath_) == static_cast<QPdfDocument::Error>(0)) {
                const auto t = doc.metaData(QPdfDocument::MetaDataField::Title).toString().trimmed();
                if (!t.isEmpty()) title = t;
            }
        }
        label = title;
        tip = QFileInfo(currentFilePath_).absoluteFilePath();
    }
    titleButton_->setText(label);
    titleButton_->setToolTip(tip);
}

void MainWindow::showCurrentPathInfo() {
    if (currentFilePath_.isEmpty()) return;
    const QString abs = QFileInfo(currentFilePath_).absoluteFilePath();
    QMessageBox msg(this);
    msg.setIcon(QMessageBox::Information);
    msg.setWindowTitle(tr("Arquivo atual"));
    msg.setText(tr("Caminho completo do arquivo:"));
    msg.setInformativeText(abs);
    QPushButton* copyBtn = msg.addButton(tr("Copiar"), QMessageBox::ActionRole);
    msg.addButton(QMessageBox::Ok);
    msg.exec();
    if (msg.clickedButton() == copyBtn) {
        QGuiApplication::clipboard()->setText(abs);
        statusBar()->showMessage(tr("Caminho copiado para a área de transferência."), 2000);
    }
}

void MainWindow::onTitleOpenDir() {
    if (currentFilePath_.isEmpty()) return;
    const QString dir = QFileInfo(currentFilePath_).absolutePath();
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

bool MainWindow::migrateEmbeddingsForPathChange(const QString& oldPath, const QString& newPath, QString* errorMsg) {
    IndexPaths oldIdx, newIdx;
    if (!computeIndexPathsFor(oldPath, &oldIdx)) { if (errorMsg) *errorMsg = tr("Falha ao calcular índice antigo."); return false; }
    if (!computeIndexPathsFor(newPath, &newIdx)) { if (errorMsg) *errorMsg = tr("Falha ao calcular índice novo."); return false; }
    // If old files don't exist, nothing to do
    const QStringList oldFiles{ oldIdx.binPath, oldIdx.idsPath, oldIdx.metaPath };
    bool anyExist = std::any_of(oldFiles.begin(), oldFiles.end(), [](const QString& p){ return QFileInfo::exists(p); });
    if (!anyExist) return true;
    // Ensure target dir exists
    QDir().mkpath(QFileInfo(newIdx.base).absolutePath());
    auto moveFile = [&](const QString& src, const QString& dst)->bool{
        if (!QFileInfo::exists(src)) return true; // nothing to move
        if (QFileInfo::exists(dst)) QFile::remove(dst);
        return QFile::rename(src, dst);
    };
    if (!moveFile(oldIdx.binPath, newIdx.binPath)) { if (errorMsg) *errorMsg = tr("Não foi possível mover %1 para %2").arg(oldIdx.binPath, newIdx.binPath); return false; }
    if (!moveFile(oldIdx.idsPath, newIdx.idsPath)) { if (errorMsg) *errorMsg = tr("Não foi possível mover %1 para %2").arg(oldIdx.idsPath, newIdx.idsPath); return false; }
    if (!moveFile(oldIdx.metaPath, newIdx.metaPath)) { if (errorMsg) *errorMsg = tr("Não foi possível mover %1 para %2").arg(oldIdx.metaPath, newIdx.metaPath); return false; }
    return true;
}

void MainWindow::onTitleAddToCalibre() {
    if (currentFilePath_.isEmpty()) return;
    // Ask for Calibre library path
    const QString lastLib = settings_.value("calibre/library").toString();
    QString lib = QFileDialog::getExistingDirectory(this, tr("Selecione a biblioteca do Calibre"), lastLib.isEmpty()? QDir::homePath() : lastLib);
    if (lib.isEmpty()) return;
    settings_.setValue("calibre/library", lib);

    // Run calibredb add --with-library <lib> --copy <file>
    QProcess proc;
    QStringList args;
    args << "--with-library" << lib << "add" << "--copy" << currentFilePath_;
    proc.start("calibredb", args);
    if (!proc.waitForStarted(3000)) {
        QMessageBox::warning(this, tr("Calibre"), tr("Não foi possível iniciar 'calibredb'. Verifique a instalação."));
        return;
    }
    proc.waitForFinished(-1);
    const QString out = QString::fromUtf8(proc.readAllStandardOutput());
    const QString err = QString::fromUtf8(proc.readAllStandardError());
    if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
        showLongAlert(tr("Falha ao adicionar ao Calibre"), out + "\n" + err);
        return;
    }

    // Ask user to select the new managed file path (Calibre organizes by Author/Title)
    QMessageBox::information(this, tr("Calibre"), tr("Arquivo adicionado ao Calibre. Selecione o novo caminho do arquivo\nna biblioteca para migrar os embeddings."));
    const QString newPath = QFileDialog::getOpenFileName(this, tr("Selecione o arquivo gerenciado pelo Calibre"), lib);
    if (newPath.isEmpty()) return;
    QString emsg;
    if (!migrateEmbeddingsForPathChange(currentFilePath_, newPath, &emsg)) {
        showLongAlert(tr("Migração de embeddings"), tr("Falhou: %1").arg(emsg));
        return;
    }
    // Open the new path in the reader
    openPath(newPath);
}

void MainWindow::onTitleRenameFile() {
    if (currentFilePath_.isEmpty()) return;
    const QFileInfo fi(currentFilePath_);
    bool ok = false;
    const QString suggestion = fi.completeBaseName();
    const QString newBase = QInputDialog::getText(this, tr("Renomear e-book"), tr("Nome atual: %1\nNovo nome (sem extensão):").arg(fi.fileName()), QLineEdit::Normal, suggestion, &ok);
    if (!ok || newBase.trimmed().isEmpty()) return;
    const QString newPath = fi.dir().filePath(newBase + "." + fi.suffix());
    if (QFileInfo::exists(newPath)) {
        QMessageBox::warning(this, tr("Renomear"), tr("Já existe um arquivo com este nome."));
        return;
    }
    if (!QFile::rename(currentFilePath_, newPath)) {
        QMessageBox::warning(this, tr("Renomear"), tr("Falha ao renomear o arquivo."));
        return;
    }
    QString emsg;
    if (!migrateEmbeddingsForPathChange(currentFilePath_, newPath, &emsg)) {
        showLongAlert(tr("Migração de embeddings"), tr("Falhou: %1").arg(emsg));
    }
    openPath(newPath);
}

void MainWindow::focusSearchBar() {
    if (!searchEdit_) return;
    searchEdit_->setFocus();
    searchEdit_->selectAll();
}

QString MainWindow::sha1(const QString& s) const {
    QCryptographicHash h(QCryptographicHash::Sha1);
    h.addData(s.toUtf8());
    return QString::fromLatin1(h.result().toHex());
}

bool MainWindow::getIndexPaths(IndexPaths* out) const {
    if (!out) return false;
    out->base.clear(); out->binPath.clear(); out->idsPath.clear(); out->metaPath.clear();
    if (currentFilePath_.isEmpty()) return false;
    QSettings s;
    const QString dbPath = s.value("emb/db_path", QDir(QDir::home().filePath(".cache")).filePath("br.tec.rapport.genai-reader")).toString();
    const QString model = s.value("emb/model", "nomic-embed-text:latest").toString();
    const QString fileHash = sha1(QFileInfo(currentFilePath_).absoluteFilePath());
    QString fileBaseName = QStringLiteral("index_%1_%2").arg(fileHash, model);
    fileBaseName.replace(':', '_');
    fileBaseName.replace('/', '_');
    out->base = QDir(dbPath).filePath(fileBaseName);
    out->binPath = out->base + ".bin";
    out->idsPath = out->base + ".ids.json";
    out->metaPath = out->base + ".meta.json";
    return QFileInfo::exists(out->binPath) && QFileInfo::exists(out->idsPath) && QFileInfo::exists(out->metaPath);
}

bool MainWindow::computeIndexPathsFor(const QString& filePath, IndexPaths* out) const {
    if (!out) return false;
    out->base.clear(); out->binPath.clear(); out->idsPath.clear(); out->metaPath.clear();
    if (filePath.isEmpty()) return false;
    QSettings s;
    const QString dbPath = s.value("emb/db_path", QDir(QDir::home().filePath(".cache")).filePath("br.tec.rapport.genai-reader")).toString();
    const QString model = s.value("emb/model", "nomic-embed-text:latest").toString();
    const QString fileHash = sha1(QFileInfo(filePath).absoluteFilePath());
    QString fileBaseName = QStringLiteral("index_%1_%2").arg(fileHash, model);
    fileBaseName.replace(':', '_');
    fileBaseName.replace('/', '_');
    out->base = QDir(dbPath).filePath(fileBaseName);
    out->binPath = out->base + ".bin";
    out->idsPath = out->base + ".ids.json";
    out->metaPath = out->base + ".meta.json";
    return true;
}

bool MainWindow::ensurePagesTextLoaded() {
    if (pagesTextLoaded_) return true;
    pagesText_.clear();
    // Only for PDFs in this build
    auto pv = qobject_cast<PdfViewerWidget*>(viewer_);
    if (!pv || !pv->document()) return false;
    QPdfDocument* doc = pv->document();
    const int pageCount = doc->pageCount();
    if (pageCount <= 0) return false;
    const bool hasPdfToText = !QStandardPaths::findExecutable("pdftotext").isEmpty();
    for (int i = 1; i <= pageCount; ++i) {
        QString pageText;
        if (hasPdfToText) {
            QProcess proc;
            QStringList args; args << "-q" << "-layout" << "-eol" << "unix" << "-f" << QString::number(i) << "-l" << QString::number(i) << currentFilePath_ << "-";
            proc.start("pdftotext", args);
            if (proc.waitForStarted(2000)) {
                proc.waitForFinished(20000);
                pageText = QString::fromUtf8(proc.readAllStandardOutput());
            }
        }
        pagesText_ << pageText;
    }
    pagesTextLoaded_ = !pagesText_.isEmpty();
    return pagesTextLoaded_;
}

QList<int> MainWindow::plainTextSearchPages(const QString& needle, int maxResults) {
    QList<int> pages;
    if (!ensurePagesTextLoaded()) return pages;
    const QString n = needle.trimmed(); if (n.isEmpty()) return pages;
    for (int i = 0; i < pagesText_.size(); ++i) {
        if (pagesText_[i].contains(n, Qt::CaseInsensitive)) {
            pages.append(i + 1);
            if (pages.size() >= maxResults) break;
        }
    }
    return pages;
}

QList<int> MainWindow::semanticSearchPages(const QString& query, int k) {
    QList<int> pages;
    IndexPaths paths; if (!getIndexPaths(&paths)) return pages;
    // Load index
    VectorIndex index;
    QString err;
    if (!index.load(paths.binPath, paths.idsPath, &err)) {
        qWarning() << "[Search] Falha ao carregar índice:" << err;
        return pages;
    }
    // Build embedding for query
    QSettings s;
    EmbeddingProvider::Config cfg;
    cfg.provider = s.value("emb/provider", "generativa").toString();
    cfg.model = s.value("emb/model", "nomic-embed-text:latest").toString();
    cfg.baseUrl = s.value("emb/base_url").toString();
    cfg.apiKey = s.value("emb/api_key").toString();
    EmbeddingProvider prov(cfg);
    // Minimal normalization (match indexer behavior): collapse whitespace and trim
    QString qnorm = query;
    qnorm.replace(QRegularExpression("\\s+"), " ");
    qnorm = qnorm.trimmed();
    QList<QVector<float>> qv;
    try { qv = prov.embedBatch(QStringList{qnorm}); }
    catch (const std::exception& ex) {
        qWarning() << "[Search] embed query failed:" << ex.what();
        return pages;
    }
    if (qv.isEmpty()) return pages;
    // Resolve Top-K and metric from settings
    const int topK = s.value("emb/top_k", qMax(1, k)).toInt();
    const QString metricStr = s.value("emb/similarity_metric", "cosine").toString();
    VectorIndex::Metric metric = VectorIndex::Metric::Cosine;
    if (metricStr == QLatin1String("dot")) metric = VectorIndex::Metric::Dot;
    else if (metricStr == QLatin1String("l2")) metric = VectorIndex::Metric::L2;
    const auto hitsAll = index.topK(qv.first(), qMax(1, topK), metric);
    // Apply thresholds
    const double simThreshold = s.value("emb/sim_threshold", 0.35).toDouble();
    const double l2Max = s.value("emb/l2_max_distance", 1.5).toDouble();
    QList<VectorIndex::Hit> hits;
    hits.reserve(hitsAll.size());
    for (const auto& h : hitsAll) {
        bool keep = true;
        if (metric == VectorIndex::Metric::L2) {
            // score = -distance; keep if distance <= l2Max => score >= -l2Max
            keep = (double(h.score) >= -l2Max);
        } else {
            // cosine/dot similarity: keep if score >= simThreshold
            keep = (double(h.score) >= simThreshold);
        }
        if (keep) hits.append(h);
    }
    if (hits.isEmpty()) return pages;
    QFile f(paths.metaPath);
    if (!f.open(QIODevice::ReadOnly)) return pages;
    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll()); f.close();
    if (!doc.isArray()) return pages;
    const QJsonArray arr = doc.array();
    QSet<int> seen;
    for (const auto& h : hits) {
        if (h.index < 0 || h.index >= arr.size()) continue;
        const int page = arr.at(h.index).toObject().value("page").toInt();
        if (page > 0 && !seen.contains(page)) { pages.append(page); seen.insert(page); }
        if (pages.size() >= k) break;
    }
    return pages;
}

void MainWindow::onSearchTriggered() {
    if (currentFilePath_.isEmpty() || !searchEdit_) return;
    const QString q = searchEdit_->text().trimmed();
    if (q.isEmpty()) return;
    // 1) Plain text search across pages
    QList<int> pages = plainTextSearchPages(q);
    // 2) Semantic search (RAG) if no plain hits
    if (pages.isEmpty()) {
        startRagSearch(q);
        return;
    }
    if (!pages.isEmpty()) {
        searchResultsPages_ = pages;
        searchResultIdx_ = 0;
        const int page = searchResultsPages_.at(searchResultIdx_);
        if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) { pv->setCurrentPage(static_cast<unsigned int>(page)); pv->flashHighlight(); }
        if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) { vw->setCurrentPage(static_cast<unsigned int>(page)); }
        updateStatus();
        statusBar()->showMessage(tr("%1 resultado(s)").arg(searchResultsPages_.size()), 2000);

        // Agentic mode: if query is longer, show constructed RAG prompt in chat dock and use LLM (LlmClient)
        // with retrieved context. Retrieval uses EmbeddingProvider (emb/*). Generation uses LlmClient settings (ai/*).
        const bool agentic = q.size() >= 120; // heuristic: long query triggers agentic prompt preview
        if (agentic) {
            showChatPanel();
            if (chatDock_) {
                // Build a readable agentic prompt describing the RAG procedure
                QString prompt;
                prompt += tr("Objetivo: Responder ao usuário em pt-BR com base no conteúdo do PDF atual, usando RAG.\n\n");
                prompt += tr("Entrada do usuário (consulta longa):\n%1\n\n").arg(q);
                prompt += tr("Passos (agentic):\n");
                prompt += tr("1) Recuperar passagens mais relevantes por similaridade semântica.\n");
                prompt += tr("2) Analisar passagens e redigir uma resposta objetiva, citando páginas quando possível.\n");
                prompt += tr("3) Não inventar conteúdo que não conste no documento.\n");
                prompt += tr("4) Responder em português do Brasil.\n");
                prompt += tr("5) Se necessário, sugerir páginas correlatas para leitura.\n\n");
                // Provide the retrieved page list in the prompt context
                prompt += tr("Páginas recuperadas (topo): %1\n").arg(
                    [&](){ QStringList s; for(int p: pages) s<<QString::number(p); return s.join(", "); }()
                );
                chatDock_->setAgenticPrompt(prompt);
                chatDock_->showAgenticPrompt(true);

                // Build RAG context from retrieved pages (use cached plain text if available)
                ensurePagesTextLoaded();
                QString ctx;
                int take = qMin(5, pages.size());
                for (int i = 0; i < take; ++i) {
                    const int pg = pages.at(i);
                    QString snippet;
                    if (pg-1 >= 0 && pg-1 < pagesText_.size()) {
                        snippet = pagesText_.at(pg-1).left(1000);
                    }
                    ctx += tr("[Página %1]\n%2\n\n").arg(pg).arg(snippet);
                }

                // Send to chat using LlmClient settings (ai/*)
                if (chatDock_) chatDock_->appendUser(q);
                if (llm_) {
                    QList<QPair<QString,QString>> msgs;
                    const QString sys = tr("Você é um assistente que responde com base no conteúdo do documento fornecido.\n"
                                           "Use apenas as informações relevantes do contexto, cite páginas quando útil, não invente.");
                    msgs.append({QStringLiteral("system"), sys});
                    const QString user = tr("Pergunta:\n%1\n\nContexto (trechos do PDF):\n%2").arg(q, ctx);
                    msgs.append({QStringLiteral("user"), user});
                    statusBar()->showMessage(tr("Consultando IA com contexto RAG..."));
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
            }
        }
    } else {
        statusBar()->showMessage(tr("Nenhum resultado encontrado."), 2000);
    }
}

void MainWindow::onSearchNext() {
    if (searchResultsPages_.isEmpty()) { onSearchTriggered(); return; }
    searchResultIdx_ = (searchResultIdx_ + 1) % searchResultsPages_.size();
    const int page = searchResultsPages_.at(searchResultIdx_);
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) { pv->setCurrentPage(static_cast<unsigned int>(page)); pv->flashHighlight(); }
    if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) { vw->setCurrentPage(static_cast<unsigned int>(page)); }
    updateStatus();
    statusBar()->showMessage(tr("Resultado %1 de %2").arg(searchResultIdx_+1).arg(searchResultsPages_.size()), 1500);
}

void MainWindow::onSearchPrev() {
    if (searchResultsPages_.isEmpty()) { onSearchTriggered(); return; }
    searchResultIdx_ = (searchResultIdx_ - 1 + searchResultsPages_.size()) % searchResultsPages_.size();
    const int page = searchResultsPages_.at(searchResultIdx_);
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) { pv->setCurrentPage(static_cast<unsigned int>(page)); pv->flashHighlight(); }
    if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) { vw->setCurrentPage(static_cast<unsigned int>(page)); }
    updateStatus();
    statusBar()->showMessage(tr("Resultado %1 de %2").arg(searchResultIdx_+1).arg(searchResultsPages_.size()), 1500);
}

void MainWindow::onDictionaryLookup(const QString& term) {
    if (!chatDock_) return;

    QSettings s;
    const bool useLlm = s.value("dictionary/use_llm", false).toBool();

    showChatPanel();

    if (useLlm) {
        if (llm_) {
            QList<QPair<QString,QString>> msgs;
            QString prompt = s.value("dictionary/llm_prompt", tr("Forneça o significado, a etimologia e os sinônimos da palavra: {palavra}")).toString();
            prompt.replace("{palavra}", term);

            const QString sys = tr("Você é um assistente de dicionário que responde à solicitação do usuário.");
            msgs.append({QStringLiteral("system"), sys});
            msgs.append({QStringLiteral("user"), prompt});
            statusBar()->showMessage(tr("Consultando IA para definição/tradução..."));
            llm_->chatWithMessages(msgs, [this, term](QString out, QString err){
                QMetaObject::invokeMethod(this, [this, term, out, err](){
                    if (!err.isEmpty()) {
                        showLongAlert(tr("Erro na IA"), err);
                        statusBar()->clearMessage();
                        return;
                    }
                    if (chatDock_) chatDock_->appendAssistant(QString("[dicionário] %1: %2").arg(term, out));
                    statusBar()->clearMessage();
                    saveChatForCurrentFile();
                });
            });
        }
        return;
    }

    const QString service = s.value("dictionary/service", "libre").toString();
    if (service == "libre") {
        const QString apiUrl = s.value("dictionary/libre/api_url", "https://libretranslate.de/translate").toString();
        const QString apiKey = s.value("dictionary/libre/api_key").toString();
        const QString sourceLang = s.value("dictionary/libre/source_lang", "en").toString();
        const QString targetLang = s.value("dictionary/libre/target_lang", "pt").toString();

        if (apiUrl.isEmpty()) {
            chatDock_->appendAssistant(tr("[dicionário] A URL da API do LibreTranslate não está configurada."));
            return;
        }

        statusBar()->showMessage(tr("Consultando dicionário (LibreTranslate)..."));

        QUrl url(apiUrl);
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        if (!apiKey.isEmpty()) {
            request.setRawHeader("Authorization", "Bearer " + apiKey.toUtf8());
        }

        QJsonObject json;
        json["q"] = term;
        json["source"] = sourceLang;
        json["target"] = targetLang;

        QJsonDocument doc(json);
        QByteArray data = doc.toJson();

        QNetworkReply* reply = netManager_->post(request, data);

        connect(reply, &QNetworkReply::finished, this, [this, reply, term]() {
            statusBar()->clearMessage();
            if (reply->error() != QNetworkReply::NoError) {
                chatDock_->appendAssistant(tr("[dicionário] Erro: %1").arg(reply->errorString()));
                reply->deleteLater();
                return;
            }

            const QByteArray responseData = reply->readAll();
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
            const QJsonObject jsonObj = jsonDoc.object();
            const QString translatedText = jsonObj.value("translatedText").toString();

            if (!translatedText.isEmpty() && translatedText.toLower() != term.toLower()) {
                chatDock_->appendAssistant(QString("[dicionário] %1: %2").arg(term, translatedText));
            } else {
                chatDock_->appendAssistant(tr("[dicionário] Nenhuma tradução encontrada para '%1'.").arg(term));
            }
            
            reply->deleteLater();
            saveChatForCurrentFile();
        });

    } else if (service == "omw") {
        chatDock_->appendAssistant(tr("[dicionário] O serviço Open Multilingual Wordnet (OMW) ainda não está implementado."));
    } else {
        chatDock_->appendAssistant(tr("[dicionário] Serviço de dicionário desconhecido ou não configurado."));
    }
}

void MainWindow::onRequestRebuildEmbeddings() {
    if (currentFilePath_.isEmpty()) {
        QMessageBox::information(this, tr("Recriar Embeddings"), tr("Abra um documento para recriar os embeddings."));
        return;
    }
    const auto ret = QMessageBox::question(
        this,
        tr("Recriar Embeddings"),
        tr("Este processo pode demorar, dependendo do tamanho do documento.\nDeseja continuar?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    if (ret != QMessageBox::Yes) return;

    // Progress UI
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Recriando embeddings"));
    dlg.setModal(true);
    auto* lay = new QVBoxLayout(&dlg);
    auto* lblStage = new QLabel(&dlg);
    lblStage->setText(tr("Iniciando..."));
    auto* bar = new QProgressBar(&dlg);
    bar->setRange(0, 100);
    bar->setValue(0);
    auto* details = new QPlainTextEdit(&dlg);
    details->setReadOnly(true);
    auto* btns = new QDialogButtonBox(QDialogButtonBox::Cancel, &dlg);
    lay->addWidget(lblStage);
    lay->addWidget(bar);
    lay->addWidget(details, 1);
    lay->addWidget(btns);

    // Load embedding settings
    QSettings s;
    const QString provider = s.value("emb/provider", "generativa").toString();
    const QString model = s.value("emb/model", "nomic-embed-text:latest").toString();
    const QString baseUrl = s.value("emb/base_url").toString();
    const QString apiKey = s.value("emb/api_key").toString();
    const QString dbPath = s.value("emb/db_path", QDir(QDir::home().filePath(".cache")).filePath("br.tec.rapport.genai-reader")).toString();
    const int chunkSize = s.value("emb/chunk_size", 1000).toInt();
    const int chunkOverlap = s.value("emb/chunk_overlap", 200).toInt();
    const int batchSize = s.value("emb/batch_size", 16).toInt();
    const int pagesPerStage = s.value("emb/pages_per_stage", -1).toInt();
    const int pauseMsBetweenBatches = s.value("emb/pause_ms_between_batches", 0).toInt();
    QDir().mkpath(dbPath);

    // Configure worker
    EmbeddingIndexer::Params params;
    params.pdfPath = currentFilePath_;
    params.dbDir = dbPath;
    params.providerCfg.provider = provider;
    params.providerCfg.model = model;
    params.providerCfg.baseUrl = baseUrl;
    params.providerCfg.apiKey = apiKey;
    params.chunkSize = chunkSize;
    params.chunkOverlap = chunkOverlap;
    params.batchSize = batchSize;
    params.pagesPerStage = pagesPerStage;
    params.pauseMsBetweenBatches = pauseMsBetweenBatches;

    auto* worker = new EmbeddingIndexer(params);
    auto* thread = new QThread(&dlg);
    worker->moveToThread(thread);

    connect(thread, &QThread::started, worker, &EmbeddingIndexer::run);
    connect(worker, &EmbeddingIndexer::stage, &dlg, [lblStage, details](const QString& name){ lblStage->setText(name); details->appendPlainText("[STAGE] " + name); });
    connect(worker, &EmbeddingIndexer::metric, &dlg, [details](const QString& k, const QString& v){ details->appendPlainText("[METRIC] " + k + " " + v); });
    connect(worker, &EmbeddingIndexer::progress, &dlg, [bar, details](int pct, const QString& info){ bar->setValue(qBound(0, pct, 100)); if (!info.isEmpty()) details->appendPlainText("[INFO] " + info); });
    connect(worker, &EmbeddingIndexer::warn, &dlg, [details](const QString& m){ details->appendPlainText("[WARN] " + m); });
    connect(worker, &EmbeddingIndexer::error, &dlg, [details](const QString& m){ details->appendPlainText("[ERROR] " + m); });
    connect(worker, &EmbeddingIndexer::finished, &dlg, [&](bool ok, const QString& msg){ if (ok) { bar->setValue(100); details->appendPlainText(tr("Concluído: ") + msg); dlg.accept(); } else { details->appendPlainText(tr("Falha: ") + msg); dlg.reject(); } });
    connect(&dlg, &QDialog::rejected, &dlg, [thread](){ if (thread->isRunning()) thread->requestInterruption(); });
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    thread->start();
    dlg.exec();
    if (thread->isRunning()) { thread->requestInterruption(); thread->quit(); thread->wait(1000); }

    if (bar->value() == 100) {
        statusBar()->showMessage(tr("Embeddings recriados."), 3000);
    }
}

void MainWindow::createActions() {
    // Core actions
    actOpen_ = new QAction(tr("Abrir"), this);
    actSaveAs_ = new QAction(tr("Salvar como..."), this);
    actClose_ = new QAction(tr("Fechar Ebook"), this);
    actQuit_ = new QAction(tr("Sair"), this);
    actReaderData_ = new QAction(tr("Dados do leitor..."), this);
    actPrev_ = new QAction(tr("Anterior"), this);
    actNext_ = new QAction(tr("Próxima"), this);
    actZoomIn_ = new QAction(tr("Zoom +"), this);
    actZoomOut_ = new QAction(tr("Zoom -"), this);
    actZoomReset_ = new QAction(tr("Zoom 100%"), this);
    actToggleTheme_ = new QAction(tr("Tema claro/escuro"), this);
    actChat_ = new QAction(tr("Chat"), this);
    actAbout_ = new QAction(tr("Informações..."), this);

    // Edit actions (seleção)
    actSelText_ = new QAction(tr("Selecionar texto"), this);
    actSelRect_ = new QAction(tr("Selecionar retângulo (imagem)"), this);
    actSelCopy_ = new QAction(tr("Copiar seleção"), this);
    actSelSaveTxt_ = new QAction(tr("Salvar seleção como TXT"), this);
    actSelSaveMd_ = new QAction(tr("Salvar seleção como Markdown"), this);
    actSelCopy_->setShortcut(QKeySequence::Copy);
    actSelCopy_->setShortcutContext(Qt::ApplicationShortcut);

    // TOC toolbar actions and wiring
    actTocModePages_ = new QAction(tr("Páginas"), this);
    actTocModeChapters_ = new QAction(tr("Conteúdo"), this);
    actTocPrev_ = new QAction(tr("Anterior"), this);
    actTocNext_ = new QAction(tr("Próxima"), this);
    actTocPrev_->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
    actTocNext_->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));
    actTocModePages_->setCheckable(true);
    actTocModeChapters_->setCheckable(true);
    actTocModePages_->setChecked(true);
    auto* tocModeGroup = new QActionGroup(this);
    tocModeGroup->addAction(actTocModePages_);
    tocModeGroup->addAction(actTocModeChapters_);
    tocModeGroup->setExclusive(true);

    // Shortcuts
    actOpen_->setShortcut(QKeySequence::Open);
    actSaveAs_->setShortcut(QKeySequence::SaveAs);
    actClose_->setShortcut(QKeySequence::Close);
    actQuit_->setShortcut(QKeySequence::Quit);
    actPrev_->setShortcut(Qt::Key_Left);
    actNext_->setShortcut(Qt::Key_Right);
    actZoomIn_->setShortcut(QKeySequence::ZoomIn);
    actZoomOut_->setShortcut(QKeySequence::ZoomOut);

    // Signals
    connect(actOpen_, &QAction::triggered, this, &MainWindow::openFile);
    connect(actSaveAs_, &QAction::triggered, this, &MainWindow::saveAs);
    connect(actPrev_, &QAction::triggered, this, &MainWindow::onTocPrev);
    connect(actNext_, &QAction::triggered, this, &MainWindow::onTocNext);
    connect(actZoomIn_, &QAction::triggered, this, &MainWindow::zoomIn);
    connect(actZoomOut_, &QAction::triggered, this, &MainWindow::zoomOut);
    connect(actZoomReset_, &QAction::triggered, this, &MainWindow::zoomReset);
    connect(actToggleTheme_, &QAction::triggered, this, &MainWindow::toggleTheme);
    connect(actQuit_, &QAction::triggered, qApp, &QApplication::quit);
    connect(actReaderData_, &QAction::triggered, this, &MainWindow::editReaderData);
    connect(actClose_, &QAction::triggered, this, &MainWindow::closeDocument);
    connect(actChat_, &QAction::triggered, this, &MainWindow::showChatPanel);
    connect(actAbout_, &QAction::triggered, this, &MainWindow::showAboutDialog);
    connect(actTocModePages_, &QAction::triggered, this, &MainWindow::setTocModePages);
    connect(actTocModeChapters_, &QAction::triggered, this, &MainWindow::setTocModeChapters);
    connect(actTocPrev_, &QAction::triggered, this, &MainWindow::onTocPrev);
    connect(actTocNext_, &QAction::triggered, this, &MainWindow::onTocNext);

    // Menus
    auto* menuArquivo = menuBar()->addMenu(tr("&Arquivo"));
    auto* menuDocumento = menuArquivo->addMenu(tr("Documento"));
    menuDocumento->addAction(actOpen_);
    menuDocumento->addAction(actSaveAs_);
    menuDocumento->addSeparator();
    menuDocumento->addAction(actClose_);

    menuRecent_ = menuDocumento->addMenu(tr("Recentes"));
    for (int i = 0; i < MaxRecentMenuItems; ++i) {
        recentActs_[i] = new QAction(this);
        recentActs_[i]->setVisible(false);
        connect(recentActs_[i], &QAction::triggered, this, &MainWindow::openRecentFile);
        menuRecent_->addAction(recentActs_[i]);
    }
    menuRecent_->addSeparator();
    actRecentDialog_ = new QAction(tr("Mostrar todos..."), this);
    connect(actRecentDialog_, &QAction::triggered, this, &MainWindow::showRecentDialog);
    menuRecent_->addAction(actRecentDialog_);

    auto* menuLeitor = menuArquivo->addMenu(tr("Leitor"));
    menuLeitor->addAction(actReaderData_);
    menuArquivo->addSeparator();
    menuArquivo->addAction(actQuit_);

    auto* menuConfig = menuBar()->addMenu(tr("Configurações"));
    menuConfig->addAction(actToggleTheme_);
    actWheelZoomPref_ = new QAction(tr("Granularidade do zoom (Ctrl+roda)..."), this);
    connect(actWheelZoomPref_, &QAction::triggered, this, &MainWindow::setWheelZoomPreference);
    menuConfig->addAction(actWheelZoomPref_);
    auto* menuLlm = menuConfig->addMenu(tr("LLM"));
    actLlmSettings_ = new QAction(tr("Configurar LLM..."), this);
    connect(actLlmSettings_, &QAction::triggered, this, &MainWindow::openLlmSettings);
    menuLlm->addAction(actLlmSettings_);

    auto* menuEmb = menuConfig->addMenu(tr("Embeddings"));
    actEmbeddingSettings_ = new QAction(tr("Configurar Embeddings..."), this);
    connect(actEmbeddingSettings_, &QAction::triggered, this, &MainWindow::openEmbeddingSettings);
    menuEmb->addAction(actEmbeddingSettings_);

    actDictionarySettings_ = new QAction(tr("Configurar Dicionários..."), this);
    connect(actDictionarySettings_, &QAction::triggered, this, &MainWindow::openDictionarySettings);
    menuConfig->addAction(actDictionarySettings_);

    auto* menuEdit = menuBar()->addMenu(tr("Editar"));
    menuEdit->addAction(actSelText_);
    menuEdit->addAction(actSelRect_);
    menuEdit->addSeparator();
    menuEdit->addAction(actSelCopy_);
    menuEdit->addAction(actSelSaveTxt_);
    menuEdit->addAction(actSelSaveMd_);

    auto* menuHelp = menuBar()->addMenu(tr("Ajuda"));
    menuHelp->addAction(actAbout_);

    // Top toolbar: centered title button
    auto* tbTitle = addToolBar(tr("Título"));
    tbTitle->setObjectName("toolbar_title");
    QWidget* titleLeftSpacer = new QWidget(tbTitle);
    titleLeftSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tbTitle->addWidget(titleLeftSpacer);
    // Title button (shows current document title/name; left-click shows full path; right-click opens menu)
    titleButton_ = new QToolButton(tbTitle);
    titleButton_->setToolButtonStyle(Qt::ToolButtonTextOnly);
    titleButton_->setPopupMode(QToolButton::InstantPopup); // context menu appears on right-click; left-click handled via clicked()
    titleMenu_ = new QMenu(titleButton_);
    actTitleOpenDir_ = new QAction(tr("Abrir diretório no gerenciador"), titleMenu_);
    actTitleAddToCalibre_ = new QAction(tr("Adicionar ao Calibre e migrar embeddings..."), titleMenu_);
    actTitleRename_ = new QAction(tr("Renomear arquivo e migrar embeddings..."), titleMenu_);
    titleMenu_->addAction(actTitleOpenDir_);
    titleMenu_->addSeparator();
    titleMenu_->addAction(actTitleAddToCalibre_);
    titleMenu_->addAction(actTitleRename_);
    titleButton_->setMenu(titleMenu_);
    connect(titleButton_, &QToolButton::clicked, this, &MainWindow::showCurrentPathInfo);
    connect(actTitleOpenDir_, &QAction::triggered, this, &MainWindow::onTitleOpenDir);
    connect(actTitleAddToCalibre_, &QAction::triggered, this, &MainWindow::onTitleAddToCalibre);
    connect(actTitleRename_, &QAction::triggered, this, &MainWindow::onTitleRenameFile);
    tbTitle->addWidget(titleButton_);
    QWidget* titleRightSpacer = new QWidget(tbTitle);
    titleRightSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tbTitle->addWidget(titleRightSpacer);

    // Break to start second row
    addToolBarBreak();

    // Second toolbar: reading controls
    auto* tb = addToolBar(tr("Leitura"));
    tb->setObjectName("toolbar_reading");
    QWidget* leftSpacer = new QWidget(tb);
    leftSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tb->addWidget(leftSpacer);
    tb->addAction(actPrev_);
    tb->addAction(actNext_);
    pageCombo_ = new QComboBox(tb);
    pageCombo_->setEditable(false);
    pageCombo_->setMinimumContentsLength(6);
    tb->addWidget(pageCombo_);
    connect(pageCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx){
        const unsigned int p = static_cast<unsigned int>(idx + 1);
        if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) vw->setCurrentPage(p);
        if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) pv->setCurrentPage(p);
        updateStatus();
    });
    tb->addSeparator();
    tb->addAction(actZoomOut_);
    tb->addAction(actZoomIn_);
    tb->addAction(actZoomReset_);
    QWidget* rightSpacer = new QWidget(tb);
    rightSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tb->addWidget(rightSpacer);
    tb->addAction(actChat_);

    // Place the search toolbar on the same second row (no extra break)
    searchToolBar_ = addToolBar(tr("Busca"));
    searchToolBar_->setObjectName("toolbar_search");
    searchToolBar_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    searchEdit_ = new QLineEdit(searchToolBar_);
    searchEdit_->setPlaceholderText(tr("/ Pesquisar no documento..."));
    searchEdit_->setClearButtonEnabled(true);
    searchToolBar_->addWidget(searchEdit_);
    searchButton_ = new QPushButton(tr("Pesquisar"), searchToolBar_);
    searchToolBar_->addWidget(searchButton_);
    searchPrevButton_ = new QPushButton(tr("Anterior"), searchToolBar_);
    searchNextButton_ = new QPushButton(tr("Próximo"), searchToolBar_);
    searchToolBar_->addWidget(searchPrevButton_);
    searchToolBar_->addWidget(searchNextButton_);

    // Quick options: metric + topK + thresholds
    searchOptionsButton_ = new QToolButton(searchToolBar_);
    searchOptionsButton_->setText(tr("Opções"));
    searchOptionsButton_->setPopupMode(QToolButton::InstantPopup);
    searchOptionsMenu_ = new QMenu(searchOptionsButton_);
    actMetricCosine_ = searchOptionsMenu_->addAction(tr("Métrica: Cosseno")); actMetricCosine_->setCheckable(true);
    actMetricDot_ = searchOptionsMenu_->addAction(tr("Métrica: Dot")); actMetricDot_->setCheckable(true);
    actMetricL2_ = searchOptionsMenu_->addAction(tr("Métrica: L2")); actMetricL2_->setCheckable(true);
    searchOptionsMenu_->addSeparator();
    QWidget* topKWidget = new QWidget(searchOptionsMenu_);
    QHBoxLayout* topKLayout = new QHBoxLayout(topKWidget);
    topKLayout->setContentsMargins(8,4,8,4);
    QLabel* topKLabel = new QLabel(tr("Top-K:"), topKWidget);
    topKSpin_ = new QSpinBox(topKWidget);
    topKSpin_->setRange(1, 1000);
    topKSpin_->setValue(5);
    topKLayout->addWidget(topKLabel);
    topKLayout->addWidget(topKSpin_);
    QWidgetAction* topKAction = new QWidgetAction(searchOptionsMenu_);
    topKAction->setDefaultWidget(topKWidget);
    searchOptionsMenu_->addAction(topKAction);

    // Similarity threshold (for cosine/dot)
    QWidget* simWidget = new QWidget(searchOptionsMenu_);
    QHBoxLayout* simLayout = new QHBoxLayout(simWidget);
    simLayout->setContentsMargins(8,4,8,4);
    QLabel* simLabel = new QLabel(tr("Mín. similaridade (cos/dot):"), simWidget);
    simThresholdSpin_ = new QDoubleSpinBox(simWidget);
    simThresholdSpin_->setDecimals(3);
    simThresholdSpin_->setRange(-1.0, 1.0);
    simThresholdSpin_->setSingleStep(0.01);
    simThresholdSpin_->setValue(0.35); // default
    simLayout->addWidget(simLabel);
    simLayout->addWidget(simThresholdSpin_);
    QWidgetAction* simAction = new QWidgetAction(searchOptionsMenu_);
    simAction->setDefaultWidget(simWidget);
    searchOptionsMenu_->addAction(simAction);

    // L2 distance threshold (max distance)
    QWidget* l2Widget = new QWidget(searchOptionsMenu_);
    QHBoxLayout* l2Layout = new QHBoxLayout(l2Widget);
    l2Layout->setContentsMargins(8,4,8,4);
    QLabel* l2Label = new QLabel(tr("Máx. distância (L2):"), l2Widget);
    l2MaxDistSpin_ = new QDoubleSpinBox(l2Widget);
    l2MaxDistSpin_->setDecimals(3);
    l2MaxDistSpin_->setRange(0.0, 100000.0);
    l2MaxDistSpin_->setSingleStep(0.1);
    l2MaxDistSpin_->setValue(1.5); // conservative default, depends on model scale
    l2Layout->addWidget(l2Label);
    l2Layout->addWidget(l2MaxDistSpin_);
    QWidgetAction* l2Action = new QWidgetAction(searchOptionsMenu_);
    l2Action->setDefaultWidget(l2Widget);
    searchOptionsMenu_->addAction(l2Action);
    searchOptionsButton_->setMenu(searchOptionsMenu_);
    searchToolBar_->addWidget(searchOptionsButton_);

    // Wire options
    connect(actMetricCosine_, &QAction::triggered, this, &MainWindow::onSearchMetricCosine);
    connect(actMetricDot_, &QAction::triggered, this, &MainWindow::onSearchMetricDot);
    connect(actMetricL2_, &QAction::triggered, this, &MainWindow::onSearchMetricL2);
    connect(topKSpin_, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onSearchTopKChanged);
    // Persist threshold changes immediately
    connect(simThresholdSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double v){ QSettings s; s.setValue("emb/sim_threshold", v); });
    connect(l2MaxDistSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double v){ QSettings s; s.setValue("emb/l2_max_distance", v); });
    connect(searchEdit_, &QLineEdit::returnPressed, this, &MainWindow::onSearchTriggered);
    connect(searchButton_, &QPushButton::clicked, this, &MainWindow::onSearchTriggered);
    connect(searchPrevButton_, &QPushButton::clicked, this, &MainWindow::onSearchPrev);
    connect(searchNextButton_, &QPushButton::clicked, this, &MainWindow::onSearchNext);

    // Wire '/' shortcut to focus the search bar
    slashShortcut_ = new QShortcut(QKeySequence("/"), this);
    slashShortcut_->setContext(Qt::ApplicationShortcut);
    connect(slashShortcut_, &QShortcut::activated, this, &MainWindow::focusSearchBar);

    // Initialize options from settings
    loadSearchOptionsFromSettings();

    // TOC toolbar
    if (tocToolBar_) {
        tocToolBar_->addAction(actTocModePages_);
        tocToolBar_->addAction(actTocModeChapters_);
        tocToolBar_->addSeparator();
        tocToolBar_->addAction(actTocPrev_);
        tocToolBar_->addAction(actTocNext_);
    }

    connect(toc_, &QTreeWidget::itemActivated, this, &MainWindow::onTocItemActivated);
    connect(toc_, &QTreeWidget::itemClicked, this, &MainWindow::onTocItemActivated);

    // Initial state
    actClose_->setEnabled(false);
    rebuildRecentMenu();

    updateTitleWidget();
}

void saveRecentEntries(QSettings& settings, const QVariantList& entries) {
    settings.setValue("recent/entries", entries);
}

QVariantMap extractPdfMeta(const QString& path) {
    QVariantMap m; m["path"] = path;
    m["title"] = QFileInfo(path).completeBaseName();
    m["author"] = QString();
    m["publisher"] = QString(); // Not available via QPdfDocument; left empty
    m["isbn"] = QString();       // Not available via QPdfDocument; left empty
    m["summary"] = QString();
    m["keywords"] = QString();
    QPdfDocument doc;
    const auto status = doc.load(path);
    if (status == static_cast<QPdfDocument::Error>(0)) {
        const auto title = doc.metaData(QPdfDocument::MetaDataField::Title).toString();
        const auto author = doc.metaData(QPdfDocument::MetaDataField::Author).toString();
        const auto subject = doc.metaData(QPdfDocument::MetaDataField::Subject).toString();
        const auto keywords = doc.metaData(QPdfDocument::MetaDataField::Keywords).toString();
        if (!title.trimmed().isEmpty()) m["title"] = title.trimmed();
        if (!author.trimmed().isEmpty()) m["author"] = author.trimmed();
        if (!subject.trimmed().isEmpty()) m["summary"] = subject.trimmed();
        if (!keywords.trimmed().isEmpty()) m["keywords"] = keywords.trimmed();
    }
    return m;
}

bool entryMatches(const QVariantMap& e, const QString& needle) {
    const QString n = needle.trimmed(); if (n.isEmpty()) return true;
    const auto contains = [&](const QString& v){ return v.contains(n, Qt::CaseInsensitive); };
    return contains(e.value("path").toString()) ||
           contains(e.value("title").toString()) ||
           contains(e.value("author").toString()) ||
           contains(e.value("publisher").toString()) ||
           contains(e.value("isbn").toString()) ||
           contains(e.value("summary").toString()) ||
           contains(e.value("keywords").toString());
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), settings_() {
    buildUi();
    createActions();
    loadSettings();
    updateStatus();
    netManager_ = new QNetworkAccessManager(this);
    // Initialize LLM and summary dialog
    llm_ = new LlmClient(this);
    summaryDlg_ = new SummaryDialog(this);
    connect(summaryDlg_, &SummaryDialog::sendToChatRequested, this, &MainWindow::onRequestSendToChat);
    // Ensure chat history is loaded for current file when panel opens
    if (!currentFilePath_.isEmpty()) {
        loadChatForFile(currentFilePath_);
    }
}

QJsonArray MainWindow::readChatSessions(const QString& filePath) {
    const QString key = QString("files/%1/chatSessions").arg(filePath);
    const QString raw = settings_.value(key).toString();
    if (raw.trimmed().isEmpty()) return {};
    const QJsonDocument doc = QJsonDocument::fromJson(raw.toUtf8());
    return doc.isArray() ? doc.array() : QJsonArray{};
}

void MainWindow::writeChatSessions(const QString& filePath, const QJsonArray& sessions) {
    const QString key = QString("files/%1/chatSessions").arg(filePath);
    settings_.setValue(key, QJsonDocument(sessions).toJson(QJsonDocument::Compact));
}

void MainWindow::showSavedChatsPicker() {
    if (currentFilePath_.isEmpty() || !chatDock_) return;
    const QJsonArray sessions = readChatSessions(currentFilePath_);
    if (sessions.isEmpty()) { QMessageBox::information(this, tr("Histórico"), tr("Nenhum chat salvo.")); return; }
    // Build a simple list dialog with titles
    QStringList titles; titles.reserve(sessions.size());
    for (const auto& v : sessions) { titles << v.toObject().value("title").toString(); }
    bool ok = false;
    const QString chosen = QInputDialog::getItem(this, tr("Histórico de chats"), tr("Escolha uma conversa:"), titles, titles.size()-1, false, &ok);
    if (!ok || chosen.isEmpty()) return;
    const int idx = titles.indexOf(chosen);
    if (idx < 0) return;
    const QJsonObject obj = sessions.at(idx).toObject();
    const QString html = obj.value("html").toString();
    QList<QPair<QString,QString>> msgs;
    const QJsonArray arr = obj.value("msgs").toArray();
    for (const auto& m : arr) { const auto o = m.toObject(); msgs.append(QPair<QString,QString>(o.value("role").toString(), o.value("content").toString())); }
    chatDock_->setTranscriptHtml(html);
    chatDock_->setConversationForLlm(msgs);
}

void MainWindow::enableAutoSelection() {
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
        pv->setSelectionMode(PdfViewerWidget::SelectionMode::Auto);
    }
}

void MainWindow::enableTextSelection() {
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
        pv->setSelectionMode(PdfViewerWidget::SelectionMode::Text);
    }
}

void MainWindow::enableRectSelection() {
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
        pv->setSelectionMode(PdfViewerWidget::SelectionMode::Rect);
    }
}

// ===== Quick options (metric / topK) and RAG helpers (definitions) =====

static void mw_setExclusive(QAction* a, QAction* b, QAction* c, QAction* on) {
    if (!a||!b||!c||!on) return;
    a->setChecked(a==on);
    b->setChecked(b==on);
    c->setChecked(c==on);
}

void MainWindow::loadSearchOptionsFromSettings() {
    QSettings s;
    const QString metric = s.value("emb/similarity_metric", "cosine").toString();
    const int topK = s.value("emb/top_k", 5).toInt();
    const double simTh = s.value("emb/sim_threshold", 0.35).toDouble();
    const double l2Max = s.value("emb/l2_max_distance", 1.5).toDouble();
    if (actMetricCosine_) actMetricCosine_->setChecked(metric == QLatin1String("cosine"));
    if (actMetricDot_) actMetricDot_->setChecked(metric == QLatin1String("dot"));
    if (actMetricL2_) actMetricL2_->setChecked(metric == QLatin1String("l2"));
    if (topKSpin_) topKSpin_->setValue(qMax(1, topK));
    if (simThresholdSpin_) simThresholdSpin_->setValue(simTh);
    if (l2MaxDistSpin_) l2MaxDistSpin_->setValue(l2Max);
    // Enable only relevant controls according to metric
    const bool isCosOrDot = (metric == QLatin1String("cosine")) || (metric == QLatin1String("dot"));
    if (simThresholdSpin_) simThresholdSpin_->setEnabled(isCosOrDot);
    if (l2MaxDistSpin_) l2MaxDistSpin_->setEnabled(metric == QLatin1String("l2"));
}

void MainWindow::saveSearchOptionsToSettings(const QString& metricKey, int topK) {
    QSettings s;
    s.setValue("emb/similarity_metric", metricKey);
    s.setValue("emb/top_k", qMax(1, topK));
    if (simThresholdSpin_) s.setValue("emb/sim_threshold", simThresholdSpin_->value());
    if (l2MaxDistSpin_) s.setValue("emb/l2_max_distance", l2MaxDistSpin_->value());
}

void MainWindow::onSearchMetricCosine() {
    mw_setExclusive(actMetricCosine_, actMetricDot_, actMetricL2_, actMetricCosine_);
    const int topK = topKSpin_ ? topKSpin_->value() : 5;
    saveSearchOptionsToSettings("cosine", topK);
    // Update controls availability
    if (simThresholdSpin_) simThresholdSpin_->setEnabled(true);
    if (l2MaxDistSpin_) l2MaxDistSpin_->setEnabled(false);
}

void MainWindow::onSearchMetricDot() {
    mw_setExclusive(actMetricCosine_, actMetricDot_, actMetricL2_, actMetricDot_);
    const int topK = topKSpin_ ? topKSpin_->value() : 5;
    saveSearchOptionsToSettings("dot", topK);
    if (simThresholdSpin_) simThresholdSpin_->setEnabled(true);
    if (l2MaxDistSpin_) l2MaxDistSpin_->setEnabled(false);
}

void MainWindow::onSearchMetricL2() {
    mw_setExclusive(actMetricCosine_, actMetricDot_, actMetricL2_, actMetricL2_);
    const int topK = topKSpin_ ? topKSpin_->value() : 5;
    saveSearchOptionsToSettings("l2", topK);
    if (simThresholdSpin_) simThresholdSpin_->setEnabled(false);
    if (l2MaxDistSpin_) l2MaxDistSpin_->setEnabled(true);
}

void MainWindow::onSearchTopKChanged(int value) {
    QString metric = "cosine";
    if (actMetricDot_ && actMetricDot_->isChecked()) metric = "dot";
    else if (actMetricL2_ && actMetricL2_->isChecked()) metric = "l2";
    saveSearchOptionsToSettings(metric, value);
}

QString MainWindow::detectDocumentLanguageSample() const {
    if (!pagesTextLoaded_) return QString();
    QString accum;
    for (int i=0;i<qMin(pagesText_.size(), 3); ++i) {
        if (!pagesText_[i].trimmed().isEmpty()) accum += pagesText_[i].left(800) + "\n";
    }
    return accum;
}

void MainWindow::detectDocumentLanguageAsync(std::function<void(QString)> onLang) {
    const QString sample = detectDocumentLanguageSample();
    if (!llm_ || sample.trimmed().isEmpty()) { if (onLang) onLang(QStringLiteral("pt")); return; }
    showChatPanel();
    if (chatDock_) {
        chatDock_->appendAssistant(tr("[LLM] Detectando idioma do documento a partir de amostra..."));
    }
    QList<QPair<QString,QString>> msgs;
    msgs.append({QStringLiteral("system"), tr("Responda apenas com o código ISO 639-1 do idioma predominante do texto do usuário.")});
    msgs.append({QStringLiteral("user"), sample});
    llm_->chatWithMessages(msgs, [this, onLang](QString out, QString err){
        QMetaObject::invokeMethod(this, [this, onLang, out, err]{
            QString lang = QStringLiteral("pt");
            if (err.isEmpty()) {
                lang = out.trimmed().toLower();
                if (lang.size()>2) lang = lang.left(2);
                if (lang.isEmpty()) lang = QStringLiteral("pt");
            }
            if (auto cd = this->chatDock_) {
                if (!err.isEmpty()) cd->appendAssistant(tr("[LLM] Erro ao detectar idioma: %1").arg(err));
                else cd->appendAssistant(tr("[LLM] Idioma detectado: %1").arg(lang));
            }
            if (onLang) onLang(lang);
        });
    });
}

void MainWindow::translateQueryIfNeededAsync(const QString& query, const QString& docLang, std::function<void(QString)> onReady) {
    if (!llm_ || docLang.isEmpty() || docLang == QLatin1String("pt")) { if (onReady) onReady(query); return; }
    showChatPanel();
    if (chatDock_) chatDock_->appendAssistant(tr("[LLM] Traduzindo consulta para '%1'...").arg(docLang));
    QList<QPair<QString,QString>> msgs;
    const QString sys = tr("Você traduzirá frases para o idioma alvo indicado, respondendo apenas a tradução, sem comentários.");
    msgs.append({QStringLiteral("system"), sys});
    msgs.append({QStringLiteral("user"), tr("Traduza para %1: %2").arg(docLang, query)});
    llm_->chatWithMessages(msgs, [this, onReady](QString out, QString err){
        QMetaObject::invokeMethod(this, [this, onReady, out, err]{
            const QString translated = err.isEmpty() ? out.trimmed() : QString();
            if (auto cd = this->chatDock_) {
                if (!err.isEmpty()) cd->appendAssistant(tr("[LLM] Erro ao traduzir: %1").arg(err));
                else cd->appendAssistant(tr("[LLM] Tradução: %1").arg(translated.isEmpty() ? out.trimmed() : translated));
            }
            if (onReady) onReady(translated.isEmpty() ? out.trimmed() : translated);
        });
    });
}

void MainWindow::startRagSearch(const QString& userQuery) {
    pendingRagQuery_ = userQuery;
    ensurePagesTextLoaded();
    // Mirror the user's search query into chat
    showChatPanel();
    if (chatDock_) chatDock_->appendUser(tr("/buscar: %1").arg(userQuery));
    statusBar()->showMessage(tr("Detectando idioma do documento..."));
    detectDocumentLanguageAsync([this, userQuery](QString lang){
        statusBar()->showMessage(tr("Traduzindo consulta se necessário (%1)...").arg(lang), 1500);
        translateQueryIfNeededAsync(userQuery, lang, [this](QString translated){
            ensureIndexAvailableThen(translated);
        });
    });
}

void MainWindow::ensureIndexAvailableThen(const QString& translatedQuery) {
    IndexPaths paths; if (getIndexPaths(&paths)) { continueRagAfterEnsureIndex(translatedQuery); return; }
    const auto ret = QMessageBox::question(this, tr("Criar índice de embeddings?"),
        tr("Este documento ainda não possui índice de embeddings.\nDeseja criar agora? Este processo pode ser demorado."),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (ret != QMessageBox::Yes) { statusBar()->showMessage(tr("Busca semântica cancelada (sem índice)."), 2500); return; }

    QSettings s;
    EmbeddingIndexer::Params p;
    p.pdfPath = currentFilePath_;
    p.providerCfg.provider = s.value("emb/provider", "generativa").toString();
    p.providerCfg.model = s.value("emb/model", "nomic-embed-text:latest").toString();
    p.providerCfg.baseUrl = s.value("emb/base_url").toString();
    p.providerCfg.apiKey = s.value("emb/api_key").toString();
    p.dbDir = s.value("emb/db_path", QDir(QDir::home().filePath(".cache")).filePath("br.tec.rapport.genai-reader")).toString();
    p.chunkSize = s.value("emb/chunk_size", 1000).toInt();
    p.chunkOverlap = s.value("emb/chunk_overlap", 200).toInt();
    p.batchSize = s.value("emb/batch_size", 16).toInt();
    p.pagesPerStage = s.value("emb/pages_per_stage", -1).toInt();
    p.pauseMsBetweenBatches = s.value("emb/pause_ms_between_batches", 0).toInt();

    auto* thread = new QThread(this);
    auto* indexer = new EmbeddingIndexer(p);
    indexer->moveToThread(thread);
    connect(thread, &QThread::started, indexer, &EmbeddingIndexer::run);
    connect(indexer, &EmbeddingIndexer::finished, this, [this, thread, indexer, translatedQuery](bool ok, const QString&){
        thread->quit();
        thread->wait();
        indexer->deleteLater();
        thread->deleteLater();
        if (!ok) { statusBar()->showMessage(tr("Falha ao criar o índice."), 3000); return; }
        continueRagAfterEnsureIndex(translatedQuery);
    });
    thread->start();
}

void MainWindow::continueRagAfterEnsureIndex(const QString& translatedQuery) {
    QList<int> pages = semanticSearchPages(translatedQuery);
    if (!pages.isEmpty()) {
        searchResultsPages_ = pages;
        searchResultIdx_ = 0;
        const int page = searchResultsPages_.at(searchResultIdx_);
        if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) { pv->setCurrentPage(static_cast<unsigned int>(page)); pv->flashHighlight(); }
        if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) { vw->setCurrentPage(static_cast<unsigned int>(page)); }
        updateStatus();
        statusBar()->showMessage(tr("%1 resultado(s)").arg(searchResultsPages_.size()), 2000);
    } else {
        statusBar()->showMessage(tr("Nenhum resultado encontrado."), 2000);
    }
}

void MainWindow::copySelection() {
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
        pv->copySelection();
    }
}

void MainWindow::saveSelectionTxt() {
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
        pv->saveSelectionAsTxt();
    }
}

void MainWindow::saveSelectionMd() {
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
        pv->saveSelectionAsMarkdown();
    }
}

void MainWindow::applyDefaultSplitterSizesIfNeeded() {
    // Only apply if there is no saved state; heuristic: if sizes are empty or equal/zero
    if (!splitter_) return;
    const QList<int> sizes = splitter_->sizes();
    bool needDefault = sizes.isEmpty();
    if (!needDefault) {
        int total = 0; for (int s : sizes) total += s;
        needDefault = (total == 0);
    }
    if (needDefault) {
        // Approximate 10% for TOC, 90% for viewer
        splitter_->setSizes({100, 900});
    }
}

void MainWindow::setTocModePages() {
    tocPagesMode_ = true;
    // Rebuild TOC based on current document
    toc_->clear();
    unsigned int pages = 0;
    if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) { pages = vw->totalPages(); }
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) { pages = pv->totalPages(); }
    if (pages == 0) return;
    for (unsigned int i = 1; i <= pages; ++i) {
        auto* item = new QTreeWidgetItem(QStringList{tr("Página %1").arg(i)});
        item->setData(0, Qt::UserRole, i);
        toc_->addTopLevelItem(item);
    }
}

void MainWindow::setTocModeChapters() {
    tocPagesMode_ = false;
    toc_->clear();

    unsigned int pages = 0;
    if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) { pages = vw->totalPages(); }
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) { pages = pv->totalPages(); }

    // Try to build TOC from real PDF bookmarks (titles) when available
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
        if (QPdfDocument* doc = pv->document()) {
            QPdfBookmarkModel bm(doc);
            if (bm.rowCount() > 0) {
                std::function<QTreeWidgetItem*(const QModelIndex&)> buildNode = [&](const QModelIndex& idx) -> QTreeWidgetItem* {
                    // Title
                    const QString title = bm.data(idx, Qt::DisplayRole).toString();
                    auto* item = new QTreeWidgetItem(QStringList{ title.isEmpty() ? tr("(sem título)") : title });
                    const int rows = bm.rowCount(idx);
                    for (int r = 0; r < rows; ++r) {
                        const QModelIndex childIdx = bm.index(r, 0, idx);
                        if (childIdx.isValid()) {
                            if (auto* child = buildNode(childIdx)) item->addChild(child);
                        }
                    }
                    return item;
                };

                // Build top-level items
                for (int r = 0; r < bm.rowCount(); ++r) {
                    const QModelIndex topIdx = bm.index(r, 0);
                    if (!topIdx.isValid()) continue;
                    if (auto* node = buildNode(topIdx)) toc_->addTopLevelItem(node);
                }

                // Provide better UX: expand first levels so subcapítulos are visible
                toc_->expandToDepth(1);

                // Note: older Qt may not expose destinations via roles; we keep non-clickable TOC here.
                // Navigation will fallback below when no bookmarks provide page targets.
            }
        }
    }

    // Fallback: placeholder grouping when no bookmarks are available
    if (pages == 0) return;
    const unsigned int group = 10;
    for (unsigned int start = 1; start <= pages; start += group) {
        const unsigned int end = std::min(start + group - 1, pages);
        auto* rangeItem = new QTreeWidgetItem(QStringList{tr("Páginas %1–%2").arg(start).arg(end)});
        // Clicking the range jumps to its first page
        rangeItem->setData(0, Qt::UserRole, start);
        rangeItem->setToolTip(0, tr("Da página %1 até %2").arg(start).arg(end));
        toc_->addTopLevelItem(rangeItem);
    }
}

void MainWindow::onTocPrev() {
    if (tocPagesMode_) { prevPage(); return; }
    // Chapters mode: move selection to previous item
    auto* cur = toc_->currentItem();
    if (!cur) {
        if (toc_->topLevelItemCount() > 0) toc_->setCurrentItem(toc_->topLevelItem(0));
        return;
    }
    // Find previous item in visible order
    QTreeWidgetItem* prev = toc_->itemAbove(cur);
    if (!prev && cur->parent()) prev = cur->parent();
    if (!prev) return;
    toc_->setCurrentItem(prev);
    onTocItemActivated(prev, 0);
}

void MainWindow::onTocNext() {
    if (tocPagesMode_) { nextPage(); return; }
    auto* cur = toc_->currentItem();
    if (!cur) {
        if (toc_->topLevelItemCount() > 0) toc_->setCurrentItem(toc_->topLevelItem(0));
        return;
    }
    // Prefer first child, else next sibling, else climb up to find next
    QTreeWidgetItem* nxt = nullptr;
    if (cur->childCount() > 0) {
        nxt = cur->child(0);
    } else {
        nxt = toc_->itemBelow(cur);
    }
    if (!nxt) return;
    toc_->setCurrentItem(nxt);
    onTocItemActivated(nxt, 0);
}

bool MainWindow::validateReaderInputs(const QString& name, const QString& email, QString* errorMsg) const {
    QStringList errors;
    if (name.trimmed().isEmpty()) errors << tr("Nome completo é obrigatório.");
    QRegularExpression re(R"(^[A-Z0-9._%+-]+@[A-Z0-9.-]+\.[A-Z]{2,}$)", QRegularExpression::CaseInsensitiveOption);
    if (!re.match(email.trimmed()).hasMatch()) errors << tr("E-mail inválido.");
    if (!errors.isEmpty()) {
        if (errorMsg) *errorMsg = errors.join('\n');
        return false;
    }
    return true;
}

QMap<QString, QString> MainWindow::loadEnvConfig() const {
    QMap<QString, QString> cfg;
    // Determine preferred .env path: application dir first, then CWD
    QString appDir = QCoreApplication::applicationDirPath();
    QString envPath = QDir(appDir).filePath(".env");
    if (!QFileInfo::exists(envPath)) {
        QString cwdEnv = QDir::current().filePath(".env");
        envPath = QFileInfo::exists(cwdEnv) ? cwdEnv : envPath;
    }
    QFile f(envPath);
    if (f.exists() && f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        while (!f.atEnd()) {
            const QByteArray line = f.readLine();
            const QByteArray trimmed = line.trimmed();
            if (trimmed.isEmpty() || trimmed.startsWith('#')) continue;
            const int eq = trimmed.indexOf('=');
            if (eq <= 0) continue;
            const QString key = QString::fromUtf8(trimmed.left(eq));
            const QString val = QString::fromUtf8(trimmed.mid(eq+1));
            cfg.insert(key, val);
        }
        f.close();
    }
    return cfg;
}

void MainWindow::submitReaderDataToPhpList(const QString& name, const QString& email, const QString& whatsapp) {
    const auto cfg = loadEnvConfig();
    const QString baseUrl = cfg.value("PHPLIST_URL");
    const QString user = cfg.value("PHPLIST_USER");
    const QString pass = cfg.value("PHPLIST_PASS");
    if (baseUrl.isEmpty() || user.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, tr("Configuração ausente"), tr("Parâmetros do PHPList ausentes no .env."));
        return;
    }

    QUrl url(baseUrl);
    if (!url.isValid()) {
        QMessageBox::warning(this, tr("URL inválida"), tr("A URL do PHPList no .env é inválida."));
        return;
    }

    // Monta payload genérico (JSON). A API específica pode exigir outro formato; manteremos configurável.
    QJsonObject payload;
    payload.insert("email", email);
    if (!name.trimmed().isEmpty()) payload.insert("name", name.trimmed());
    if (!whatsapp.trimmed().isEmpty()) {
        QJsonObject attrs; attrs.insert("whatsapp", whatsapp.trimmed());
        payload.insert("attributes", attrs);
    }

    const QByteArray data = QJsonDocument(payload).toJson(QJsonDocument::Compact);

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    // Basic Auth
    const QByteArray credentials = (user + ":" + pass).toUtf8().toBase64();
    req.setRawHeader("Authorization", "Basic " + credentials);

    auto* reply = netManager_->post(req, data);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            // Não exibir credenciais, apenas mensagem amigável
            const QString err = reply->errorString();
            QMessageBox::warning(this, tr("Falha no envio"), tr("Não foi possível enviar os dados para o PHPList. Detalhes: %1").arg(err));
            return;
        }
        const QByteArray body = reply->readAll();
        // Supondo JSON de resposta; não fazemos parsing rígido para evitar acoplamento
        Q_UNUSED(body);
        QMessageBox::information(this, tr("Sucesso"), tr("Dados enviados com sucesso para a lista."));
    });
}

void MainWindow::saveChatForCurrentFile() {
    if (currentFilePath_.isEmpty()) return;
    if (!chatDock_) return;
    const QString key = QString("files/%1/chatHtml").arg(currentFilePath_);
    settings_.setValue(key, chatDock_->transcriptHtml());
    // Save structured conversation for LLM context (role/content list)
    const auto msgs = chatDock_->conversationForLlm();
    QJsonArray arr;
    for (const auto& rc : msgs) {
        QJsonObject o; o.insert("role", rc.first); o.insert("content", rc.second); arr.append(o);
    }
    const QString keyMsgs = QString("files/%1/chatMsgs").arg(currentFilePath_);
    settings_.setValue(keyMsgs, QJsonDocument(arr).toJson(QJsonDocument::Compact));
}

void MainWindow::loadChatForFile(const QString& filePath) {
    if (filePath.isEmpty()) return;
    if (!chatDock_) return;
    const QString key = QString("files/%1/chatHtml").arg(filePath);
    const QString html = settings_.value(key).toString();
    if (!html.isEmpty()) chatDock_->setTranscriptHtml(html);
    // Load structured conversation for LLM context
    const QString keyMsgs = QString("files/%1/chatMsgs").arg(filePath);
    const QString raw = settings_.value(keyMsgs).toString();
    if (!raw.trimmed().isEmpty()) {
        const QJsonDocument doc = QJsonDocument::fromJson(raw.toUtf8());
        if (doc.isArray()) {
            QList<QPair<QString, QString>> msgs;
            for (const auto& v : doc.array()) {
                const auto o = v.toObject();
                msgs.append(QPair<QString,QString>(o.value("role").toString(), o.value("content").toString()));
            }
            chatDock_->setConversationForLlm(msgs);
        }
    }
}

void MainWindow::editReaderData() {
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Dados do leitor"));

    auto* form = new QFormLayout(&dlg);
    auto* nameEdit = new QLineEdit(&dlg);
    auto* emailEdit = new QLineEdit(&dlg);
    auto* whatsappEdit = new QLineEdit(&dlg);

    // Load existing values from settings
    nameEdit->setText(settings_.value("reader/name").toString());
    emailEdit->setText(settings_.value("reader/email").toString());
    whatsappEdit->setText(settings_.value("reader/whatsapp").toString());

    auto* info = new QLabel(&dlg);
    info->setWordWrap(true);
    info->setText(tr("Os dados do leitor (Nome completo, E-mail e WhatsApp) serão usados nas anotações e para personalizar a interação com o ChatGPT.\n\nA integração com o PHPList utiliza configurações no arquivo .env localizado ao lado do executável ou no diretório de onde o aplicativo foi chamado. As credenciais não são exibidas aqui."));

    form->addRow(info);
    form->addRow(tr("Nome completo"), nameEdit);
    form->addRow(tr("E-mail"), emailEdit);
    form->addRow(tr("WhatsApp"), whatsappEdit);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    form->addRow(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        // Save settings
        settings_.setValue("reader/name", nameEdit->text());
        settings_.setValue("reader/email", emailEdit->text());
        settings_.setValue("reader/whatsapp", whatsappEdit->text());

        // Validate and optionally submit to PHPList
        QString err;
        if (!validateReaderInputs(nameEdit->text().trimmed(), emailEdit->text().trimmed(), &err)) {
            QMessageBox::warning(this, tr("Dados inválidos"), err);
            return;
        }
        const auto choice = QMessageBox::question(
            this,
            tr("Confirmar envio"),
            tr("Enviar meus dados para a lista ÁrvoreDosSaberes?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );
        if (choice == QMessageBox::Yes) {
            submitReaderDataToPhpList(nameEdit->text().trimmed(), emailEdit->text().trimmed(), whatsappEdit->text().trimmed());
        }
    }
}

void MainWindow::saveAs() {
    if (currentFilePath_.isEmpty() || !QFileInfo::exists(currentFilePath_)) {
        QMessageBox::information(this, tr("Salvar como"), tr("Nenhum arquivo aberto para salvar."));
        return;
    }
    const QString startDir = settings_.value("session/lastDir", QFileInfo(currentFilePath_).absolutePath()).toString();
    const QString suggested = QFileInfo(currentFilePath_).fileName();
    const QString target = QFileDialog::getSaveFileName(this, tr("Salvar como"), QDir(startDir).filePath(suggested), tr("E-books (*.pdf *.epub);;Todos (*.*)"));
    if (target.isEmpty()) return;

    if (QFileInfo::exists(target)) {
        const auto ret = QMessageBox::question(this, tr("Confirmar"), tr("O arquivo já existe. Deseja sobrescrever?"),
                                               QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ret != QMessageBox::Yes) return;
        if (!QFile::remove(target)) {
            QMessageBox::warning(this, tr("Erro"), tr("Não foi possível remover o arquivo de destino."));
            return;
        }
    }

    if (!QFile::copy(currentFilePath_, target)) {
        QMessageBox::warning(this, tr("Erro"), tr("Falha ao copiar para o destino."));
        return;
    }

    // Atualiza estado para o novo caminho (comportamento comum de 'Salvar como')
    currentFilePath_ = QFileInfo(target).absoluteFilePath();
    settings_.setValue("session/lastDir", QFileInfo(target).absolutePath());
    settings_.setValue("session/lastFile", currentFilePath_);
    statusBar()->showMessage(tr("Salvo em %1").arg(currentFilePath_), 3000);
}

MainWindow::~MainWindow() {
    // Persist chat transcript for current file before closing
    saveChatForCurrentFile();
    saveSettings();
}

void MainWindow::buildUi() {
    viewer_ = new ViewerWidget(this);

    // TOC panel with toolbar on top and tree below
    tocPanel_ = new QWidget(this);
    tocPanel_->setMinimumWidth(140); // ensure TOC toolbar/buttons remain visible
    auto* tocLayout = new QVBoxLayout(tocPanel_);
    tocLayout->setContentsMargins(0,0,0,0);
    tocLayout->setSpacing(0);

    tocToolBar_ = new QToolBar(tr("TOC"), tocPanel_);
    tocToolBar_->setIconSize(QSize(16,16));
    // Show text so buttons are visible even without icons
    tocToolBar_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    tocLayout->addWidget(tocToolBar_);

    toc_ = new QTreeWidget(tocPanel_);
    toc_->setHeaderHidden(true);
    tocLayout->addWidget(toc_);

    splitter_ = new QSplitter(this);
    splitter_->addWidget(tocPanel_);
    splitter_->addWidget(viewer_);
    // Ensure viewer occupies remaining space
    splitter_->setStretchFactor(0, 0);
    splitter_->setStretchFactor(1, 1);
    splitter_->setContentsMargins(0,0,0,0);
    // Avoid collapsing the TOC completely
    splitter_->setCollapsible(0, false);

    setCentralWidget(splitter_);
    statusBar();
    applyDefaultSplitterSizesIfNeeded();

    // Create and show chat dock by default
    chatDock_ = new ChatDock(this);
    chatDock_->setAllowedAreas(Qt::RightDockWidgetArea);
    chatDock_->setFeatures(QDockWidget::NoDockWidgetFeatures);
    addDockWidget(Qt::RightDockWidgetArea, chatDock_);
    connect(chatDock_, &ChatDock::sendMessageRequested, this, &MainWindow::onChatSendMessage);
    connect(chatDock_, &ChatDock::saveTranscriptRequested, this, &MainWindow::onChatSaveTranscript);
    connect(chatDock_, &ChatDock::summarizeTranscriptRequested, this, &MainWindow::onChatSummarizeTranscript);
    connect(chatDock_, &ChatDock::conversationCleared, this, [this](const QString& title, const QString& html, const QList<QPair<QString,QString>>& msgs){
        if (currentFilePath_.isEmpty()) return;
        // Append to sessions list in QSettings as JSON array
        QJsonArray sessions = readChatSessions(currentFilePath_);
        QJsonObject obj; obj["title"] = title; obj["html"] = html;
        QJsonArray arrMsgs; for (const auto& rc : msgs) { QJsonObject o; o["role"] = rc.first; o["content"] = rc.second; arrMsgs.append(o);} obj["msgs"] = arrMsgs;
        sessions.append(obj);
        writeChatSessions(currentFilePath_, sessions);
    });
    connect(chatDock_, &ChatDock::requestShowSavedChats, this, [this]{ showSavedChatsPicker(); });
    chatDock_->show();
    chatDock_->raise();
}

void MainWindow::showChatPanel() {
    if (chatDock_) {
        chatDock_->show();
        chatDock_->raise();
    }
    if (!currentFilePath_.isEmpty()) {
        loadChatForFile(currentFilePath_);
    }
}
void MainWindow::loadSettings() {
    restoreGeometry(settings_.value("ui/geometry").toByteArray());
    restoreState(settings_.value("ui/state").toByteArray());
    darkTheme_ = settings_.value("ui/dark", false).toBool();
    // apply zoom if viewer supports it (initial viewer_ is ViewerWidget)
    if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) {
        vw->setZoomFactor(settings_.value("view/zoom", 1.0).toDouble());
    }
    if (darkTheme_) applyDarkPalette(true);

    // Optional: reopen last file on start (RF-20)
    const QString lastFile = settings_.value("session/lastFile").toString();
    if (!lastFile.isEmpty() && QFileInfo::exists(lastFile)) {
        openPath(lastFile);
    }
}

void MainWindow::setWheelZoomPreference() {
    const double current = settings_.value("view/wheelZoomStep", 1.1).toDouble();
    bool ok = false;
    const double val = QInputDialog::getDouble(
        this,
        tr("Granularidade do zoom"),
        tr("Fator por passo (1.01 a 1.50):"),
        current,
        1.01,
        1.50,
        2,
        &ok
    );
    if (!ok) return;
    settings_.setValue("view/wheelZoomStep", val);
    // apply immediately if viewer is PdfViewerWidget
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
        pv->setWheelZoomStep(val);
    }
}

void MainWindow::openLlmSettings() {
    LlmSettingsDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        if (llm_) llm_->reloadSettings();
        statusBar()->showMessage(tr("Configurações de LLM atualizadas."), 2000);
    }
}

void MainWindow::openDictionarySettings() {
    DictionarySettingsDialog dlg(this);
    dlg.exec();
}

void MainWindow::openEmbeddingSettings() {
    EmbeddingSettingsDialog dlg(this);
    // Conecta ação de recriação de embeddings do documento atual
    connect(&dlg, &EmbeddingSettingsDialog::rebuildRequested, this, [this]() {
        QSettings s;
        const QString dbPath = s.value("emb/db_path").toString();
        QDir dir(dbPath);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        if (currentFilePath_.isEmpty()) {
            QMessageBox::information(this, tr("Recriar Embeddings"), tr("Abra um documento para recriar os embeddings."));
            return;
        }
        QMessageBox::information(
            this,
            tr("Recriar Embeddings"),
            tr("Recriação de embeddings agendada para:\n%1\nBanco: %2")
                .arg(QFileInfo(currentFilePath_).fileName(), dbPath)
        );
        // TODO: implementar pipeline de indexação RAG (chunking -> embeddings -> ChromaDB)
    });
    dlg.exec();
}

void MainWindow::saveSettings() {
    settings_.setValue("ui/geometry", saveGeometry());
    settings_.setValue("ui/state", saveState());
    settings_.setValue("ui/dark", darkTheme_);
    double z = 1.0;
    if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) {
        z = vw->zoomFactor();
    }
#ifdef HAVE_QT_PDF
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
        z = pv->zoomFactor();
    }
#endif
    settings_.setValue("view/zoom", z);
}

void MainWindow::showLongAlert(const QString& title, const QString& longText) {
    QDialog dlg(this);
    dlg.setWindowTitle(title);
    dlg.resize(700, 500);
    auto* lay = new QVBoxLayout(&dlg);
    auto* txt = new QPlainTextEdit(&dlg);
    txt->setReadOnly(true);
    txt->setPlainText(longText);
    lay->addWidget(txt, 1);
    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok, &dlg);
    lay->addWidget(btns);
    connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    dlg.exec();
}

void MainWindow::updateStatus() {
    unsigned int cur = 1, tot = 0; double z = 1.0;
    if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) {
        cur = vw->currentPage() == 0 ? 1u : vw->currentPage();
        tot = vw->totalPages();
        z = vw->zoomFactor();
    }
#ifdef HAVE_QT_PDF
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
        cur = pv->currentPage() == 0 ? 1u : pv->currentPage();
        tot = pv->totalPages();
        z = pv->zoomFactor();
    }
#endif
    // Persist per-file reading position and zoom whenever we refresh status
    if (!currentFilePath_.isEmpty()) {
        settings_.setValue(QString("files/%1/page").arg(currentFilePath_), static_cast<uint>(cur));
        settings_.setValue(QString("files/%1/zoom").arg(currentFilePath_), z);
    }
    statusBar()->showMessage(tr("Página %1 de %2 | Zoom %3%")
        .arg(cur)
        .arg(tot == 0 ? 100 : tot)
        .arg(int(z*100)));
    updatePageCombo();
}

void MainWindow::applyDarkPalette(bool enable) {
    QPalette pal;
    if (enable) {
        pal.setColor(QPalette::Window, QColor(37,37,38));
        pal.setColor(QPalette::WindowText, Qt::white);
        pal.setColor(QPalette::Base, QColor(30,30,30));
        pal.setColor(QPalette::AlternateBase, QColor(45,45,48));
        pal.setColor(QPalette::ToolTipBase, Qt::white);
        pal.setColor(QPalette::ToolTipText, Qt::white);
        pal.setColor(QPalette::Text, Qt::white);
        pal.setColor(QPalette::Button, QColor(45,45,48));
        pal.setColor(QPalette::ButtonText, Qt::white);
        pal.setColor(QPalette::BrightText, Qt::red);
        pal.setColor(QPalette::Highlight, QColor(38,79,120));
        pal.setColor(QPalette::HighlightedText, Qt::white);
    }
    QApplication::setPalette(pal);
}

void MainWindow::openFile() {
    const QString startDir = settings_.value("session/lastDir", QDir::homePath()).toString();
    const QString file = QFileDialog::getOpenFileName(this, tr("Abrir e-book"), startDir, tr("E-books (*.pdf *.epub);;Todos (*.*)"));
    if (file.isEmpty()) return;
    openPath(file);
}

bool MainWindow::openPath(const QString& file) {
    const QFileInfo fi(file);

#ifdef HAVE_QT_PDF
    if (fi.suffix().compare("pdf", Qt::CaseInsensitive) == 0) {
        // Replace current viewer with a PdfViewerWidget
        auto* newViewer = new PdfViewerWidget(this);
        // Apply wheel zoom step preference
        newViewer->setWheelZoomStep(settings_.value("view/wheelZoomStep", 1.1).toDouble());
        QString err;
        if (!newViewer->openFile(file, &err)) {
            delete newViewer;
            QMessageBox::warning(this, tr("Erro"), err.isEmpty() ? tr("Falha ao abrir PDF.") : err);
            return false;
        }
        // Wire signals for AI actions from the PDF viewer
        connect(newViewer, &PdfViewerWidget::requestSendToChat, this, &MainWindow::onRequestSendToChat);
        connect(newViewer, &PdfViewerWidget::requestSendImageToChat, this, &MainWindow::onRequestSendImageToChat);
        // Replace current viewer in the splitter
        const int idx = splitter_->indexOf(viewer_);
        splitter_->replaceWidget(idx, newViewer);
        viewer_->deleteLater();
        viewer_ = newViewer;

        // Connect context-menu AI actions from the PDF viewer to MainWindow slots
        connect(newViewer, &PdfViewerWidget::requestSynonyms, this, &MainWindow::onRequestSynonyms);
        connect(newViewer, &PdfViewerWidget::requestSummarize, this, &MainWindow::onRequestSummarize);
        connect(newViewer, &PdfViewerWidget::requestSendImageToChat, this, &MainWindow::onRequestSendImageToChat);
        connect(newViewer, &PdfViewerWidget::requestDictionaryLookup, this, &MainWindow::onDictionaryLookup);
        connect(newViewer, &PdfViewerWidget::requestRebuildEmbeddings, this, &MainWindow::onRequestRebuildEmbeddings);
        // Keep status updated (and persistence) when user scrolls/zooms in the PDF viewer
        connect(newViewer, &PdfViewerWidget::scrollChanged, this, &MainWindow::updateStatus);
        connect(newViewer, &PdfViewerWidget::zoomFactorChanged, this, &MainWindow::updateStatus);

        // Build TOC according to current mode
        if (tocPagesMode_) setTocModePages(); else setTocModeChapters();
        // Apply persisted zoom: per-file if available, else global default; fall back to fit-to-width
        const QString absPath = fi.absoluteFilePath();
        double fileZoom = settings_.value(QString("files/%1/zoom").arg(absPath),
                             settings_.value("view/zoom", 0.0)).toDouble();
        if (fileZoom > 0.0) {
            newViewer->setZoomFactor(fileZoom);
        } else {
            // Fit to width initially for natural reading
            newViewer->fitToWidth();
        }
        // Restore last-read page for this file if available
        {
            bool ok = false;
            const QVariant v = settings_.value(QString("files/%1/page").arg(absPath));
            unsigned int lastPage = v.toUInt(&ok);
            if (ok && lastPage > 0) {
                const unsigned int total = newViewer->totalPages();
                if (lastPage >= 1 && lastPage <= total) {
                    newViewer->setCurrentPage(lastPage);
                }
            }
        }
        // Save last dir/file
        settings_.setValue("session/lastDir", fi.absolutePath());
        settings_.setValue("session/lastFile", fi.absoluteFilePath());
        currentFilePath_ = fi.absoluteFilePath();

        // Track in recent files
        addRecentFile(currentFilePath_);

        // Load chat history for this file
        loadChatForFile(currentFilePath_);

        // Update window title with metadata Title when available (Qt6+), else filename
        QString titleText = fi.completeBaseName();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        if (auto* doc = newViewer->document()) {
            // Some Qt6 versions provide MetaDataField::Title
            QString metaTitle;
#ifdef Q_OS_WIN
            metaTitle = doc->metaData(QPdfDocument::MetaDataField::Title).toString();
#else
            metaTitle = doc->metaData(QPdfDocument::MetaDataField::Title).toString();
#endif
            if (!metaTitle.trimmed().isEmpty()) titleText = metaTitle.trimmed();
        }
#endif
        setWindowTitle(QString("%1 v%2 — %3")
                           .arg(genai::AppInfo::Name)
                           .arg(genai::AppInfo::Version)
                           .arg(titleText));
        updateTitleWidget();
        updateStatus();
        actClose_->setEnabled(true);
        return true;
    }
#endif

    // Sem suporte a outros formatos nesta compilação obrigatória de Qt PDF
    QMessageBox::warning(this, tr("Formato não suportado"), tr("Apenas arquivos PDF são suportados nesta versão."));
    return false;
}

void MainWindow::openRecentFile() {
    QString path;
    if (auto act = qobject_cast<QAction*>(sender())) {
        path = act->data().toString();
    }
    if (path.isEmpty()) {
        showRecentDialog();
        return;
    }
    if (!QFileInfo::exists(path)) {
        QMessageBox::warning(this, tr("Arquivo ausente"), tr("O arquivo não existe mais: %1").arg(path));
        return;
    }
    openPath(path);
}

void MainWindow::showRecentDialog() {
    QVariantList entries = loadRecentEntries(settings_);
    if (entries.isEmpty()) {
        QMessageBox::information(this, tr("Recentes"), tr("Nenhum arquivo recente."));
        return;
    }

    QDialog dlg(this);
    dlg.setWindowTitle(tr("Abrir recente"));
    auto* lay = new QVBoxLayout(&dlg);

    auto* searchEdit = new QLineEdit(&dlg);
    searchEdit->setPlaceholderText(tr("Pesquisar por nome, título, autor, editora, ISBN, palavras-chave..."));
    lay->addWidget(searchEdit);

    auto* tree = new QTreeWidget(&dlg);
    tree->setColumnCount(2);
    QStringList headers; headers << tr("Título") << tr("Arquivo");
    tree->setHeaderLabels(headers);
    tree->setRootIsDecorated(false);
    tree->setAlternatingRowColors(true);
    lay->addWidget(tree, 1);

    auto populate = [&](const QString& filter){
        tree->clear();
        for (const QVariant& v : entries) {
            const QVariantMap e = v.toMap();
            if (!entryMatches(e, filter)) continue;
            auto* it = new QTreeWidgetItem(tree);
            const QString title = e.value("title").toString();
            const QString path = e.value("path").toString();
            it->setText(0, title.isEmpty() ? QFileInfo(path).completeBaseName() : title);
            it->setText(1, QFileInfo(path).fileName());
            it->setToolTip(0, path);
            it->setToolTip(1, path);
            it->setData(0, Qt::UserRole, path);
        }
        for (int c = 0; c < tree->columnCount(); ++c) tree->resizeColumnToContents(c);
    };
    populate(QString());

    QObject::connect(searchEdit, &QLineEdit::textChanged, &dlg, [populate](const QString& t){ populate(t); });
    QObject::connect(tree, &QTreeWidget::itemDoubleClicked, &dlg, [&dlg](QTreeWidgetItem* it, int){
        dlg.setProperty("selectedPath", it->data(0, Qt::UserRole));
        dlg.accept();
    });

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Open | QDialogButtonBox::Cancel, &dlg);
    lay->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&](){
        auto* it = tree->currentItem();
        if (!it) { dlg.reject(); return; }
        dlg.setProperty("selectedPath", it->data(0, Qt::UserRole));
        dlg.accept();
    });
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        const QString choice = dlg.property("selectedPath").toString();
        if (!choice.isEmpty()) openPath(choice);
    }
}

void MainWindow::configureRecentDialogCount() {
    const int current = settings_.value("recent/maxCount", MaxRecentMenuItems).toInt();
    bool ok = false;
    const int val = QInputDialog::getInt(
        this,
        tr("Configurar recentes"),
        tr("Quantidade máxima de itens:"),
        current,
        1,
        20,
        1,
        &ok
    );
    if (!ok) return;
    settings_.setValue("recent/maxCount", val);
    rebuildRecentMenu();
}

void MainWindow::addRecentFile(const QString& absPath) {
    if (absPath.isEmpty()) return;
    QVariantList entries = loadRecentEntries(settings_);
    // Remove existing entry for this path
    for (int i = entries.size() - 1; i >= 0; --i) {
        if (entries[i].toMap().value("path").toString() == absPath) entries.removeAt(i);
    }
    // Prepend fresh metadata
    QVariantMap entry = extractPdfMeta(absPath);
    entries.prepend(entry);
    const int maxCount = settings_.value("recent/maxCount", MaxRecentMenuItems).toInt();
    while (entries.size() > maxCount) entries.removeLast();
    saveRecentEntries(settings_, entries);
    rebuildRecentMenu();
}

void MainWindow::rebuildRecentMenu() {
    if (!menuRecent_) return;
    const QVariantList entries = loadRecentEntries(settings_);
    int i = 0;
    for (; i < MaxRecentMenuItems; ++i) {
        if (!recentActs_[i]) continue;
        if (i < entries.size()) {
            const QVariantMap e = entries.at(i).toMap();
            const QString path = e.value("path").toString();
            const QString title = e.value("title").toString();
            const QString label = title.isEmpty() ? QFileInfo(path).fileName() : QString("%1 — %2").arg(title, QFileInfo(path).fileName());
            recentActs_[i]->setText(label);
            recentActs_[i]->setData(path);
            recentActs_[i]->setToolTip(path);
            recentActs_[i]->setVisible(true);
        } else {
            recentActs_[i]->setVisible(false);
            recentActs_[i]->setData(QVariant());
        }
    }
}

void MainWindow::closeDocument() {
    // Clear TOC
    toc_->clear();
    // Save chat for current file before clearing
    saveChatForCurrentFile();
    // Replace current viewer with a fresh ViewerWidget
    QWidget* newViewer = new ViewerWidget(this);
    int idx = splitter_->indexOf(viewer_);
    splitter_->replaceWidget(idx, newViewer);
    viewer_->deleteLater();
    viewer_ = newViewer;
    // Reset state
    currentFilePath_.clear();
    settings_.remove("session/lastFile");
    // Update UI state
    updateStatus();
    actClose_->setEnabled(false);
    setWindowTitle(QString("%1 v%2 — Leitor")
                       .arg(genai::AppInfo::Name)
                       .arg(genai::AppInfo::Version));
    updateTitleWidget();
}

void MainWindow::nextPage() {
    unsigned int cur = 1, tot = 0;
    if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) {
        cur = vw->currentPage(); tot = vw->totalPages();
        if (cur < tot) vw->setCurrentPage(cur + 1);
    }
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
        cur = pv->currentPage(); tot = pv->totalPages();
        if (cur < tot) pv->setCurrentPage(cur + 1);
    }
    updateStatus();
}

void MainWindow::prevPage() {
    if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) {
        if (vw->currentPage() > 1) vw->setCurrentPage(vw->currentPage() - 1);
    }
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
        if (pv->currentPage() > 1) pv->setCurrentPage(pv->currentPage() - 1);
    }
    updateStatus();
}

void MainWindow::zoomIn() {
    if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) {
        vw->setZoomFactor(vw->zoomFactor() * 1.1);
    }
#ifdef HAVE_QT_PDF
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
        pv->setZoomFactor(pv->zoomFactor() * 1.1);
    }
#endif
    updateStatus();
}

void MainWindow::zoomOut() {
    if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) {
        vw->setZoomFactor(vw->zoomFactor() / 1.1);
    }
#ifdef HAVE_QT_PDF
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
        pv->setZoomFactor(pv->zoomFactor() / 1.1);
    }
#endif
    updateStatus();
}

void MainWindow::zoomReset() {
    if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) {
        vw->setZoomFactor(1.0);
    }
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
        pv->setZoomFactor(1.0);
    }
    updateStatus();
}

void MainWindow::toggleTheme() {
    darkTheme_ = !darkTheme_;
    applyDarkPalette(darkTheme_);
}

void MainWindow::onTocItemActivated(QTreeWidgetItem* item, int) {
    bool ok = false;
    const unsigned int page = item->data(0, Qt::UserRole).toUInt(&ok);
    if (ok) {
        if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) {
            vw->setCurrentPage(page);
        }
        if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
            pv->setCurrentPage(page);
        }
        updateStatus();
    }
}

void MainWindow::updatePageCombo() {
    if (!pageCombo_) return;
    unsigned int cur = 1, tot = 0;
    if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) { cur = vw->currentPage(); tot = vw->totalPages(); }
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) { cur = pv->currentPage(); tot = pv->totalPages(); }
    const unsigned int total = tot == 0 ? 100u : tot;
    // Update items only if count mismatches to avoid flicker
    if (pageCombo_->count() != int(total)) {
        pageCombo_->blockSignals(true);
        pageCombo_->clear();
        for (unsigned int i=1; i<=total; ++i) {
            pageCombo_->addItem(QString::number(i));
        }
        pageCombo_->blockSignals(false);
    }
}
