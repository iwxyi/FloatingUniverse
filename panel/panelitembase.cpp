#include <QIcon>
#include <QMouseEvent>
#include <QApplication>
#include <QDebug>
#include <QLayout>
#include <QPropertyAnimation>
#include "panelitembase.h"
#include "usettings.h"
#include "runtime.h"

PanelItemBase::PanelItemBase(QWidget *parent) : QWidget(parent)
{
    selectWidget = new QWidget(this);
    selectWidget->hide();

    setFocusPolicy(Qt::ClickFocus);
    setCursor(Qt::PointingHandCursor);
    setAcceptDrops(true);
    type = PanelItemType::DefaultItem;

    setMinimumSize(ITEM_MIN_SIZE);
}

MyJson PanelItemBase::toJson() const
{
    MyJson json;

    QRect rect(this->geometry());
    json.insert("left", rect.left());
    json.insert("top", rect.top());
    json.insert("type", int(type));

    return json;
}

void PanelItemBase::fromJson(const MyJson &json)
{
    // 位置
    move(QPoint(json.i("left"), json.i("top")));

    // 类型
    this->type = PanelItemType(json.i("type"));
}

void PanelItemBase::setType(PanelItemType type)
{
    this->type = type;
}

PanelItemType PanelItemBase::getType() const
{
    return type;
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

void PanelItemBase::facileMenuEvent(FacileMenu *menu)
{
    Q_UNUSED(menu)
}

void PanelItemBase::triggerEvent()
{

}

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

    selectWidget->setStyleSheet("background: transparent; border: " + QString::number(selectBorder) + "px solid " + QVariant(us->panelSelectEdge).toString() + "; border-radius: " + QString::number(us->fluentRadius) + "px; ");
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

    selectWidget->setStyleSheet("background: transparent; border: " + QString::number(selectBorder) + "px solid " + QVariant(us->panelHoverEdge).toString() + "; border-radius: " + QString::number(us->fluentRadius) + "px; ");
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

