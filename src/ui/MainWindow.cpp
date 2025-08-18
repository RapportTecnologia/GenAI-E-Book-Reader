#ifdef USE_QT
#include "ui/MainWindow.h"
#include "ui/ViewerWidget.h"
#ifdef HAVE_QT_PDF
#include "ui/PdfViewerWidget.h"
#endif

#include <QApplication>
#include <QCoreApplication>
#include <QAction>
#include <QToolBar>
#include <QMenuBar>
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
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QMap>
#include <QSizePolicy>
#include <QStyle>
#ifdef HAVE_QT_NETWORK
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#endif
#include <algorithm>
#include <functional>
#include <QModelIndex>
#include <QVariant>

#include "app/App.h"

#ifdef HAVE_QT_PDF
#if __has_include(<QPdfBookmarkModel>)
#include <QPdfBookmarkModel>
#define HAS_QPDF_BOOKMARK_MODEL 1
#endif
#include <QPdfDocument>
#if __has_include(<QPdfLinkDestination>)
#include <QPdfLinkDestination>
#define HAS_QPDF_LINK_DESTINATION 1
#endif
#endif

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), settings_("GenAI", "EBookReader") {
    buildUi();
    createActions();
    loadSettings();
    updateStatus();
#ifdef HAVE_QT_NETWORK
    netManager_ = new QNetworkAccessManager(this);
#endif
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
#ifdef HAVE_QT_PDF
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) { pages = pv->totalPages(); }
#endif
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
#ifdef HAVE_QT_PDF
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) { pages = pv->totalPages(); }

#ifdef HAS_QPDF_BOOKMARK_MODEL
    // Try to build TOC from real PDF bookmarks (titles) when available
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
        if (QPdfDocument* doc = pv->document()) {
            QPdfBookmarkModel bm(doc);
            if (bm.rowCount() > 0) {
                std::function<QTreeWidgetItem*(const QModelIndex&)> buildNode = [&](const QModelIndex& idx) -> QTreeWidgetItem* {
                    // Title
                    const QString title = bm.data(idx, Qt::DisplayRole).toString();
#ifdef HAS_QPDF_LINK_DESTINATION
                    // Destination -> page (0-based in Qt PDF); store 1-based for our navigation
                    const QVariant destVar = bm.data(idx, QPdfBookmarkModel::DestinationRole);
                    unsigned int page1 = 0u;
                    if (destVar.isValid()) {
                        const QPdfLinkDestination dest = destVar.value<QPdfLinkDestination>();
                        if (dest.isValid()) {
                            const int p0 = dest.page();
                            if (p0 >= 0) page1 = static_cast<unsigned int>(p0 + 1);
                        }
                    }
#endif
                    auto* item = new QTreeWidgetItem(QStringList{ title.isEmpty() ? tr("(sem título)") : title });
#ifdef HAS_QPDF_LINK_DESTINATION
                    if (page1 > 0) item->setData(0, Qt::UserRole, page1);
#endif
                    const int rows = bm.rowCount(idx);
                    for (int r = 0; r < rows; ++r) {
                        const QModelIndex childIdx = bm.index(r, 0, idx);
                        if (childIdx.isValid()) {
                            if (auto* child = buildNode(childIdx)) item->addChild(child);
                        }
                    }
                    // Fallback: if this node has no direct destination, use first child's destination
#ifdef HAS_QPDF_LINK_DESTINATION
                    if (!item->data(0, Qt::UserRole).isValid() && item->childCount() > 0) {
                        const QVariant firstChildPage = item->child(0)->data(0, Qt::UserRole);
                        if (firstChildPage.isValid()) item->setData(0, Qt::UserRole, firstChildPage);
                    }
#endif
                    return item;
                };

                // Build top-level items
                for (int r = 0; r < bm.rowCount(); ++r) {
                    const QModelIndex topIdx = bm.index(r, 0);
                    if (!topIdx.isValid()) continue;
                    if (auto* node = buildNode(topIdx)) toc_->addTopLevelItem(node);
                }

                // If at least one item has a valid page, keep this TOC; otherwise fallback
                bool hasNavigable = false;
                std::function<void(QTreeWidgetItem*)> scan = [&](QTreeWidgetItem* it){
                    if (!it || hasNavigable) return;
                    if (it->data(0, Qt::UserRole).isValid()) { hasNavigable = true; return; }
                    for (int i = 0; i < it->childCount() && !hasNavigable; ++i) scan(it->child(i));
                };
                for (int i = 0; i < toc_->topLevelItemCount() && !hasNavigable; ++i) scan(toc_->topLevelItem(i));
                if (hasNavigable && toc_->topLevelItemCount() > 0) { return; }
                // else: clear and continue to fallback numeric grouping
                toc_->clear();
            }
        }
    }
