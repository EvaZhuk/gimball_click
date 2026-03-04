#include "gui/mainwindow.h"
#include "mavlink/mavlinkinterface.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    MavlinkInterface mav;
    mav.start();


    w.show();
    return a.exec();
}
