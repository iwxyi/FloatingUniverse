#include "usettings.h"

USettings::USettings(QString filePath, QObject *parent) : MySettings(filePath, QSettings::Format::IniFormat, parent)
{
    restoreSettings();
}

void USettings::restoreSettings()
{
    beginGroup("panel");
    assign(panelCenterOffset, "centerOffset");
    assign(panelWidth, "width");
    assign(panelHeight, "height");
    // assign(panelBangWidth, "bangWidth");
    assign(panelBangMarginLeft, "bangMarginLeft");
    assign(panelBangMarginRight, "bangMarginRight");
    assign(panelBgColor, "bgColor");
    assign(panelBangBg, "bangBg");
    assign(panelSelectRectColor, "selectRectColor");
    assign(panelSelectEdgeColor, "selectEdgeColor");
    assign(panelHoverEdgeColor, "hoverEdgeColor");
    assign(panelBlurRadius, "blurRadius");
    assign(panelBlurOpacity, "blurOpacity");
    assign(panelIconSize, "iconSize");
    endGroup();

    beginGroup("geometry");
    assign(moduleSize_Text, "moduleText");
    assign(moduleSize_Todo, "moduleTodo");
    assign(moduleSize_Image, "moduleImage");
    assign(moduleSize_Card, "moduleCard");
    endGroup();

    beginGroup("interactive");
    assign(modifyFileNameSync, "modifyFileNameSync");
    assign(allowMoveOut, "allowMoveOut");
    assign(fastOpenDir, "fastOpenDir");
    assign(fastOpenDirLevel, "fastOpenDirLevel");
    assign(spatialMoveRatio, "spatialMoveRatio");
    assign(autoReboot, "autoReboot");
    endGroup();

    beginGroup("usage");
    assign(bootCount, "bootCount");
    assign(linkOpenCount, "linkOpenCount");
    endGroup();
}
