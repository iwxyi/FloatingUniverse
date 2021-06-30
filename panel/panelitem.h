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

    PanelItem(const QPixmap& pixmap, const QString& text, QWidget* parent = nullptr);

    MyJson toJson() const;
    void fromJson(MyJson json);

signals:

public slots:
    void showSelect(bool sh);

public:
    QLabel* iconLabel;
    QLabel* textLabel;
    QWidget* selectWidget;

    const int selectBorder = 2;
};

#endif // PANELITEM_H
