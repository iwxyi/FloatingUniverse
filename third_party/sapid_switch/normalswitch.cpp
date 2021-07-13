#include "normalswitch.h"

NormalSwitch::NormalSwitch(QWidget *parent) : BoundarySwitchBase(parent)
{
    setMinimumSize(60, 20);
    setAnimationEasingCurve(QEasingCurve::OutBack);
    setForeground(Qt::white);
    setBorder(Qt::gray, 2);
    circleOutMargin = 2;
}

NormalSwitch::NormalSwitch(bool state, QWidget *parent) : NormalSwitch(parent)
{
    setState(state);
}

void NormalSwitch::setSuitableHeight(int h)
{
    BoundarySwitchBase::setSuitableHeight(h);
}

void NormalSwitch::calculateGeometry()
{
    BoundarySwitchBase::calculateGeometry();
    QRectF rect = this->rect();
    double l = rect.left() + rect.width()*0.08;
    double r = rect.right() - rect.width()*0.08;
    bgRect = QRectF(l+circleOutMargin, rect.top()+circleOutMargin,
                  r-l-circleOutMargin*2, rect.height()-circleOutMargin*2);
}

void NormalSwitch::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true); // 抗锯齿

    // 画背景圆角矩形
    QPainterPath path = getBgPath();
    path.setFillRule(Qt::WindingFill);
    path.addRoundedRect(bgRect, radius, radius);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.fillPath(path, QBrush(getBgColor()));

    // 计算指示球进度
    double l = bgRect.left() + radius - circleOutMargin;
    double r = bgRect.right() - radius + circleOutMargin;
    double x = l + (r - l) * aniProgess;

    // 画圆形滑块外圈
    QPainterPath path2;
    path2.addEllipse(QRectF(x-radius, borderSize/2.0, radius*2, radius*2));
    painter.fillPath(path2, colorBd);

    // 画圆形滑块
    QPainterPath path3;
    path3.addEllipse(QRectF(x-radius+circleOutMargin, borderSize/2.0+circleOutMargin,
                            radius*2-circleOutMargin*2, radius*2-circleOutMargin*2));
    painter.fillPath(path3, colorFg);
}

QPainterPath NormalSwitch::getBgPath() const
{
    QPainterPath path;
    path.addRoundedRect(bgRect, bgRect.height()/2, bgRect.height()/2);
    return path;
}
