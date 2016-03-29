#include "gui.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Gui gui_window;
    gui_window.show();

    return a.exec();
}
