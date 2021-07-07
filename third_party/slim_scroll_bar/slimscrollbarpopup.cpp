#include "slimscrollbarpopup.h"

SlimScrollBarPopup::SlimScrollBarPopup(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents,true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::ToolTip);
    setAttribute(Qt::WA_TranslucentBackground);
    setFocusPolicy(Qt::NoFocus);
}

void SlimScrollBarPopup::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
//    painter.fillRect(0, 0, width()-1, height()-1, QColor(0x33, 0xFF, 0xff, 0x33));

    painter.drawPixmap(QPoint(0,0), scrollbar_pixmap);
//    painter.drawPixmap(offset, scrollbar_pixmap);
}
