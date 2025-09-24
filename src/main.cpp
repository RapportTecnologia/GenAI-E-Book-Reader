#include <QApplication>
#include <QCoreApplication>
#include <QSplashScreen>
#include <QPixmap>
#include <QIcon>
#include <QTimer>
#include <QEvent>
#include <QMouseEvent>
#include <QSettings>
#include "ui/MainWindow.h"
#include "ui/WelcomeDialog.h"
#include "app/App.h"

// Added for update check and download flow
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFileDialog>
#include <QProgressDialog>
#include <QFile>
#include <QStandardPaths>
#include <QProcess>
#include <QSysInfo>
#include <QRegularExpression>
#include <QMessageBox>
#include <QDir>
#include <functional>
#include <QFont>
#include <QSize>

int main(int argc, char* argv[]) {
    QCoreApplication::setOrganizationDomain("br.com.rapport.genai-reader");
    QCoreApplication::setOrganizationName("br.com.rapport.genai-reader");
    QCoreApplication::setApplicationName("genai-reader");
    QApplication app(argc, argv);
    // Set global application icon
    app.setWindowIcon(QIcon(":/app/logo.png"));

    QSettings settings;
    bool firstRun = settings.value("firstRun", true).toBool();
    // Defer any user interaction until after the splash screen and main window are shown

    // Main window first so we can size splash relative to it
    MainWindow win;

    // Splash screen scaled to 60% of main window size
    QPixmap pm(":/app/logo.png");
    const QSize target = QSize(int(win.size().width() * 0.6), int(win.size().height() * 0.6));
    QPixmap scaledPm = pm.scaled(target, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QSplashScreen splash(scaledPm);
    QFont splashFont = splash.font();
    splashFont.setBold(true);
    splash.setFont(splashFont);
    splash.showMessage(QString("%1 v%2<br><br>https://rapport.tec.br")
                           .arg(genai::AppInfo::Name)
                           .arg(genai::AppInfo::Version),
                       Qt::AlignBottom | Qt::AlignHCenter, Qt::black);
    splash.show();


    win.setWindowTitle(QString("%1 v%2 — Leitor")
                           .arg(genai::AppInfo::Name)
                           .arg(genai::AppInfo::Version));
    win.resize(1024, 768);

    app.processEvents();

    // Helper: compare semantic-like versions (e.g., 0.1.8 vs 0.1.7). Returns +1 if a>b, 0 if equal, -1 if a<b
    auto compareVersions = [](const QString& a, const QString& b)->int {
        auto splitNums = [](const QString& v){
            QString s = v.trimmed();
            // Strip leading 'v' if present
            if (s.startsWith('v') || s.startsWith('V')) s = s.mid(1);
            // Keep only digits and dots prefix
            QRegularExpression re("^([0-9]+(?:\\.[0-9]+)*)");
            auto m = re.match(s);
            if (m.hasMatch()) s = m.captured(1);
            QStringList parts = s.split('.', Qt::SkipEmptyParts);
            QList<int> nums; nums.reserve(parts.size());
            for (const QString& p : parts) nums.push_back(p.toInt());
            return nums;
        };
        QList<int> A = splitNums(a), B = splitNums(b);
        const int n = qMax(A.size(), B.size());
        A.resize(n); B.resize(n);
        for (int i=0;i<n;++i) {
            if (A[i] > B[i]) return +1;
            if (A[i] < B[i]) return -1;
        }
        return 0;
    };

    // Helper: determine preferred asset suffix for this OS
    auto preferredAssetSuffixes = []()->QStringList{
#if defined(Q_OS_WIN)
        return { ".exe", ".msi" };
#elif defined(Q_OS_MAC)
        return { ".dmg", ".pkg", ".zip" };
#else
        // Linux
        return { ".AppImage", ".tar.gz" };
#endif
    };

    // Post-splash checks: update and file association, then continue
    std::function<void(std::function<void()>)> runPostSplashChecks;
    runPostSplashChecks = [&](std::function<void()> cont){
        // Avoid re-asking for the same latest version if user skipped recently
        const QString skipVersion = settings.value("update/skip_version").toString();

        // Network manager owned by a temporary QObject to auto-delete after finish
        QNetworkAccessManager* nm = new QNetworkAccessManager(&win);
        QObject::connect(nm, &QNetworkAccessManager::finished, nm, &QObject::deleteLater);

        // Query GitHub API for latest release
        const QUrl apiUrl("https://api.github.com/repos/RapportTecnologia/GenAi-E-Book-Reader/releases/latest");
        QNetworkRequest req(apiUrl);
        req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("%1/%2").arg(genai::AppInfo::Name, genai::AppInfo::Version));
        QNetworkReply* rep = nm->get(req);

        QObject::connect(rep, &QNetworkReply::finished, &win, [&, rep, cont, compareVersions, skipVersion]{
            rep->deleteLater();
            QString latestVersion;
            QString latestNotes;
            QUrl assetUrl;
            QString assetName;
            bool hasNewer = false;
            if (rep->error() == QNetworkReply::NoError) {
                const QByteArray body = rep->readAll();
                QJsonParseError je; QJsonDocument jd = QJsonDocument::fromJson(body, &je);
                if (je.error == QJsonParseError::NoError && jd.isObject()) {
                    const QJsonObject obj = jd.object();
                    latestVersion = obj.value("tag_name").toString();
                    if (latestVersion.isEmpty()) latestVersion = obj.value("name").toString();
                    latestNotes = obj.value("body").toString();
                    if (!latestVersion.isEmpty()) {
                        const int cmp = compareVersions(latestVersion, QString::fromUtf8(genai::AppInfo::Version));
                        hasNewer = (cmp > 0) && (latestVersion != skipVersion);
                    }
                    // Find suitable asset
                    const QJsonArray assets = obj.value("assets").toArray();
                    const QStringList prefs = preferredAssetSuffixes();
                    for (const auto& aVal : assets) {
                        const QJsonObject a = aVal.toObject();
                        const QString name = a.value("name").toString();
                        const QString url = a.value("browser_download_url").toString();
                        for (const QString& suf : prefs) {
                            if (name.endsWith(suf)) { assetUrl = QUrl(url); assetName = name; break; }
                        }
                        if (!assetUrl.isEmpty()) break;
                    }
                }
            }

            auto proceedToAssociation = [&, cont](){
#if defined(Q_OS_LINUX)
                // Ask to associate PDFs
                const bool assocAsked = settings.value("mime/asked_pdf", false).toBool();
                if (!assocAsked) {
                    settings.setValue("mime/asked_pdf", true);
                    const QMessageBox::StandardButton r = QMessageBox::question(&win,
                        QObject::tr("Associar PDFs?"),
                        QObject::tr("Deseja associar arquivos PDF ao GER - GenAI E-Book Reader?\nIsso permitirá abrir PDFs com duplo clique."),
                        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                    if (r == QMessageBox::Yes) {
                        // Try to set default application for PDFs
                        QProcess::execute("xdg-mime", {"default", "genai-reader.desktop", "application/pdf"});
                        QProcess::execute("xdg-mime", {"default", "genai-reader.desktop", "application/x-pdf"});
                        // Optionally update desktop database
                        QProcess::execute("update-desktop-database");
                    }
                }
#endif
                cont();
            };

            if (!hasNewer) {
                proceedToAssociation();
                return;
            }

            // Ask user whether to download the new tool/version
            QString msg = QObject::tr("Uma nova versão está disponível: %1 (atual: %2).\nDeseja fazer o download?")
                              .arg(latestVersion, QString::fromUtf8(genai::AppInfo::Version));
            if (!latestNotes.trimmed().isEmpty()) {
                msg += QObject::tr("\n\nNotas:\n%1").arg(latestNotes.left(800));
            }
            QMessageBox::StandardButton choice = QMessageBox::question(&win,
                QObject::tr("Nova versão disponível"), msg,
                QMessageBox::Yes | QMessageBox::No);
            if (choice == QMessageBox::No) {
                settings.setValue("update/skip_version", latestVersion);
                proceedToAssociation();
                return;
            }

            // Ask for target directory
            const QString defDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
            QString dir = QFileDialog::getExistingDirectory(&win, QObject::tr("Escolha a pasta para download"), defDir.isEmpty()? QDir::homePath() : defDir);
            if (dir.isEmpty()) { proceedToAssociation(); return; }

            if (assetUrl.isEmpty()) {
                QMessageBox::information(&win, QObject::tr("Download"), QObject::tr("Não foi encontrado um ativo compatível para esta plataforma."));
                proceedToAssociation();
                return;
            }

            const QString outPath = QDir(dir).filePath(assetName.isEmpty()? QString("GenAI-Reader-%1").arg(latestVersion) : assetName);

            // Download with progress
            QNetworkReply* dl = nm->get(QNetworkRequest(assetUrl));
            QFile* outFile = new QFile(outPath, &win);
            if (!outFile->open(QIODevice::WriteOnly)) {
                QMessageBox::warning(&win, QObject::tr("Download"), QObject::tr("Não foi possível escrever em %1").arg(outPath));
                outFile->deleteLater();
                proceedToAssociation();
                return;
            }

            QProgressDialog* pd = new QProgressDialog(QObject::tr("Baixando %1...").arg(assetName), QObject::tr("Cancelar"), 0, 100, &win);
            pd->setWindowModality(Qt::WindowModal);
            pd->setAutoClose(true);

            QObject::connect(pd, &QProgressDialog::canceled, dl, &QNetworkReply::abort);
            QObject::connect(dl, &QNetworkReply::downloadProgress, &win, [pd](qint64 rec, qint64 tot){
                if (tot > 0) pd->setValue(int((rec * 100) / tot));
            });
            QObject::connect(dl, &QIODevice::readyRead, &win, [dl, outFile]{ outFile->write(dl->readAll()); });
            QObject::connect(dl, &QNetworkReply::finished, &win, [&, dl, outFile, pd, proceedToAssociation]{
                outFile->write(dl->readAll());
                outFile->flush();
                outFile->close();
                const bool ok = (dl->error() == QNetworkReply::NoError);
                dl->deleteLater();
                if (pd) pd->close();
                if (ok) {
                    QMessageBox::information(&win, QObject::tr("Download"), QObject::tr("Arquivo salvo em:\n%1").arg(outFile->fileName()));
#if defined(Q_OS_LINUX)
                    // Optionally make executable if AppImage
                    if (outFile->fileName().endsWith(".AppImage")) {
                        QFile f(outFile->fileName());
                        f.setPermissions(f.permissions() | QFile::ExeOwner | QFile::ExeGroup | QFile::ExeOther);
                    }
#endif
                } else {
                    QMessageBox::warning(&win, QObject::tr("Download"), QObject::tr("Falha no download."));
                }
                outFile->deleteLater();
                proceedToAssociation();
            });
        });
    };

    // Ensure the main window is shown once (timer or click)
    bool shown = false;
    auto showMainOnce = [&]() {
        if (shown) return;
        shown = true;
        win.show();
        splash.finish(&win);
        // Run post-splash checks first, then proceed with normal flow
        runPostSplashChecks([&](){
            // If a file was passed on command line, open it now
            if (app.arguments().size() > 1) {
                win.openPath(app.arguments().at(1));
            }
            // After the main window is visible, handle first-run welcome dialog
            if (firstRun) {
                WelcomeDialog welcomeDialog;
                if (welcomeDialog.exec() == QDialog::Accepted) {
                    settings.setValue("firstRun", false);
                    // If no file was specified via CLI, prompt to open one now
                    if (app.arguments().size() <= 1) {
                        win.openFile();
                    }
                }
            }
        });
    };

    // Auto-close after 3 seconds
    QTimer::singleShot(3000, &app, showMainOnce);

    // Close on click
    class SplashClickFilter : public QObject {
    public:
        SplashClickFilter(QSplashScreen* s, std::function<void()> f)
            : splash_(s), func_(std::move(f)) {}
    protected:
        bool eventFilter(QObject* obj, QEvent* ev) override {
            if (obj == splash_ && ev->type() == QEvent::MouseButtonPress) {
                func_();
                return true; // consume the click
            }
            return QObject::eventFilter(obj, ev);
        }
    private:
        QSplashScreen* splash_;
        std::function<void()> func_;
    };

    SplashClickFilter filter(&splash, showMainOnce);
    splash.installEventFilter(&filter);

    return app.exec();
}
