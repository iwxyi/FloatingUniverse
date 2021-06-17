#include <QApplication>
#include "runtime.h"
#include "usettings.h"
#include "accountinfo.h"
#include "signaltransfer.h"
#include "mainwindow.h"
#include "fileutil.h"

QString APPLICATION_NAME = "悬浮宇宙";
QString VERSION_CODE = "0.1";
Runtime* rt = new Runtime;
USettings* us = new USettings;
AccountInfo* ac = new AccountInfo;
SignalTransfer* sig = new SignalTransfer;

int main(int argc, char *argv[])
{
    QApplication::setDesktopSettingsAware(true); // 据说是避免不同分辨率导致显示的字体大小不一致
#if (QT_VERSION > QT_VERSION_CHECK(5, 6, 0))
    // 设置高分屏属性以便支持 2K4K 等高分辨率，尤其是手机 app。
    // 必须写在 main 函数的 QApplication a (argc, argv); 的前面
    // 设置后，读取到的窗口会随着显示器倍数而缩小
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName(APPLICATION_NAME);
    QCoreApplication::setApplicationVersion(VERSION_CODE);

    QFont font(a.font());
    font.setFamily("微软雅黑");
    a.setFont(font);

    // 初始化全局配置
    rt->APP_PATH = QApplication::applicationDirPath() + "/";
    rt->DATA_PATH = QApplication::applicationDirPath() + "/data/";
    rt->IMAGE_PATH = rt->DATA_PATH + "images/";
    rt->CACHE_PATH = rt->DATA_PATH + "cache/";
    rt->CACHE_IMAGE_PATH = rt->CACHE_PATH + "images/";
    ensureDirExist(rt->CACHE_IMAGE_PATH);

    MainWindow w;
    w.show();

    return a.exec();
}
