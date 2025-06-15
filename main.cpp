#include "runtime.h"
#include "usettings.h"
#include "accountinfo.h"
#include "signaltransfer.h"
#include "mainwindow.h"
#include "fileutil.h"
#include "qtsingleapplication/qtsingleapplication.h"

QString APPLICATION_NAME = "悬浮宇宙";
QString VERSION_CODE = "0.1";
Runtime* rt;
USettings* us;
AccountInfo* ac;
SignalTransfer* sig;

int main(int argc, char *argv[])
{
    QApplication::setDesktopSettingsAware(true); // 据说是避免不同分辨率导致显示的字体大小不一致
#if (QT_VERSION > QT_VERSION_CHECK(5, 6, 0))
    // 设置高分屏属性以便支持 2K4K 等高分辨率，尤其是手机 app。
    // 必须写在 main 函数的 QApplication a (argc, argv); 的前面
    // 设置后，读取到的窗口会随着显示器倍数而缩小
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QCoreApplication::setApplicationName(APPLICATION_NAME);
    QCoreApplication::setApplicationVersion(VERSION_CODE);

    const QString APP_ID("FloatingUniverse");// 单例ID
    QtSingleApplication a(APP_ID, argc, argv);
    if (a.isRunning())
        return !a.sendMessage("FUFUFU");
    a.setQuitOnLastWindowClosed(false); // 关闭最后一个窗口的时候程序不退出（菜单也算窗口）

#ifdef Q_OS_WIN32
    QFont font(a.font());
    font.setFamily("微软雅黑");
    font.setPointSize(qMax(10, font.pointSize()));
    a.setFont(font);
#endif

    // Mac窗口
    qputenv("QT_MAC_WANTS_LAYER", "1");

    // 初始化全局配置
    rt = new Runtime(QApplication::applicationDirPath());
    us = new USettings(rt->DATA_PATH + "settings.ini");
    ac = new AccountInfo;
    sig = new SignalTransfer;

    FacileMenu::auto_theme_by_bg = true;

    ensureDirExist(rt->ICON_PATH);
    ensureDirExist(rt->PANEL_FILE_PATH);
    ensureDirExist(rt->CACHE_IMAGE_PATH);

    MainWindow w;
    if (!us->b("mainwindow/hide", false))
        w.show();

    QObject::connect(&a, &QtSingleApplication::messageReceived, &w, [&](const QString&) {
        w.show(); // 单例自带的 setActivationWindow() 无法打开hide的窗口，得手动show一下才行
        w.activateWindow();
    });

    return a.exec();
}
