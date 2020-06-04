#include <iostream>
#include <QtWidgets/QApplication>
#include "MainWindow.h"

int main(int argc, char*argv[]) {
    std::cout << "Hello, World!" << std::endl;


    QApplication app(argc,argv);
    MainWindow mw;
    mw.show();
    app.exec();

    return 0;
}
