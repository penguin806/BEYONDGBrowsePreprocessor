#include "snowmainwnd.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("GtfFilePreprocessor");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationDomain("xuefeng.space");
    SnowMainWnd w;
    w.show();

    return a.exec();
}
