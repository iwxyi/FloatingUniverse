#ifndef PANELITEM_H
#define PANELITEM_H

#include <QObject>
#include <QLabel>
#include "myjson.h"

class PanelItem : public QWidget
{
    Q_OBJECT
public:
    explicit PanelItem(QWidget *parent = nullptr);

    MyJson toJson() const;
    static PanelItem* fromJson(const MyJson& json, QWidget* parent = nullptr);

    void setIcon(const QString& iconName);
    void setText(const QString& text);
    void setLink(const QString& link);

signals:
    void triggered();

public slots:
    void showSelect(bool sh);

public:
    QLabel* iconLabel;
    QLabel* textLabel;
    QWidget* selectWidget;

    const int selectBorder = 2;

    QString iconName;
    QString text;
    QString link; // 文件或者网址
};

#endif // PANELITEM_H
