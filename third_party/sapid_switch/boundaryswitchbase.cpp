#include "boundaryswitchbase.h"

BoundarySwitchBase::BoundarySwitchBase(QWidget *parent) : SapidSwitchBase(parent)
{
    setMinimumSize(96, 24);

    setAnimationDuration(600);
    setBackground(QColor(30, 144, 255), Qt::lightGray);
    setBorder(QColor(30, 144, 255), 2);
    setAnimationEasingCurve(QEasingCurve::OutCirc);
    calculateGeometry();
}

BoundarySwitchBase::BoundarySwitchBase(int type, QWidget *parent) : BoundarySwitchBase(parent)
{
    setType(type);
}

BoundarySwitchBase::BoundarySwitchBase(bool state, QWidget *parent) : BoundarySwitchBase(parent)
{
    setState(state);
}

BoundarySwitchBase::BoundarySwitchBase(int type, bool state, QWidget *parent) : BoundarySwitchBase(parent)
{
    setType(type);
    setState(state);
}

void BoundarySwitchBase::setType(int mode)
{
    this->mode = mode;
}

void BoundarySwitchBase::paintEvent(QPaintEvent *event)
{
    SapidSwitchBase::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 画边界
    drawBorder(painter);

    // 画文字（会被上面的开关覆盖）
    drawText(painter);

    // 画开关
    drawFg(painter);
}

QSize BoundarySwitchBase::sizeHint() const
{
    return QSize(128, 32);
}

void BoundarySwitchBase::startSwitchAnimation(double target, int duration)
{
    SapidSwitchBase::startSwitchAnimation(target, duration);

    // 开启边界动画
    QPropertyAnimation* ani = new QPropertyAnimation(this, "border");
    ani->setStartValue(borderProg);
    ani->setEndValue(target);
    ani->setEasingCurve(curve);
    ani->setDuration(static_cast<int>(duration * 1.5));
    connect(ani, SIGNAL(finished()), ani, SLOT(deleteLater()));
    connect(ani, SIGNAL(valueChanged(const QVariant &)), this, SLOT(update()));
    ani->start();
}

void BoundarySwitchBase::setSwtchProgManual(double p)
{
    setBorderProg(p);
    SapidSwitchBase::setSwtchProgManual(p);
}

void BoundarySwitchBase::calculateGeometry()
{
    SapidSwitchBase::calculateGeometry();
    radius = height() / 2 - borderSize/2.0;
}

QPainterPath BoundarySwitchBase::getBgPath() const
{
    QPainterPath path;
    if (mode == 3)
    {
        path.addRect(rect());
    }
    else if (mode == 1 || mode == 2)
    {
        path.addRoundedRect(rect(), radius, radius);
    }
    return path;
}

