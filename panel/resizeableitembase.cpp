#include <QApplication>
#ifdef Q_OS_WIN32
#include <windows.h>
#include <windowsx.h>
#endif
#include "resizeableitembase.h"

ResizeableItemBase::ResizeableItemBase(QWidget *parent) : PanelItemBase(parent)
{
    for (int i = 0; i < 4; i++)
    {
        resizeCorner[i] = new MoveableWidget(this);
        resizeCorner[i]->setFixedSize(boundaryWidth, boundaryWidth);
        connect(resizeCorner[i], &MoveableWidget::dragPressed, this, [=]{
            setSelect(true, resizeCorner[i]->geometry().center());
        });
        connect(resizeCorner[i], &MoveableWidget::dragReleased, this, [=]{
            if (resizeCorner[i]->isMoved())
                emit modified();
        });
    }

    connect(resizeCorner[0], &MoveableWidget::dragMoved, this, [=](QPoint delta) {
        QRect geo = geometry();
        geo.setTopLeft(geo.topLeft() + delta);
        setGeometry(geo);
    });

    connect(resizeCorner[1], &MoveableWidget::dragMoved, this, [=](QPoint delta) {
        QRect geo = geometry();
        geo.setTopRight(geo.topRight() + delta);
        setGeometry(geo);
    });

    connect(resizeCorner[2], &MoveableWidget::dragMoved, this, [=](QPoint delta) {
        QRect geo = geometry();
        geo.setBottomRight(geo.bottomRight() + delta);
        setGeometry(geo);
    });

    connect(resizeCorner[3], &MoveableWidget::dragMoved, this, [=](QPoint delta) {
        QRect geo = geometry();
        geo.setBottomLeft(geo.bottomLeft() + delta);
        setGeometry(geo);
    });
}

void ResizeableItemBase::adjustCornerPos()
{
    resizeCorner[0]->move(0, 0);
    resizeCorner[1]->move(width() - boundaryWidth, 0);
    resizeCorner[2]->move(width() - boundaryWidth, height() - boundaryWidth);
    resizeCorner[3]->move(0, height() - boundaryWidth);
}

MyJson ResizeableItemBase::toJson() const
{
    MyJson json = PanelItemBase::toJson();
    json.insert("width", width());
    json.insert("height", height());
    return json;
}

void ResizeableItemBase::fromJson(const MyJson &json)
{
    PanelItemBase::fromJson(json);

    int w = json.i("width", this->width());
    int h = json.i("height", this->height());
    resize(w, h);
}

void ResizeableItemBase::resizeEvent(QResizeEvent *event)
{
    PanelItemBase::resizeEvent(event);

    adjustCornerPos();
}

void ResizeableItemBase::selectEvent()
{
    PanelItemBase::selectEvent();

    for (int i = 0; i < 4; i++)
    {
        resizeCorner[i]->raise();
    }
}
