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

enum Direction
{
    LeftToRight,
    RightToLeft,
    TopToBottom,
    BottomToTop
};

class USettings : public MySettings
{
public:
    USettings(QObject* parent = nullptr);

private:
    void restoreSettings();

public:

public:
    Side bannerFloatSide = SideRight; // 边：0上，1左，2右，3下，-1任意
    Direction bannerFloatDirection = TopToBottom; // 方向：0从左到右，1从右到左，2从上到下，3从下到上

};

extern USettings* us;

#endif // USERSETTINGS_H
