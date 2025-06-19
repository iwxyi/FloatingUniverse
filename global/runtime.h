#ifndef RUNTIME_H
#define RUNTIME_H

#include <QObject>

class Runtime : public QObject
{
    Q_OBJECT
public:
    explicit Runtime(QString path, QObject *parent = nullptr);

public:
    QString APP_PATH;
    QString DATA_PATH;
    QString PANEL_PATH; // 面板JSON文件位置
    QString PANEL_FILE_PATH; // 面板暂存文件位置
    QString ICON_PATH; // 图标文件夹位置
    QString IMAGE_PATH;

    QString CACHE_PATH;
    QString CACHE_IMAGE_PATH;

    bool flag_readingItems = false; // 是否正在读取item，可能会触发一些信号

    const QString imageSuffix = ".png";
    bool* panel_expading = nullptr;
    bool* panel_animating = nullptr;
    bool* panel_fixing = nullptr;
};

extern Runtime* rt;

#endif // RUNTIME_H
