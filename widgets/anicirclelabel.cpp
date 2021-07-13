#include "anicirclelabel.h"

AniCircleLabel::AniCircleLabel(QWidget *parent) : QLabel(parent)
{
    color = QColor(0xFF, 0xFF, 0xFF, 0);
}

AniCircleLabel::AniCircleLabel(QColor c, QWidget *parent) : AniCircleLabel(parent)
{
    color = c;
}

void AniCircleLabel::setMainColor(QColor c)
{
    //old_color = color;
    // 颜色差：新-旧，新-(100-progress)*颜色差
    dr = c.red() - color.red();
    dg = c.green() - color.green();
    db = c.blue() - color.blue();
    da = c.alpha() - color.alpha();
    color = c;

    startChangeAnimation();
}

void AniCircleLabel::setSplitColor(QColor sp_c)
{
    this->split_color = sp_c;
}

void AniCircleLabel::startChangeAnimation()
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "animation_progress");
    animation->setDuration(500);
    animation->setStartValue(0);
    animation->setEndValue(100);
    animation->start();
}

int AniCircleLabel::getAnimationProgress()
{
    return animation_progress;
}

void AniCircleLabel::setAnimationProgress(int x)
{
    animation_progress = x;
    update();
}

void AniCircleLabel::resetAnimationProgress()
{
    animation_progress = 0;
}

void AniCircleLabel::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    QPainterPath pathBack;
    painter.setRenderHint(QPainter::Antialiasing, true); // 抗锯齿
    pathBack.setFillRule(Qt::WindingFill);

    // 设置标签宽高
    int height = this->height(), width = this->width();
    int top = 0, left = 0;
    int dia;
    if (width > height)   // 挤满高度
        dia = height / ZOOM;
    else                  // 挤满宽度
        dia = width / ZOOM;

    if (animation_progress <= 50)
    {
        dia += dia * 0.25 * animation_progress / 50;
    }
    else if (animation_progress < 100)
    {
        dia += dia * 0.25 * (100-animation_progress) / 50;
    }
    top = (height - dia) >> 1;
    left = (width - dia) >> 1;

    // 设置标签颜色
    QColor c = color;
    if (animation_progress >= 0 && animation_progress < 100)
    {
        int r = color.red();
        int g = color.green();
        int b = color.blue();
        int a = color.alpha();
        r -= dr * (100-animation_progress) / 100;
        g -= dg * (100-animation_progress) / 100;
        b -= db * (100-animation_progress) / 100;
        a -= da * (100-animation_progress) / 100;
        c = QColor(r, g, b, a);
    }

    // 添加周围的圆形边框（开启分割线阴影）
    if (split)
    {
        QPen pen(painter.pen());
        pen.setWidth(2);
        pen.setColor(split_color);
        painter.setPen(pen);
        painter.drawEllipse(QRect(left, top, dia, dia));
    }

    // 添加表示正中心的圆
    pathBack.addEllipse(QRect(left, top, dia, dia));
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.fillPath(pathBack, QBrush(c));

    return QLabel::paintEvent(e);
}
