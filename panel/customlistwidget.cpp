#include <QKeyEvent>
#include <QApplication>
#include "customlistwidget.h"
#include "third_party/slim_scroll_bar/slimscrollbar.h"

CustomListWidget::CustomListWidget(QWidget *parent) : QListWidget(parent)
{
    setHorizontalScrollMode(QListWidget::ScrollPerPixel);
    setVerticalScrollMode(QListWidget::ScrollPerPixel);
    auto slim = new SlimScrollBar;
    setVerticalScrollBar(slim);
}
// ---------- scroll ----------

void CustomListWidget::setSmoothScrollEnabled(bool e)
{
    this->enabledSmoothScroll = e;
}

void CustomListWidget::setSmoothScrollSpeed(int speed)
{
    this->smoothScrollSpeed = speed;
}

void CustomListWidget::setSmoothScrollDuration(int duration)
{
    this->smoothScrollDuration = duration;
}

void CustomListWidget::scrollToTop()
{
    scrollTo(verticalScrollBar()->minimum());
}

void CustomListWidget::scrollTo(int pos)
{
    if (!enabledSmoothScroll)
        return verticalScrollBar()->setSliderPosition(pos);

    auto scrollBar = verticalScrollBar();
    int delta = pos - scrollBar->sliderPosition();
    if (qAbs(delta) <= 1)
        return verticalScrollBar()->setSliderPosition(pos);
    addSmoothScrollThread(delta, smoothScrollDuration);
}

void CustomListWidget::scrollToBottom()
{
    int count = smooth_scrolls.size();
    scrollTo(verticalScrollBar()->maximum());
    if (!count || count >= smooth_scrolls.size()) // 理论上来说size会+1
        return ;

    toBottoming++;
    connect(smooth_scrolls.last(), &SmoothScrollBean::signalSmoothScrollFinished, this, [=]{
        toBottoming--;
        if (toBottoming < 0) // 到底部可能会提前中止
            toBottoming = 0;
    });
}

bool CustomListWidget::isToBottoming() const
{
    return toBottoming;
}

void CustomListWidget::addSmoothScrollThread(int distance, int duration)
{
    SmoothScrollBean* bean = new SmoothScrollBean(distance, duration);
    smooth_scrolls.append(bean);
    connect(bean, SIGNAL(signalSmoothScrollDistance(SmoothScrollBean*, int)), this, SLOT(slotSmoothScrollDistance(SmoothScrollBean*, int)));
    connect(bean, &SmoothScrollBean::signalSmoothScrollFinished, [=]{
        delete bean;
        smooth_scrolls.removeOne(bean);
    });
}

void CustomListWidget::slotSmoothScrollDistance(SmoothScrollBean *bean, int dis)
{
    int slide = verticalScrollBar()->sliderPosition();
    slide += dis;
    if (slide < 0)
    {
        slide = 0;
        smooth_scrolls.removeOne(bean);
    }
    else if (slide > verticalScrollBar()->maximum())
    {
        slide = verticalScrollBar()->maximum();
        smooth_scrolls.removeOne(bean);
    }
    verticalScrollBar()->setSliderPosition(slide);
}

void CustomListWidget::wheelEvent(QWheelEvent *event)
{
    if (enabledSmoothScroll)
    {
        if (event->delta() > 0) // 上滚
        {
            if (verticalScrollBar()->sliderPosition() == verticalScrollBar()->minimum() && !smooth_scrolls.size()) // 到顶部了
                emit signalLoadTop();
            addSmoothScrollThread(-smoothScrollSpeed, smoothScrollDuration);
            toBottoming = 0;
        }
        else if (event->delta() < 0) // 下滚
        {
            if (verticalScrollBar()->sliderPosition() == verticalScrollBar()->maximum() && !smooth_scrolls.size()) // 到顶部了
                emit signalLoadBottom();
            addSmoothScrollThread(smoothScrollSpeed, smoothScrollDuration);
        }
    }
    else
    {
        QListWidget::wheelEvent(event);
    }
}

void CustomListWidget::mousePressEvent(QMouseEvent *e)
{
    pressGlobalPos = e->globalPos();
    emit pressedEvent();
    QListWidget::mousePressEvent(e);
}

void CustomListWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton
            && (e->globalPos() - pressGlobalPos).manhattanLength() < QApplication::startDragDistance())
    {
        emit releasedEvent();
    }
    QListWidget::mouseReleaseEvent(e);
}