#endif // HAS_QPDF_BOOKMARK_MODEL
#endif // HAVE_QT_PDF

    // Fallback: placeholder grouping when no bookmarks are available
    if (pages == 0) return;
    const unsigned int group = 10;
    unsigned int ch = 1;
    for (unsigned int start = 1; start <= pages; start += group, ++ch) {
        auto* chap = new QTreeWidgetItem(QStringList{tr("Capítulo %1").arg(ch)});
        chap->setData(0, Qt::UserRole, start);
        for (unsigned int p = start; p < start + group && p <= pages; ++p) {
            auto* child = new QTreeWidgetItem(QStringList{tr("Página %1").arg(p)});
            child->setData(0, Qt::UserRole, p);
            chap->addChild(child);
        }
        toc_->addTopLevelItem(chap);
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

#endif // USE_QT

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
#ifndef HAVE_QT_NETWORK
    QMessageBox::information(this, tr("Indisponível"), tr("Suporte de rede não está disponível nesta compilação."));
    return;
#else
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
#endif
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
}

void MainWindow::createActions() {
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

    // TOC toolbar actions and wiring
    actTocModePages_ = new QAction(tr("Páginas"), this);
    actTocModeChapters_ = new QAction(tr("Conteúdo"), this);
    actTocPrev_ = new QAction(tr("Anterior"), this);
    actTocNext_ = new QAction(tr("Proxima"), this);
    // Provide standard navigation icons
    actTocPrev_->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
    actTocNext_->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));
    actTocModePages_->setCheckable(true);
    actTocModeChapters_->setCheckable(true);
    actTocModePages_->setChecked(true);
    auto* tocModeGroup = new QActionGroup(this);
    tocModeGroup->addAction(actTocModePages_);
    tocModeGroup->addAction(actTocModeChapters_);
    tocModeGroup->setExclusive(true);

    // Clarify behavior of main toolbar navigation based on mode
    actPrev_->setToolTip(tr("Voltar (página ou item do conteúdo, conforme modo)"));
    actNext_->setToolTip(tr("Avançar (página ou item do conteúdo, conforme modo)"));

    connect(actTocModePages_, &QAction::triggered, this, &MainWindow::setTocModePages);
    connect(actTocModeChapters_, &QAction::triggered, this, &MainWindow::setTocModeChapters);
    connect(actTocPrev_, &QAction::triggered, this, &MainWindow::onTocPrev);
    connect(actTocNext_, &QAction::triggered, this, &MainWindow::onTocNext);

    actOpen_->setShortcut(QKeySequence::Open);
    actSaveAs_->setShortcut(QKeySequence::SaveAs);
    actClose_->setShortcut(QKeySequence::Close); // Ctrl+W
    actQuit_->setShortcut(QKeySequence::Quit);   // Ctrl+Q
    actPrev_->setShortcut(Qt::Key_Left);
    actNext_->setShortcut(Qt::Key_Right);
    actZoomIn_->setShortcut(QKeySequence::ZoomIn);
    actZoomOut_->setShortcut(QKeySequence::ZoomOut);

    connect(actOpen_, &QAction::triggered, this, &MainWindow::openFile);
    connect(actSaveAs_, &QAction::triggered, this, &MainWindow::saveAs);
    // Mode-aware navigation: uses TOC handlers (pages vs capítulos)
    connect(actPrev_, &QAction::triggered, this, &MainWindow::onTocPrev);
    connect(actNext_, &QAction::triggered, this, &MainWindow::onTocNext);
    connect(actZoomIn_, &QAction::triggered, this, &MainWindow::zoomIn);
    connect(actZoomOut_, &QAction::triggered, this, &MainWindow::zoomOut);
    connect(actZoomReset_, &QAction::triggered, this, &MainWindow::zoomReset);
    connect(actToggleTheme_, &QAction::triggered, this, &MainWindow::toggleTheme);
    connect(actQuit_, &QAction::triggered, qApp, &QApplication::quit);
    connect(actReaderData_, &QAction::triggered, this, &MainWindow::editReaderData);
    connect(actClose_, &QAction::triggered, this, &MainWindow::closeDocument);

    // Menu Arquivo com submenus
    auto* menuArquivo = menuBar()->addMenu(tr("&Arquivo"));
    auto* menuDocumento = menuArquivo->addMenu(tr("Documento"));
    menuDocumento->addAction(actOpen_);
    menuDocumento->addAction(actSaveAs_);
    menuDocumento->addSeparator();
    menuDocumento->addAction(actClose_);
    auto* menuLeitor = menuArquivo->addMenu(tr("Leitor"));
    menuLeitor->addAction(actReaderData_);
    menuArquivo->addSeparator();
    menuArquivo->addAction(actQuit_); // Sair diretamente em Arquivo

    // Menu Configurações
    auto* menuConfig = menuBar()->addMenu(tr("Configurações"));
    menuConfig->addAction(actToggleTheme_);

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
#ifdef HAVE_QT_PDF
        if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) pv->setCurrentPage(p);
