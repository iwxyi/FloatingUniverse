#include "usettings.h"

USettings::USettings(QObject *parent) : MySettings("settings.ini", QSettings::Format::IniFormat, parent)
{
    restoreSettings();
}

void USettings::restoreSettings()
{

}
