#ifndef RUNTIME_H
#define RUNTIME_H

#include <QObject>

class Runtime : public QObject
{
    Q_OBJECT
public:
    explicit Runtime(QObject *parent = nullptr);

    void setAppPath(QString path);

public:
    QString APP_PATH;
    QString PANEL_PATH;
    QString ICON_PATH;
    QString DATA_PATH;
    QString IMAGE_PATH;

    QString CACHE_PATH;
    QString CACHE_IMAGE_PATH;

private:
    const QString imageSuffix = ".png";
};

extern Runtime* rt;

#endif // RUNTIME_H