#endif
        updateStatus();
    });
    tb->addSeparator();
    tb->addAction(actZoomOut_);
    tb->addAction(actZoomIn_);
    tb->addAction(actZoomReset_);
    // Removido: alternância de tema na toolbar (foi para Configurações)

    QWidget* rightSpacer = new QWidget(tb);
    rightSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tb->addWidget(rightSpacer);

    // Add actions to the TOC toolbar now that it exists
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
        QString err;
        if (!newViewer->openFile(file, &err)) {
            delete newViewer;
            QMessageBox::warning(this, tr("Erro"), err.isEmpty() ? tr("Falha ao abrir PDF.") : err);
            return false;
        }

        // swap widget in splitter
        int idx = splitter_->indexOf(viewer_);
        splitter_->replaceWidget(idx, newViewer);
        viewer_->deleteLater();
        viewer_ = newViewer;

        // Build TOC according to current mode
        if (tocPagesMode_) setTocModePages(); else setTocModeChapters();
        // Fit to width initially (RF-13)
        // QPdfView supports FitToWidth via ZoomMode; expose through public API if needed.
        newViewer->setZoomFactor(1.0); // ensure valid
        // Save last dir/file
        settings_.setValue("session/lastDir", fi.absolutePath());
        settings_.setValue("session/lastFile", fi.absoluteFilePath());
        currentFilePath_ = fi.absoluteFilePath();

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
        updateStatus();
        actClose_->setEnabled(true);
        return true;
    }
#endif

    // Fallback to dummy reader and placeholder ViewerWidget
    auto res = reader_.open(file.toStdString());
    if (!res.ok) {
        QMessageBox::warning(this, tr("Erro"), QString::fromStdString(res.message));
        return false;
    }
    if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) {
        vw->setTotalPages(res.totalPages);
        vw->setCurrentPage(1);
    }
    // Build TOC according to current mode
    if (tocPagesMode_) setTocModePages(); else setTocModeChapters();
    settings_.setValue("session/lastDir", fi.absolutePath());
    settings_.setValue("session/lastFile", fi.absoluteFilePath());
    currentFilePath_ = fi.absoluteFilePath();
    setWindowTitle(QString("%1 v%2 — %3")
                       .arg(genai::AppInfo::Name)
                       .arg(genai::AppInfo::Version)
                       .arg(fi.completeBaseName()));
    updateStatus();
    actClose_->setEnabled(true);
    return true;
}

void MainWindow::closeDocument() {
    // Clear TOC
    toc_->clear();
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
}

void MainWindow::nextPage() {
    unsigned int cur = 1, tot = 0;
    if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) {
        cur = vw->currentPage(); tot = vw->totalPages();
        if (cur < tot) vw->setCurrentPage(cur + 1);
    }
#ifdef HAVE_QT_PDF
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
        cur = pv->currentPage(); tot = pv->totalPages();
        if (cur < tot) pv->setCurrentPage(cur + 1);
    }
#endif
    updateStatus();
}

void MainWindow::prevPage() {
    if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) {
        if (vw->currentPage() > 1) vw->setCurrentPage(vw->currentPage() - 1);
    }
#ifdef HAVE_QT_PDF
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
        if (pv->currentPage() > 1) pv->setCurrentPage(pv->currentPage() - 1);
    }
#endif
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
#ifdef HAVE_QT_PDF
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
        pv->setZoomFactor(1.0);
    }
#endif
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
#ifdef HAVE_QT_PDF
        if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) {
            pv->setCurrentPage(page);
        }
#endif
        updateStatus();
    }
}

void MainWindow::updatePageCombo() {
    if (!pageCombo_) return;
    unsigned int cur = 1, tot = 0;
    if (auto vw = qobject_cast<ViewerWidget*>(viewer_)) { cur = vw->currentPage(); tot = vw->totalPages(); }
#ifdef HAVE_QT_PDF
    if (auto pv = qobject_cast<PdfViewerWidget*>(viewer_)) { cur = pv->currentPage(); tot = pv->totalPages(); }
#endif
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
    const unsigned int curPage = (cur == 0 ? 1u : cur);
    const int desiredIndex = int(curPage) - 1;
    if (pageCombo_->currentIndex() != desiredIndex && desiredIndex >= 0 && desiredIndex < pageCombo_->count()) {
        pageCombo_->blockSignals(true);
        pageCombo_->setCurrentIndex(desiredIndex);
        pageCombo_->blockSignals(false);
    }
}
