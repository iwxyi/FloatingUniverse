#include <QIcon>
#include <QMouseEvent>
#include <QApplication>
#include <QDebug>
#include <QLayout>
#include <QPropertyAnimation>
#include <QStyleOption>
#include <QPainter>
#include "panelitembase.h"
#include "usettings.h"
#include "runtime.h"

bool PanelItemBase::_blockPress = false;

PanelItemBase::PanelItemBase(QWidget *parent) : QWidget(parent)
{
    setObjectName("ItemBase");
    setFocusPolicy(Qt::ClickFocus);
    setCursor(Qt::PointingHandCursor);
    setAcceptDrops(true);
    setType(PanelItemType::DefaultItem);
    setMinimumSize(ITEM_MIN_SIZE);

    selectWidget = new QWidget(this);
    selectWidget->hide();
    selectWidget->setObjectName("SelectEdge");
}

MyJson PanelItemBase::toJson() const
{
    MyJson json;
    if (itemId != 0)
        json.insert("id", this->itemId);
    if (groupId != 0)
        json.insert("group_id", this->groupId);

    QRect rect(this->geometry());
    json.insert("left", rect.left());
    json.insert("top", rect.top());
    json.insert("type", int(type));

    if (!customQss.isEmpty())
        json.insert("qss", customQss);
    if (isSelected())
        json.insert("selected", true);

    return json;
}

void PanelItemBase::fromJson(const MyJson &json)
{
    // 位置
    this->itemId = json.l("id", 0);
    this->groupId = json.l("group_id", 0);
    move(QPoint(json.i("left"), json.i("top")));

    // 类型
    this->type = PanelItemType(json.i("type"));

    customQss = json.s("qss");
    if (!customQss.isEmpty())
        setStyleSheet(customQss);
}

qint64 PanelItemBase::getItemId() const
{
    return itemId;
}

void PanelItemBase::setType(PanelItemType type)
{
    this->type = type;
}

PanelItemType PanelItemBase::getType() const
{
    return type;
}

void PanelItemBase::setGroupId(qint64 id)
{
    this->groupId = id;
    emit modified();
}

qint64 PanelItemBase::getGroupId() const
{
    return groupId;
}

bool PanelItemBase::isSelected() const
{
    return selected;
}

bool PanelItemBase::isHovered() const
{
    return hovered;
}

bool PanelItemBase::isUsing() const
{
    return false;
}

QRect PanelItemBase::contentsRect() const
{
    int border = layout() ? layout()->margin() : selectBorder;
    return QRect(border, border, width() - border * 2, height() * border * 2);
}

bool PanelItemBase::isAutoRaise() const
{
    return autoRaise;
}

bool PanelItemBase::isIgnoreSelect() const
{
    return ignoreSelect;
}

void PanelItemBase::setCustomQss(const QString& qss)
{
    customQss = qss;
    setStyleSheet(qss);
}

QString PanelItemBase::getCustomQss() const
{
    return customQss;
}

void PanelItemBase::facileMenuEvent(FacileMenu *menu)
{
    Q_UNUSED(menu)
}

void PanelItemBase::triggerEvent()
{

}

/// 每次创建时初始化资源
void PanelItemBase::initResource()
{

}

/// 删除item时释放资源
void PanelItemBase::releaseResource()
{

}

void PanelItemBase::setSelect(bool sh, const QPoint &startPos)
{
    if (selected == sh)
        return ;

    if (sh)
    {
        selectEvent(startPos);
        hovered = false; // 强制关闭hover状态
    }
    else
    {
        unselectEvent();
    }
    selected = sh;
}

void PanelItemBase::setHover(bool sh, const QPoint &startPos)
{
    if (hovered == sh)
        return ;

    if (sh)
    {
        hoverEvent(startPos);
    }
    else
    {
        unhoverEvent();
    }
    hovered = sh;
}

void PanelItemBase::mousePressEvent(QMouseEvent *event)
{
    // 动画中，忽略按下操作
    if (_blockPress)
        return ;

    if (event->button() == Qt::LeftButton)
    {
        // 不被选中，跳过单击选择
        //if (ignoreSelect) return ; // 不用管，transparentMouse自动跳过了

        pressPos = event->pos();
        pressGlobalPos = pressPos + this->pos();
        dragged = false;
        event->accept();
        emit pressed(event->pos());
        return ;
    }

    QWidget::mousePressEvent(event);
}

void PanelItemBase::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (event->modifiers() & Qt::ControlModifier) // 多选，不进行操作
            return ;

        if (dragged) // 拖拽移动结束
        {
            emit moveFinished();
            emit modified();
        }
        else // 点击事件
        {
            if (!isSelected()) // 鼠标按下，但是ESC键取消了
                return ;
            emit triggered();
        }
        return ;
    }

    QWidget::mouseReleaseEvent(event);
}

