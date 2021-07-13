#ifndef SAPIDSWITCHBASE_H
#define SAPIDSWITCHBASE_H

#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QDebug>
#include <QPropertyAnimation>
#include <QMouseEvent>
#include "math.h"

class SapidSwitchBase : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(double swtch READ getSwtchProg WRITE setSwtchProg)
    Q_PROPERTY(double press READ getPressProg WRITE setPressProg)
public:
    explicit SapidSwitchBase(QWidget *parent = nullptr);

    bool getState() const;
    bool isChecked() const;

    virtual void setSuitableHeight(int h);

signals:
    void stateChanged(bool state);

public slots:
    void setState(bool state);
    void setStateWithoutSignal(bool state);
    void toggleState();
    void toggleStateWithoutSignal();

    void setForeground(QColor color);
    void setBackground(QColor on, QColor off);
    void setBorder(QColor color, int size);

    void setAnimationDuration(int dur);
    void setAnimationEasingCurve(QEasingCurve curve);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;

    virtual void calculateGeometry();
    virtual QPainterPath getBgPath() const;
    virtual QColor getBgColor() const;
    virtual void startSwitchAnimation();
    virtual void startSwitchAnimation(double target, int duration);
    virtual void startNoSwitchAnimation();
    virtual void setSwtchProgManual(double p);

private:
    double getSwtchProg();
    void setSwtchProg(double p);
    double getPressProg();
    void setPressProg(double p);

protected:
    // 状态
    bool currentState = false; // 开关状态
    double aniProgess = 0;     // 当前开关动画值，趋向state，范围0~1
    QColor colorFg = QColor(255, 250, 250);
    QColor colorOn = QColor(30, 144, 255);
    QColor colorOff = Qt::lightGray;
    QColor colorBd = Qt::transparent;
    int borderSize = 0; // 四周的间距，算入手势拖拽的位置
    int switchDuration = 350;
    QEasingCurve curve = QEasingCurve::InOutCubic;

    // 几何
    const double sapid_PI = 3.1415926535;
    const double GenHao2 = sqrt(2.0);

    // 滑动手势
    QPoint pressPos;
    bool moved = false;
    bool dragging = false;
    bool moveTargetState = false; // 滑动的目标状态，等待松手
    const double stickOnProp = 0.15; // 在两侧贴靠，不收左右滑动手势影响
    int prevX = 0;
    double slideLeft, slideRight;  // 滑动左右判定边界

    // 按压缩放
    const double pressScale = 0.9; // 按压缩小动画
    double pressAniProg; // 按下去的动画进度
    double pressScaleProgress = 1; // 按压缩小进度
};

#endif // SAPIDSWITCHBASE_H
