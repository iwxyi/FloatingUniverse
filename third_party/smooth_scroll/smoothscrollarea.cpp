#include <QScrollBar>
#include <QWheelEvent>
#include "smoothscrollarea.h"

SmoothScrollArea::SmoothScrollArea(QWidget *parent) : QScrollArea(parent)
{

}

void SmoothScrollArea::setSmoothScrollEnabled(bool e)
{
    this->enabledSmoothScroll = e;
}

void SmoothScrollArea::setSmoothScrollSpeed(int speed)
{
    this->smoothScrollSpeed = speed;
}

void SmoothScrollArea::setSmoothScrollDuration(int duration)
{
    this->smoothScrollDuration = duration;
}

void SmoothScrollArea::scrollToTop()
{
    scrollTo(verticalScrollBar()->minimum());
}

void SmoothScrollArea::scrollTo(int pos)
{
    if (!enabledSmoothScroll)
        return verticalScrollBar()->setSliderPosition(pos);

    auto scrollBar = verticalScrollBar();
    int delta = pos - scrollBar->sliderPosition();
    if (qAbs(delta) <= 1)
        return verticalScrollBar()->setSliderPosition(pos);
    addSmoothScrollThread(delta, smoothScrollDuration);
}

void SmoothScrollArea::scrollToBottom()
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

bool SmoothScrollArea::isToBottoming() const
{
    return toBottoming;
}

void SmoothScrollArea::addSmoothScrollThread(int distance, int duration)
{
    SmoothScrollBean* bean = new SmoothScrollBean(distance, duration);
    smooth_scrolls.append(bean);
    connect(bean, SIGNAL(signalSmoothScrollDistance(SmoothScrollBean*, int)), this, SLOT(slotSmoothScrollDistance(SmoothScrollBean*, int)));
    connect(bean, &SmoothScrollBean::signalSmoothScrollFinished, [=]{
        delete bean;
        smooth_scrolls.removeOne(bean);
    });
}

void SmoothScrollArea::slotSmoothScrollDistance(SmoothScrollBean *bean, int dis)
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

void SmoothScrollArea::wheelEvent(QWheelEvent *event)
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
        QScrollArea::wheelEvent(event);
    }
}
