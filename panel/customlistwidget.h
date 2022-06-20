#ifndef CUSTOMLISTWIDGET_H
#define CUSTOMLISTWIDGET_H

#include <QListWidget>
#include "third_party/smooth_scroll/smoothscrollbean.h"

class CustomListWidget : public QListWidget
{
    Q_OBJECT
public:
    CustomListWidget(QWidget* parent);

signals:
    void focusIn();

protected:
    // ---------- scroll ----------
    void setSmoothScrollEnabled(bool e);
    void setSmoothScrollSpeed(int speed);
    void setSmoothScrollDuration(int duration);

    void scrollToTop();
    void scrollTo(int pos);
    void scrollToBottom();
    bool isToBottoming() const;

private:
    void addSmoothScrollThread(int distance, int duration);

signals:
    void signalLoadTop(); // 到顶端之后下拉
    void signalLoadBottom(); // 到底部之后上拉

public slots:
    void slotSmoothScrollDistance(SmoothScrollBean* bean, int dis);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;

private:
    bool enabledSmoothScroll = true;
    int smoothScrollSpeed = 64;
    int smoothScrollDuration = 200;
    QList<SmoothScrollBean*> smooth_scrolls;
    int toBottoming = 0;
};

#endif // CUSTOMLISTWIDGET_H
