#ifndef ROUNDLABEL_H
#define ROUNDLABEL_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QColor>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QBrush>
#include <QPropertyAnimation>

#define ZOOM 1.25

/**
 * 显示颜色的带缩放动画的圆形控件
 */
class AniCircleLabel : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(int animation_progress READ getAnimationProgress WRITE setAnimationProgress RESET resetAnimationProgress)

public:
    AniCircleLabel(QWidget* parent);
    AniCircleLabel(QColor c, QWidget* parent);

    void setMainColor(QColor c);
    void setSplit(bool sp);
    void setSplitColor(QColor sp_c);

protected:
    void paintEvent(QPaintEvent* e);

    void startChangeAnimation();

private:
    int getAnimationProgress();
	void setAnimationProgress(int x);
    void resetAnimationProgress();

private:
    QColor color;
    int dr, dg, db, da;
    int animation_progress;
    bool split = false;
    QColor split_color = Qt::white;
};

#endif // ROUNDLABEL_H
