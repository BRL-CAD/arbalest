#define ARB_DEBUG

#include <QtWidgets/QApplication>
#include "MainWindow.h"

int main(int argc, char*argv[]) {
#ifdef ARB_DEBUG
    argv = new char* [2]{ argv[0], "C:/Users/Sadeep/Desktop/sma/moss.g" };
    argc = 2;
#endif


    QApplication app(argc,argv);
    MainWindow mainWindow;
    mainWindow.showMaximized();
    QApplication::exec();
    return 0;
}

/*
INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR lpCmdLine, INT nCmdShow)
{

// also add to cmake: add_executable(arbelst WIN32
    int argc = 1;
    char* argv[1]{ "" };

    QApplication app(argc, argv);
    MainWindow mainWindow;
    mainWindow.show();
    QApplication::exec();
    return 0;
}
*/