#ifndef CLICKLABEL_H
#define CLICKLABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <QApplication>

class ClickLabel : public QLabel
{
    Q_OBJECT
public:
    ClickLabel(QWidget* parent = nullptr) : QLabel(parent)
    {

    }

signals:
    void leftPressed();
    void leftClicked();
    void rightPressed();
    void rightClicked();

protected:
    void mousePressEvent(QMouseEvent *ev) override
    {
        if (ev->button() == Qt::LeftButton)
        {
            emit leftPressed();
        }
        else if (ev->button() == Qt::RightButton)
        {
            emit rightPressed();
        }
        else
        {
            return QLabel::mousePressEvent(ev);
        }

        pressPos = ev->pos();
    }

    void mouseReleaseEvent(QMouseEvent *ev) override
    {
        if ((ev->pos() - pressPos).manhattanLength() > QApplication::startDragDistance())
            return QLabel::mouseReleaseEvent(ev);

        // 单击事件
        if (ev->button() == Qt::LeftButton)
        {
            emit leftClicked();
        }
        else if (ev->button() == Qt::RightButton)
        {
            emit rightClicked();
        }
    }

private:
    QPoint pressPos;
};

#endif // CLICKLABEL_H
