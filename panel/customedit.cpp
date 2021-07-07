#include <QKeyEvent>
#include "customedit.h"
#include "third_party/slim_scroll_bar/slimscrollbar.h"

CustomEdit::CustomEdit(QWidget *parent) : QTextEdit(parent)
{
    auto slim = new SlimScrollBar;
    setVerticalScrollBar(slim);
}

// ---------- edit ----------

void CustomEdit::focusInEvent(QFocusEvent *e)
{
    QTextEdit::focusInEvent(e);

    emit focusIn();
}

void CustomEdit::focusOutEvent(QFocusEvent *e)
{
    QTextEdit::focusOutEvent(e);

    emit focusOut();
}

void CustomEdit::keyPressEvent(QKeyEvent *e)
{
    auto key = e->key();
    if (key == Qt::Key_Escape)
    {
        emit finished();
        return ;
    }

    QTextEdit::keyPressEvent(e);
}

// ---------- scroll ----------

void CustomEdit::setSmoothScrollEnabled(bool e)
{
    this->enabledSmoothScroll = e;
}

void CustomEdit::setSmoothScrollSpeed(int speed)
{
    this->smoothScrollSpeed = speed;
}

void CustomEdit::setSmoothScrollDuration(int duration)
{
    this->smoothScrollDuration = duration;
}

void CustomEdit::scrollToTop()
{
    scrollTo(verticalScrollBar()->minimum());
}

void CustomEdit::scrollTo(int pos)
{
    if (!enabledSmoothScroll)
        return verticalScrollBar()->setSliderPosition(pos);

    auto scrollBar = verticalScrollBar();
    int delta = pos - scrollBar->sliderPosition();
    if (qAbs(delta) <= 1)
        return verticalScrollBar()->setSliderPosition(pos);
    addSmoothScrollThread(delta, smoothScrollDuration);
}

void CustomEdit::scrollToBottom()
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

bool CustomEdit::isToBottoming() const
{
    return toBottoming;
}

void CustomEdit::addSmoothScrollThread(int distance, int duration)
{
    SmoothScrollBean* bean = new SmoothScrollBean(distance, duration);
    smooth_scrolls.append(bean);
    connect(bean, SIGNAL(signalSmoothScrollDistance(SmoothScrollBean*, int)), this, SLOT(slotSmoothScrollDistance(SmoothScrollBean*, int)));
    connect(bean, &SmoothScrollBean::signalSmoothScrollFinished, [=]{
        delete bean;
        smooth_scrolls.removeOne(bean);
    });
}

void CustomEdit::slotSmoothScrollDistance(SmoothScrollBean *bean, int dis)
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

void CustomEdit::wheelEvent(QWheelEvent *event)
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
        QTextEdit::wheelEvent(event);
    }
}
