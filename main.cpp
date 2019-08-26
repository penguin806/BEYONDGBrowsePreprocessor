// https://github.com/penguin806/GtfFilePreprocessor.git
#include "snowmainwnd.h"
#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QTextCodec *codec = QTextCodec::codecForName("GBK");
    QTextCodec::setCodecForLocale(codec);
    const QString applicationName =
            QString::fromLocal8Bit("高通量多组学序列数据可视化浏览器 - 数据预处理");
    QApplication a(argc, argv);
    // QApplication::setApplicationName("GtfFilePreprocessor");
    QApplication::setApplicationName(applicationName);
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationDomain("xuefeng.space");
    SnowMainWnd w;
    w.setWindowTitle(applicationName);
    w.show();

    return a.exec();
}
