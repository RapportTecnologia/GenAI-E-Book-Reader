#ifdef USE_QT
#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <Qt>
#endif
#include <iostream>

int main(int argc, char* argv[]) {
#ifdef USE_QT
    QApplication app(argc, argv);

    QMainWindow win;
    win.setWindowTitle("GenAI E-Book Reader (MVP Skeleton)");

    auto *label = new QLabel(&win);
    label->setText("GenAI E-Book Reader — Skeleton inicial");
    label->setAlignment(Qt::AlignCenter);
    win.setCentralWidget(label);

    win.resize(800, 600);
    win.show();

    return app.exec();
#else
    (void)argc; (void)argv;
    std::cout << "GenAI E-Book Reader — Console placeholder (Qt não encontrado)." << std::endl;
    return 0;
#endif
}
