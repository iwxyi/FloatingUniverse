#ifndef SLIMSCROLLBARPOPUP_H
#define SLIMSCROLLBARPOPUP_H

#include <QObject>
#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QDebug>

class SlimScrollBarPopup : public QWidget
{
    Q_OBJECT
    friend class SlimScrollBar;
public:
    SlimScrollBarPopup(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;

signals:

public slots:

private:
    QPixmap scrollbar_pixmap;
    QPoint offset;
};

#endif // SLIMSCROLLBARPOPUP_H
