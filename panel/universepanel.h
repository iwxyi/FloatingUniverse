#ifndef UNIVERSEPANEL_H
#define UNIVERSEPANEL_H

#include <QObject>
#include <QWidget>
#include "facilemenu.h"

class UniversePanel : public QWidget
{
    Q_OBJECT
public:
    explicit UniversePanel(QWidget *parent = nullptr);

signals:

public slots:
    void initPanel();
    void expandPanel();
    void foldPanel();

public:
    QRect screenGeometry() const;

protected:
    void closeEvent(QCloseEvent *) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    bool expanding = false;
    bool animating = false;

    bool pressing = false;
    QPoint pressPos;
    QPoint draggingPos;
    FacileMenu* currentMenu = nullptr;
};

#endif // UNIVERSEPANEL_H
