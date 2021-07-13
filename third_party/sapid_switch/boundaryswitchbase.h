#ifndef BOUNDARYSWITCHBASE_H
#define BOUNDARYSWITCHBASE_H

#include "sapidswitchbase.h"

class BoundarySwitchBase : public SapidSwitchBase
{
    Q_OBJECT
    Q_PROPERTY(double border READ getBorderProg WRITE setBorderProg)
public:
    BoundarySwitchBase(QWidget* parent);
    BoundarySwitchBase(int type, QWidget* parent);
    BoundarySwitchBase(bool state, QWidget* parent);
    BoundarySwitchBase(int type, bool state, QWidget* parent);
    void setType(int mode);

protected:
    void paintEvent(QPaintEvent* event) override;
    virtual QSize sizeHint() const override;

    virtual void startSwitchAnimation(double target, int duration) override;
    virtual void setSwtchProgManual(double p) override;

    virtual void calculateGeometry() override;
    virtual QPainterPath getBgPath() const override;
    virtual void drawBorder(QPainter &painter);
    virtual void drawFg(QPainter &painter);
    virtual void drawText(QPainter &painter);

protected:
    double getBorderProg() const;
    void setBorderProg(double prog);
    bool isTextReverse() const;
    QRectF rect() const;

protected:
    double borderProg;
    int mode = 0;
    double radius = 0; // 圆角矩形的半径 = height()/2
    double circleOutMargin = 4; // 指示球和边界的距离
};

#endif // BOUNDARYSWITCHBASE_H