void BoundarySwitchBase::drawBorder(QPainter &painter)
{
    const double prop = borderProg;

    if (mode == 1)
    {
        // 画OFF
        if (prop <= 0.75)
        {
            painter.save();
            // 右半部分整体rect
            QPainterPath path;
            QRectF rect = this->rect();
            rect.setRight(rect.center().x());
            path.addRoundedRect(rect, radius, radius);
            double startAngle = 0;
            double arcAngle = 360 * (0.75 - prop) / 0.75;

            // 去掉的部分
            QPainterPath clipPath;
            const QPointF center = rect.center();
            const double r = rect.width()/2;
            clipPath.moveTo(center);
            clipPath.lineTo(this->rect().center());
            QRectF circleRect(center.x()-r-1, center.y()-r, r*2+2, r*2);
            clipPath.arcTo(circleRect, startAngle, arcAngle);
            clipPath.lineTo(center);

            painter.setClipPath(clipPath);
            painter.setPen(QPen(colorOff, borderSize));
            painter.drawPath(path);
            painter.restore();
        }

        // 画ON
        if (prop >= 0.25)
        {
            painter.save();
            // 左半部分整体rect
            QPainterPath path;
            QRectF rect = this->rect();
            rect.setLeft(this->width() / 2);
            path.addRoundedRect(rect, radius, radius);
            double startAngle = 180;
            double arcAngle = 360 * (prop-0.25) / 0.75;

            // 去掉的部分
            QPainterPath clipPath;
            const QPointF center = rect.center();
            const double r = rect.width()/2;
            clipPath.moveTo(center);
            clipPath.lineTo(this->rect().center());
            QRectF circleRect(center.x()-r-1, center.y()-r, r*2+2, r*2);
            clipPath.arcTo(circleRect, startAngle, -arcAngle);
            clipPath.lineTo(center);

            painter.setClipPath(clipPath);
            painter.setPen(QPen(colorOn, borderSize));
            painter.drawPath(path);
            painter.restore();
        }
    }
    else if (mode == 2)
    {
        painter.save();
        QRectF rect = this->rect();
        painter.setPen(QPen(colorOff, borderSize));
        QPainterPath path;
        path.addRoundedRect(rect, radius, radius);
        painter.drawPath(path);

        // 画动画背景
        const double totalLen = 2 * sapid_PI * radius + 2 * (rect.width() - 2 * radius);
        double currLen = totalLen * prop; // 应当动画的时长
        path.clear();
        path.moveTo(borderSize/2.0, radius);
        if (currLen > 0) // 左下角四分之一个圆
        {
            QPointF o(rect.left() + radius, radius);
            double len = radius * sapid_PI / 2; // 90°周长
            double angle;
            if (len <= currLen) // 够长
            {
                angle = 90;
                currLen -= len;
            }
            else // len > currLen，不够这一个圆的
            {
                angle = 90 * currLen / len;
                currLen = 0;
            }
            path.arcTo(QRectF(borderSize/2.0, borderSize/2.0, radius*2, radius*2), 180, angle);
        }
        if (currLen > 0) // 下面直线
        {
            double len = rect.right() - 2 * radius;
            double moveLen;
            if (currLen > len) // 够长
            {
                moveLen = len;
                currLen -= moveLen;
            }
            else
            {
                moveLen = currLen;
                currLen = 0;
            }
            path.lineTo(borderSize/2.0 + radius + moveLen, height()-borderSize/2.0);
        }
        if (currLen > 0) // 右边半圆
        {
            QPointF o(rect.right() - radius, radius);
            double len = radius * sapid_PI; // 半圆周长
            double angle;
            if (len <= currLen) // 够长
            {
                angle = 180;
                currLen -= len;
            }
            else // len > currLen，不够这一个圆的
            {
                angle = 180 * currLen / len;
                currLen = 0;
            }
            path.arcTo(QRectF(width()-2*radius-borderSize/2.0, borderSize/2.0, radius*2, radius*2), 270, angle);
        }
        if (currLen > 0) // 上面直线
        {
            double len = rect.width() - 2 * radius;
            double moveLen;
            if (currLen > len) // 够长
            {
                moveLen = len;
                currLen -= moveLen;
            }
            else
            {
                moveLen = currLen;
                currLen = 0;
            }
            path.lineTo(width()-radius-moveLen-borderSize/2.0, borderSize/2.0);
        }
        if (currLen > 0) // 左上角圆
        {
            QPointF o(rect.left() + radius, radius);
            double len = radius * sapid_PI / 2; // 90°周长
            double angle;
            if (len <= currLen) // 够长
            {
                angle = 90;
                currLen -= len;
            }
            else // len > currLen，不够这一个圆的
            {
                angle = 90 * currLen / len;
                currLen = 0;
            }
            path.arcTo(QRectF(borderSize/2.0, borderSize/2.0, radius*2, radius*2), 90, angle);
        }
        painter.setPen(QPen(colorOn, borderSize));
        painter.drawPath(path);
        painter.restore();
    }
    else if (mode == 3)
    {
        painter.save();
        QRectF rect = this->rect();
        painter.setPen(QPen(colorOff, borderSize));
        QPainterPath path;
        path.addRect(rect);
        painter.drawPath(path);

        const double w = rect.width(), h = rect.height();
        const double totalLen = w*2 + h*2;
        double currLen = totalLen * prop;
        path.clear();
        path.moveTo(borderSize/2.0, borderSize/2.0);
        if (currLen > 0) // 左边
        {
            double len = h;
            double moveLen;
            if (len < currLen) // 够长
            {
                moveLen = len;
                currLen -= moveLen;
            }
            else
            {
                moveLen = currLen;
                currLen = 0;
            }
            path.lineTo(rect.left(), rect.top() + moveLen);
        }
        if (currLen > 0) // 下边
        {
            double len = w;
            double moveLen;
            if (len < currLen) // 够长
            {
                moveLen = len;
                currLen -= moveLen;
            }
            else
            {
                moveLen = currLen;
                currLen = 0;
            }
            path.lineTo(rect.left() + moveLen, rect.bottom());
        }
        if (currLen > 0) // 右边
        {
            double len = h;
            double moveLen;
            if (len < currLen) // 够长
            {
                moveLen = len;
                currLen -= moveLen;
            }
            else
            {
                moveLen = currLen;
                currLen = 0;
            }
            path.lineTo(rect.right(), rect.bottom()-moveLen);
        }
        if (currLen > 0) // 上边
        {
            double len = w;
            double moveLen;
            if (len < currLen) // 够长
            {
                moveLen = len;
                currLen -= moveLen;
            }
            else
            {
                moveLen = currLen;
                currLen = 0;
            }
            path.lineTo(rect.right() - moveLen, rect.top());
        }

        painter.setPen(QPen(colorOn, borderSize));
        painter.drawPath(path);
        painter.restore();
    }
}

