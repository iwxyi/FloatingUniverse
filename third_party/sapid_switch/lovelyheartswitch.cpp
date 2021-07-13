#include "lovelyheartswitch.h"

LovelyHeartSwitch::LovelyHeartSwitch(QWidget *parent) : SapidSwitchBase(parent)
{
    colorOn = QColor(236, 97, 139);
}

LovelyHeartSwitch::LovelyHeartSwitch(bool state, QWidget *parent) : LovelyHeartSwitch(parent)
{
    setState(state);
}

void LovelyHeartSwitch::paintEvent(QPaintEvent *event)
{
    SapidSwitchBase::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 爱心背景
    QPainterPath path = getBgPath();
    painter.fillPath(path, getBgColor());

    // 绘制边界
    if (borderSize)
    {
        painter.setPen(QPen(colorBd, borderSize));
        painter.drawPath(path);
    }

    // 绘制开关圆球
    QPointF slideCenter(rightAnglePos.x(), rightAnglePos.y());
    const double slideRadius = circleRadius + circleOutBorder;
    QRectF slideRect(slideCenter.x()-slideRadius, slideCenter.y()-slideRadius, slideRadius*2, slideRadius*2);
    QPainterPath circleSlidePath;
    circleSlidePath.moveTo(rightAnglePos.x()-slideRadius/GenHao2, rightAnglePos.y()+slideRadius/GenHao2);
    circleSlidePath.arcTo(slideRect, -135, 90);
    QPointF circlePos = circleSlidePath.pointAtPercent(aniProgess);
    QPainterPath circlePath;
    const double currentRadius = circleRadius * pressScaleProgress;
    circlePath.addEllipse(circlePos.x()-currentRadius, circlePos.y()-currentRadius, currentRadius*2, currentRadius*2);
    painter.fillPath(circlePath, colorFg);
}

QPainterPath LovelyHeartSwitch::getBgPath() const
{
    const double& r = diamondSide;
    const double r_2 = r / 2;
    QPainterPath path;
    path.moveTo(rightAnglePos);
    // 右上角耳朵
    const QPointF center1(rightAnglePos.x() + r_2/GenHao2, rightAnglePos.y() + r/2/GenHao2);
    const QRectF rect1(center1.x()-r_2, center1.y()-r_2, r, r);
    path.arcTo(rect1, 135, -180);
    // 中下部分
    const QPointF center2 = rightAnglePos;
    const double radius2 = r;
    const QRectF rect2(center2.x() - radius2, center2.y() - radius2, 2*radius2, 2*radius2);
    path.arcTo(rect2, -45, -90);
    // 左上角耳朵
    const QPointF center3(rightAnglePos.x() - r_2/GenHao2, rightAnglePos.y() + r/2/GenHao2);
    const QRectF rect3 = QRectF(center3.x()-r_2, center3.y()-r_2, r, r);
    path.arcTo(rect3, -135, -180);
    return path;
}

void LovelyHeartSwitch::calculateGeometry()
{
    SapidSwitchBase::calculateGeometry();

    const double widthProp = GenHao2 + 1.0 - 1.0/GenHao2;
    const double heightProp = (GenHao2 - 1.0 + 2 * GenHao2) / 2 / GenHao2;
    diamondSide = qMin((width()-2)/widthProp, (height()-2)/heightProp);
    circleRadius = diamondSide/2-2;
    const double topProp = (GenHao2-1)/2/GenHao2/heightProp;
    rightAnglePos = QPointF(width()/2,
                           diamondSide*heightProp * topProp + (height()-diamondSide*heightProp)/2);

    const double slideRadius = circleRadius + circleOutBorder;
    slideLeft = rightAnglePos.x()-slideRadius/GenHao2;
    slideRight = rightAnglePos.x()+slideRadius/GenHao2;
}