void PanelItemBase::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        QPoint globalPos = event->pos() + this->pos();
        QPoint delta = globalPos - pressGlobalPos;

        if (!dragged) // 第一次拖拽
        {
            if (delta.manhattanLength() > QApplication::startDragDistance())
                dragged = true;
        }

        if (dragged)
        {
            // 拖拽
            emit moveItems(delta);
            pressGlobalPos = globalPos;
        }
        return ;
    }
    // 这里右键是穿透了的，自己不监听
    // 右键菜单都是有画板来管理

    QWidget::mouseMoveEvent(event);
}

void PanelItemBase::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (!selectWidget->isHidden())
    {
        selectWidget->setGeometry(getSelectorBorder());
    }
}

void PanelItemBase::dragEnterEvent(QDragEnterEvent *event)
{
    if (canDropEvent(event->mimeData()))
    {
        setHover(true);
    }
    event->accept();
}

void PanelItemBase::dragLeaveEvent(QDragLeaveEvent *event)
{
    setHover(false);
    event->accept();
}

void PanelItemBase::dropEvent(QDropEvent *event)
{
    QWidget::dropEvent(event);
}

void PanelItemBase::paintEvent(QPaintEvent *e)
{
    QStyleOption option;
    option.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &option, &p, this);
}

void PanelItemBase::selectEvent(const QPoint &startPos)
{
    if ((selectWidget->isHidden() && startPos != UNDEFINED_POS) || !selected)
    {
        showSelectEdge(startPos);
    }
    else if (selectWidget->isHidden())
    {
        selectWidget->setGeometry(getSelectorBorder());
    }

    if (!customQss.contains("#SelectEdge"))
        selectWidget->setStyleSheet("background: transparent; border: " + QString::number(selectBorder) + "px solid " + QVariant(us->panelSelectEdgeColor).toString() + "; border-radius: " + QString::number(us->fluentRadius) + "px; ");
    selectWidget->raise();
    selectWidget->show();
}

void PanelItemBase::unselectEvent()
{
    hideSelectEdge();
}

void PanelItemBase::hoverEvent(const QPoint &startPos)
{
    if ((selectWidget->isHidden() && startPos != UNDEFINED_POS) || !hovered)
    {
        showSelectEdge(startPos);
    }
    else if (selectWidget->isHidden())
    {
        selectWidget->setGeometry(getSelectorBorder());
    }

    if (!customQss.contains("#SelectEdge"))
        selectWidget->setStyleSheet("background: transparent; border: " + QString::number(selectBorder) + "px solid " + QVariant(us->panelHoverEdgeColor).toString() + "; border-radius: " + QString::number(us->fluentRadius) + "px; ");
    selectWidget->raise();
    selectWidget->show();
}

void PanelItemBase::unhoverEvent()
{
    hideSelectEdge();
}

void PanelItemBase::showSelectEdge(const QPoint& startPos)
{
    QPropertyAnimation* ani = new QPropertyAnimation(selectWidget, "geometry");
    ani->setStartValue(selectWidget->isHidden() && startPos != UNDEFINED_POS ? QRect(startPos, QSize(1, 1)) : selectWidget->geometry());
    ani->setEndValue(getSelectorBorder());
    ani->setDuration(150);
    ani->setEasingCurve(QEasingCurve::OutCubic);
    ani->start(QAbstractAnimation::DeleteWhenStopped);
}

void PanelItemBase::hideSelectEdge()
{
    if (selectWidget->isHidden())
        return ;
    QPropertyAnimation* ani = new QPropertyAnimation(selectWidget, "geometry");
    ani->setStartValue(selectWidget->geometry());
    // ani->setEndValue(QRect(this->rect().center(), QSize(1, 1)));
    ani->setEndValue(getHalfRect(selectWidget->geometry()));
    ani->setDuration(100);
    ani->setEasingCurve(QEasingCurve::InExpo);
    ani->start(QAbstractAnimation::DeleteWhenStopped);
    connect(ani, &QPropertyAnimation::finished, this, [=]{
        selectWidget->hide();
    });
}

bool PanelItemBase::canDropEvent(const QMimeData *mime)
{
    Q_UNUSED(mime)
    return false;
}

QRect PanelItemBase::getSelectorBorder() const
{
    return QRect(selectBorder / 2, selectBorder / 2, width() - selectBorder, height() - selectBorder);
}

QRect PanelItemBase::getHalfRect(QRect big) const
{
    double prop = 0.5;
    QRectF rect(
                big.left() + big.width() * prop / 2,
                big.top() + big.height() * prop / 2,
                big.width() * prop,
                big.height() * prop
                );
    return rect.toRect();
}

