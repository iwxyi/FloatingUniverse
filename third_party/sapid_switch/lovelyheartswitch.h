#ifndef LOVELYHEARTSWITCH_H
#define LOVELYHEARTSWITCH_H

#include "sapidswitchbase.h"

class LovelyHeartSwitch : public SapidSwitchBase
{
    Q_OBJECT
public:
    LovelyHeartSwitch(QWidget *parent);
    LovelyHeartSwitch(bool state, QWidget* parent);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    virtual void calculateGeometry() override;
    virtual QPainterPath getBgPath() const override;

private:
    QPointF rightAnglePos; // 中间直角的点(rightAngle是直角的意思)
    double diamondSide;    // 每个菱形单位的边长
    double circleRadius;   // 控制大小的圆的半径（中心竖着的长度）
    double circleOutBorder = 2; // 球和背景边缘的间距
};

#endif // LOVELYHEARTSWITCH_H
