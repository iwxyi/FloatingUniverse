#ifndef MOVEABLEWIDGET_H
#define MOVEABLEWIDGET_H

#include <QObject>
#include <QWidget>

class MoveableWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MoveableWidget(QWidget *parent = nullptr);

    bool isMoved() const;

signals:
    void dragMoved(QPoint delta);
    void dragReleased();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QPoint prevPressPos;
    bool pressing = false;
    bool moved = false;
};

#endif // MOVEABLEWIDGET_H