void BoundarySwitchBase::drawFg(QPainter &painter)
{
    if (mode == 1)
    {
        // 这个模式不用画前景
    }
    else if (mode == 2)
    {
        const double prop = aniProgess;
        const double margin = circleOutMargin;
        double maxDelta = (width() - margin*2) / 2;
        double left = margin + maxDelta * (qMax(prop, 0.5)-0.5) / 0.5; // 0.5~1
        double right = width() - margin - maxDelta * (0.5 - qMin(prop, 0.5)) / 0.5; // 0~0.5
        double r = (height() - margin*2) / 2;
        QPainterPath path;
        path.addRoundedRect(QRectF(left, margin, right - left, height() - margin*2), r, r);
        painter.fillPath(path, getBgColor());
    }
    else if (mode == 3)
    {
        const double prop = aniProgess;
        const double fixedX = borderSize+2; // 偏向的一半（上下各加减，即X差乘二）
        const QRectF rect(this->rect());
        const double w = rect.width()/2;
        QPainterPath path;
        const double left = rect.left() + w * prop;
        const double right = left + w;
        const double topLeft = left - fixedX * prop;
        const double bottomLeft = left + fixedX * prop;
        const double topRight = right + fixedX * (1-prop);
        const double bottomRight = right - fixedX * (1-prop);
        path.moveTo(topLeft, rect.top());
        path.lineTo(bottomLeft, rect.bottom());
        path.lineTo(bottomRight, rect.bottom());
        path.lineTo(topRight, rect.top());
        path.lineTo(topLeft, rect.top());
        painter.fillPath(path, getBgColor());
    }
}

void BoundarySwitchBase::drawText(QPainter &painter)
{
    // 画OFF
    // if (isTextReverse() && getState()) // 分状态画？
    QRectF rect = this->rect();
    rect.setRight(width() / 2);
    painter.setPen(colorOff);
    painter.drawText(rect, Qt::AlignCenter, !isTextReverse() ? "OFF" : "ON");

    // 画ON
    rect = this->rect();
    rect.setLeft(width() / 2);
    painter.setPen((!isTextReverse() && isChecked()) ? colorOn : colorOff);
    painter.drawText(rect, Qt::AlignCenter, !isTextReverse() ? "ON" : "OFF");
}

double BoundarySwitchBase::getBorderProg() const
{
    return borderProg;
}

void BoundarySwitchBase::setBorderProg(double prog)
{
    this->borderProg = prog;
    update();
}

/**
 * 有些是操作文字
 * 有些是状态文字
 * 所以要反过来
 */
bool BoundarySwitchBase::isTextReverse() const
{
    return mode != 1;
}

QRectF BoundarySwitchBase::rect() const
{
    return QRectF(borderSize/2.0, borderSize/2.0, width()-borderSize, height()-borderSize);
}
