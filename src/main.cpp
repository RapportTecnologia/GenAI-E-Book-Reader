#ifdef USE_QT
#include <QApplication>
#include "ui/MainWindow.h"
#endif
#include <iostream>
#include "app/App.h"

int main(int argc, char* argv[]) {
#ifdef USE_QT
    QApplication app(argc, argv);

    MainWindow win;
    win.setWindowTitle(QString("%1 v%2 — Leitor")
                           .arg(genai::AppInfo::Name)
                           .arg(genai::AppInfo::Version));
    win.resize(1024, 768);
    win.show();

    return app.exec();
#else
    (void)argc; (void)argv;
    std::cout << genai::AppInfo::Name << " v" << genai::AppInfo::Version
              << " — Console placeholder (Qt não encontrado)." << std::endl;
    return 0;
#endif
}
