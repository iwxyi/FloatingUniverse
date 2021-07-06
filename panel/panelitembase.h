#ifndef PANELITEM_H
#define PANELITEM_H

#include <QObject>
#include <QLabel>
#include "myjson.h"

class FacileMenu;

enum PanelItemType
{
    DefaultItem,
    IconText,
    LocalFile,
    WebUrl,
    LongText,
    ImageView
};

class PanelItemBase : public QWidget
{
    Q_OBJECT
public:
    explicit PanelItemBase(QWidget *parent);

    virtual MyJson toJson() const;
    virtual void fromJson(const MyJson& json);

    void setType(PanelItemType type);
    PanelItemType getType() const;

    bool isSelected() const;
    bool isHovered() const;
    virtual bool isUsing() const; // 是否正在使用中，临时屏蔽leaveEvent（比如输入法框）
    QRect contentsRect() const;

    virtual void facileMenuEvent(FacileMenu* menu);
    virtual void triggerEvent();

signals:
    void triggered();
    void pressed();
    void selectMe();
    void modified();
    void hidePanel();
    void moveItems(QPoint delta);
    void facileMenuUsed(FacileMenu* menu);
    void useFinished();

public slots:
    void showSelect(bool sh);
    void showHover(bool sh);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    virtual void showEdgeEvent();

protected:
    QLabel* iconLabel;
    QLabel* textLabel;
    QWidget* selectWidget;

    const int selectBorder = 2;
    QPoint pressPos;
    QPoint pressGlobalPos;
    bool dragged = false; // 按压下来有没有拖拽过
    bool selected = false;
    bool hovered = false;

    PanelItemType type;
};

#endif // PANELITEM_H
