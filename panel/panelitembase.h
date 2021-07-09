#ifndef PANELITEM_H
#define PANELITEM_H

#include <QObject>
#include <QLabel>
#include <QMimeData>
#include "myjson.h"

class FacileMenu;

enum PanelItemType
{
    DefaultItem,
    IconText,
    LocalFile,
    WebUrl,
    LongText,
    ImageView,
    CardView
};

#define UNDEFINED_POS QPoint(-1, -1)
#define ITEM_MIN_SIZE QSize(8, 8)
#define ITEM_MAX_SIZE (this->size() * 0.8)

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
    bool isAutoRaise() const;
    bool isIgnoreSelect() const;

    virtual void facileMenuEvent(FacileMenu* menu);
    virtual void triggerEvent();

    virtual void releaseResource();

signals:
    void triggered();
    void pressed(const QPoint& pos);
    void modified();
    void selectMe();
    void unselectMe();
    void hidePanel();
    void moveItems(QPoint delta);
    void facileMenuUsed(FacileMenu* menu);
    void useFinished();
    void deleteMe();
    void raiseMe();
    void lowerMe();
    void keepPanelFixing();
    void restorePanelFixing();

public slots:
    void setSelect(bool sh, const QPoint& startPos = UNDEFINED_POS);
    void setHover(bool sh, const QPoint& startPos = UNDEFINED_POS);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    virtual void selectEvent(const QPoint& startPos);
    virtual void unselectEvent();
    virtual void hoverEvent(const QPoint& startPos);
    virtual void unhoverEvent();
    virtual void showSelectEdge(const QPoint &startPos);
    virtual void hideSelectEdge();
    virtual bool canDropEvent(const QMimeData *mime);

private:
    QRect getSelectorBorder() const;
    QRect getHalfRect(QRect big) const;

protected:
    QWidget* selectWidget;

    const int selectBorder = 2;
    QPoint pressPos;
    QPoint pressGlobalPos;
    bool dragged = false; // 按压下来有没有拖拽过
    bool selected = false;
    bool hovered = false;

    PanelItemType type;
    bool autoRaise = true; // press自动置顶
    bool ignoreSelect = false; // 作为背景，不允许左键点击打开、拖拽选中，只能右键选择
};

#endif // PANELITEM_H
