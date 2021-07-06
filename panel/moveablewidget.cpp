#include <QMouseEvent>
#include "moveablewidget.h"

MoveableWidget::MoveableWidget(QWidget *parent) : QWidget(parent)
{
    setCursor(Qt::SizeAllCursor);
}

bool MoveableWidget::isMoved() const
{
    return moved;
}

void MoveableWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        pressing = true;
        prevPressPos = event->globalPos();
        moved = false;
        return ;
    }

    return QWidget::mousePressEvent(event);
}

void MoveableWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (pressing)
    {
        QPoint pos = event->globalPos();
        emit dragMoved(pos - prevPressPos);
        prevPressPos = pos;
        moved = true;
        return ;
    }

    return QWidget::mouseMoveEvent(event);
}

void MoveableWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (pressing)
    {
        emit dragReleased();
        pressing = false;
    }
    return QWidget::mouseReleaseEvent(event);
}
