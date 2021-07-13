#include "runtime.h"

Runtime::Runtime(QString path, QObject *parent) : QObject(parent)
{
    APP_PATH = path + "/";
    DATA_PATH = APP_PATH + "data/";
    PANEL_PATH = DATA_PATH + "panel.json";
    PANEL_FILE_PATH = DATA_PATH + "files/";
    ICON_PATH = DATA_PATH + "icons/";
    IMAGE_PATH = DATA_PATH + "images/";
    CACHE_PATH = DATA_PATH + "cache/";
    CACHE_IMAGE_PATH = CACHE_PATH + "images/";
}
