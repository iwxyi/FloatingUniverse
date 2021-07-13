#ifndef NORMALSWITCH_H
#define NORMALSWITCH_H

#include "boundaryswitchbase.h"

class NormalSwitch : public BoundarySwitchBase
{
public:
    NormalSwitch(QWidget* parent);
    NormalSwitch(bool state, QWidget* parent);

    virtual void setSuitableHeight(int h) override;

protected:
    virtual void calculateGeometry() override;
    virtual void paintEvent(QPaintEvent*) override;
    virtual QPainterPath getBgPath() const override;

private:
    QRectF bgRect; // 背景圆角矩形的位置
};

#endif // NORMALSWITCH_H
