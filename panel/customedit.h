#ifndef CUSTOMEDIT_H
#define CUSTOMEDIT_H

#include <QTextEdit>
#include "third_party/smooth_scroll/smoothscrollbean.h"

class CustomEdit : public QTextEdit
{
    Q_OBJECT
public:
    CustomEdit(QWidget* parent);

    // ---------- edit ----------
signals:
    void focusIn();
    void focusOut();
    void finished();

protected:
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;

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

#endif // CUSTOMEDIT_H
