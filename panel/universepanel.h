#ifndef UNIVERSEPANEL_H
#define UNIVERSEPANEL_H

#include <QObject>
#include <QWidget>
#include "facilemenu.h"
#include "myjson.h"
#include "panelitem.h"

class UniversePanel : public QWidget
{
    Q_OBJECT
public:
    explicit UniversePanel(QWidget *parent = nullptr);
    ~UniversePanel() override;

private:
    void initPanel();
    void readItems();

signals:

public slots:
    void expandPanel();
    void foldPanel();
    void save();

public:
    QRect screenGeometry() const;

protected:
    void closeEvent(QCloseEvent *) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

    void paintEvent(QPaintEvent *) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    void contextMenuEvent(QContextMenuEvent *event) override;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    bool expanding = false;
    bool animating = false;
    bool fixing = false;

    bool pressing = false;
    QPoint pressPos;
    QPoint draggingPos;
    FacileMenu* currentMenu = nullptr;

    QList<PanelItem*> items;
};

#endif // UNIVERSEPANEL_H
