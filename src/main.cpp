#define ARB_DEBUG

#include <QApplication>
#include <QOpenGLWidget>
#include "MainWindow.h"

int main(int argc, char *argv[]) {

#ifdef ARB_DEBUG
    if (argc < 2) {
        argv = new char* [2]{ argv[0], "../extra/db/moss.g" };
        argc = 2;
    }
#endif

    QApplication app(argc,argv);

    MainWindow mainWindow;
    mainWindow.showMaximized();

    return app.exec();
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
