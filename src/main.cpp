#ifdef USE_QT
#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <Qt>
#endif
#include <iostream>
#include "app/App.h"

int main(int argc, char* argv[]) {
#ifdef USE_QT
    QApplication app(argc, argv);

    QMainWindow win;
    win.setWindowTitle(QString("%1 v%2 — MVP Skeleton")
                           .arg(genai::AppInfo::Name)
                           .arg(genai::AppInfo::Version));

    auto *label = new QLabel(&win);
    label->setText(QString("%1 v%2 — Skeleton inicial")
                       .arg(genai::AppInfo::Name)
                       .arg(genai::AppInfo::Version));
    label->setAlignment(Qt::AlignCenter);
    win.setCentralWidget(label);

    win.resize(800, 600);
    win.show();

    return app.exec();
#else
    (void)argc; (void)argv;
    std::cout << genai::AppInfo::Name << " v" << genai::AppInfo::Version
              << " — Console placeholder (Qt não encontrado)." << std::endl;
    return 0;
#endif
}
