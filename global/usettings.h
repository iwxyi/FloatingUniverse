#ifndef USERSETTINGS_H
#define USERSETTINGS_H

#include <QColor>
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
    USettings(QObject* parent = nullptr);

private:
    void restoreSettings();

public:

public:
    Side panelFloatSide = SideTop; // 边：0上，1左，2右，3下，-1任意
    int panelCenterOffset = 0; // 中心点偏移
    int panelWidth = 1000;
    int panelHeight = 200;
    int panelBangHeight = 1;
    int panelBangWidth = 800;
    QColor panelBg = QColor(240, 248, 255);
    QColor panelBangBg = QColor(128, 128, 128, 128);
    QColor panelSelectBg = QColor(230, 230, 250);
    QColor panelSelectEdge = QColor(0, 71, 125);
    int pannelItemSize = 64;

    int fluentRadius = 5;
};

extern USettings* us;

#endif // USERSETTINGS_H
