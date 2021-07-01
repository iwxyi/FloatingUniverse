#include "usettings.h"

USettings::USettings(QObject *parent) : MySettings("settings.ini", QSettings::Format::IniFormat, parent)
{
    restoreSettings();
}

void USettings::restoreSettings()
{
    beginGroup("panel");
    assign(panelCenterOffset, "centerOffset");
    assign(panelWidth, "width");
    assign(panelHeight, "height");
    panelBangWidth = panelWidth * 0.8;
    endGroup();
}
