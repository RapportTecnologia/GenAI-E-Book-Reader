#include <QApplication>
#include <QCoreApplication>
#include <QSplashScreen>
#include <QPixmap>
#include <QIcon>
#include <QTimer>
#include <QEvent>
#include <QMouseEvent>
#include "ui/MainWindow.h"
#include "app/App.h"

int main(int argc, char* argv[]) {
    QCoreApplication::setOrganizationDomain("br.com.rapport.genai-reader");
    QCoreApplication::setOrganizationName("br.com.rapport.genai-reader");
    QCoreApplication::setApplicationName("genai-reader");
    QApplication app(argc, argv);
    // Set global application icon
    app.setWindowIcon(QIcon(":/app/logo.png"));

    // Main window first so we can size splash relative to it
    MainWindow win;
    win.setWindowTitle(QString("%1 v%2 — Leitor")
                           .arg(genai::AppInfo::Name)
                           .arg(genai::AppInfo::Version));
    win.resize(1024, 768);

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
    app.processEvents();

    // Ensure the main window is shown once (timer or click)
    bool shown = false;
    auto showMainOnce = [&]() {
        if (shown) return;
        shown = true;
        win.show();
        splash.finish(&win);
        // If a file was passed on command line, open it now
        if (app.arguments().size() > 1) {
            win.openPath(app.arguments().at(1));
        }
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
