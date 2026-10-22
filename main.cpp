#include "mainwindow.h"
#include "Application.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    Application::bootstrap(argv[0]);
    Application app(argc, argv);

    if (!app.init())
    {
        return 0;
    }

    MainWindow win;
    Application::setMainWindow(&win);

    win.show();

    return app.exec();
}
