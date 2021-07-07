#ifndef SLIMSCROLLBAR_H
#define SLIMSCROLLBAR_H

#include <QObject>
#include <QScrollBar>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QDebug>
#include <QDateTime>
#include <QtMath>
#include "slimscrollbarpopup.h"

#define DEB_EVENT if (0) qDebug()

class SlimScrollBar : public QScrollBar
{
    Q_OBJECT
public:
    SlimScrollBar(QWidget* parent = nullptr);

    /**
     * 鼠标松开时抖动动画
     * 松开的时候计算每一次抖动距离+时间，放入队列中
     * 定时调整抖动的队列实体索引
     */
    struct Jitter
    {
        Jitter(QPoint p, qint64 t) : point(p), timestamp(t) {}
        QPoint point;     // 要运动到的目标坐标
        qint64 timestamp; // 运动到目标坐标应该的时间戳，结束后删除本次抖动路径对象
    };

    /**
     * 鼠标按下/弹起水波纹动画
     * 鼠标按下时动画速度慢（压住），松开后动画速度骤然加快
     * 同样用队列记录所有的水波纹动画实体
     */
    struct Water
    {
        Water(QPoint p, qint64 t) : point(p), press_timestamp(t),
                                    release_timestamp(0), finish_timestamp(0), finished(false) {}
        QPoint point;
        qint64 press_timestamp;   // 鼠标按下时间戳
        qint64 release_timestamp; // 鼠标松开时间戳。与按下时间戳、现行时间戳一起成为水波纹进度计算参数
        qint64 finish_timestamp;  // 结束时间戳。与当前时间戳相减则为渐变消失经过的时间戳
        bool finished;            // 是否结束。结束后改为渐变消失
    };

    void enable();
    void disable();
    void setBgColors(QColor normal, QColor hover, QColor press);
    void setFgColors(QColor normal, QColor hover, QColor press);
    void setRoundCap(bool round);

protected:
    void enterEvent(QEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    void sliderChange(SliderChange change) override;
    void contextMenuEvent(QContextMenuEvent *e) override;
    void hideEvent(QHideEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private:
    void paintPixmap();
    void activeTimer();
    void setOffsetPoss();
    int quick_sqrt(long X) const;
    void calcPixmapSize();
    void startPopup();
    void repaintPopup();
    void setJitter();
    qint64 getTimestamp();
    inline QPen getPen(QColor color, int width);

signals:

public slots:

private slots:
    void eventTimer();

private:
    QPixmap pixmap;
    QTimer* event_timer;

    QColor bg_normal_color = QColor(128, 128, 128, 64);
    QColor bg_hover_color = QColor(128, 128, 128, 0);
    QColor bg_press_color = QColor(128, 128, 128, 32);
    QColor fg_normal_color = QColor(66, 103, 124, 64);
    QColor fg_hover_color = QColor(66, 103, 124, 128);
    QColor fg_press_color = QColor(66, 103, 124, 192);
    bool round_cap = true;

    bool enabling = true;
    bool hovering = false;
    bool pressing = false;
    int hover_prop = 0;
    int press_prop = 0;
    QPoint press_pos;  // 按下位置
    QPoint mouse_pos;  // 鼠标位置
    QPoint target_pos; // 实时随队鼠标的目标点（相对竖直中心）
    QPoint anchor_pos; // 逐步靠近目标点的锚点
    QPoint effect_pos; // 偏差（相对于左上角），逐步靠近锚点根号位置

    bool popuping = false;
    SlimScrollBarPopup* popup = nullptr;
    QPoint popup_offset; // 弹窗和自己的左上角的绝对位置差

    double elastic_coefficient; // 弹性系数
    QList<Jitter> jitters;
    int jitter_duration;

    QList<Water> waters;
    int water_press_duration, water_release_duration, water_finish_duration;
    int water_radius;
};

#endif // SLIMSCROLLBAR_H
