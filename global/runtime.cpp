#include "runtime.h"

Runtime::Runtime(QObject *parent) : QObject(parent)
{
}

/// 设置程序路径
/// path带后缀/
void Runtime::setAppPath(QString path)
{
    APP_PATH = path;
    DATA_PATH = path + "data/";
    PANEL_PATH = DATA_PATH + "panel.json";
    PANEL_FILE_PATH = DATA_PATH + "files/";
    ICON_PATH = rt->DATA_PATH + "icons/";
    IMAGE_PATH = rt->DATA_PATH + "images/";
    CACHE_PATH = rt->DATA_PATH + "cache/";
    CACHE_IMAGE_PATH = rt->CACHE_PATH + "images/";
}

