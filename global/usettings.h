#ifndef USERSETTINGS_H
#define USERSETTINGS_H

#include <QColor>
#include <QSize>
#include "mysettings.h"

enum Side
{
    SideTop,
    SideLeft,
    SideRight,
    SideBottom
};

class USettings : public MySettings
{
public:
    USettings(QString filePath, QObject* parent = nullptr);

private:
    void restoreSettings();

public:
    // 面板坐标
    Side panelFloatSide = SideTop; // 边：0上，1左，2右，3下，-1任意
    int panelCenterOffset = 0; // 中心点偏移
    int panelWidth = 1000;
    int panelHeight = 200;
    int panelBangHeight = 5;
    int panelBangWidth = 800;
    int panelBangMarginLeft = 100;
    int panelBangMarginRight = 100;

    // 面板设置
    QColor panelBgColor = QColor(240, 248, 255);
    QColor panelBangBg = QColor(128, 128, 128, 1);
    QColor panelSelectRectColor = QColor(230, 230, 250);
    QColor panelSelectEdgeColor = QColor(179, 153, 255);
    QColor panelHoverEdgeColor = QColor(204, 204, 255);
    bool panelBlur = true;
    int panelBlurRadius = 64; // 毛玻璃背景模糊半径
    int panelBlurOpacity = 32; // 毛玻璃透明度

    // 控件尺寸（宇宙荧光）
    int panelItemSize = 64; // Item大小
    int panelIconSize = 32; // 图标大小
    int buttonSize = 24; // 小型按钮控件的大小
    int widgetSize = 32; // 一些中型控件
    int fluentRadius = 5;
    int settingsMaxWidth = 680;
    int settingsMinWidth = 300;
    int settingsMargin = 12;

    QSize moduleSize_Text; // 该类型模块默认大小
    QSize moduleSize_Todo;
    QSize moduleSize_Image;
    QSize moduleSize_Card;

    // 交互
    int spatialMoveRatio = 1; // 控件移动倍率
    bool allowMoveOut = true; // 允许拖拽到面板外面，而不隐藏
    bool fastOpenDir = false; // 默认使用快速打开
    int fastOpenDirLevel = 5; // 最多显示3级文件
    QColor themeMainColor = QColor(51, 103, 214); // #3367D6
    bool modifyFileNameSync = true; // 询问同步修改文件名
    bool autoReboot = false; // 开机自启
    bool keepOnItemUsing = true; // 组件正在使用时不隐藏面板

    // 使用记录
    int bootCount = 0; // 程序启动次数
    int linkOpenCount = 0; // 链接打开次数
};

extern USettings* us;

#endif // USERSETTINGS_H
