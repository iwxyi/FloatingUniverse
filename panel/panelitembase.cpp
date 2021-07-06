#include <QIcon>
#include <QMouseEvent>
#include <QApplication>
#include <QDebug>
#include <QLayout>
#include "panelitembase.h"
#include "usettings.h"
#include "runtime.h"

PanelItemBase::PanelItemBase(QWidget *parent) : QWidget(parent)
{
    selectWidget = new QWidget(this);
    selectWidget->hide();

    setFocusPolicy(Qt::ClickFocus);
    setCursor(Qt::PointingHandCursor);
    type = PanelItemType::DefaultItem;
}

MyJson PanelItemBase::toJson() const
{
    MyJson json;

    QRect rect(this->geometry());
    json.insert("left", rect.left());
    json.insert("top", rect.top());
    json.insert("width", rect.width());
    json.insert("height", rect.height());
    json.insert("type", int(type));

    return json;
}

void PanelItemBase::fromJson(const MyJson &json)
{
    // 位置
    QRect rect(json.i("left"), json.i("top"), json.i("width"), json.i("height"));
    move(rect.topLeft());

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

void PanelItemBase::facileMenuEvent(FacileMenu *menu)
{
    Q_UNUSED(menu)
}

void PanelItemBase::triggerEvent()
{

}

void PanelItemBase::showSelect(bool sh)
{
    if (sh)
    {
        selectWidget->setStyleSheet("background: transparent; border: " + QString::number(selectBorder) + "px solid " + QVariant(us->panelSelectEdge).toString() + "; border-radius: " + QString::number(us->fluentRadius) + "px; ");
        selectWidget->setGeometry(selectBorder / 2, selectBorder / 2, width() - selectBorder, height() - selectBorder);
        selectWidget->raise();
        selectWidget->show();

        showEdgeEvent();
    }
    else
    {
        selectWidget->hide();
    }
    selected = sh;
}

void PanelItemBase::showHover(bool sh)
{
    if (sh)
    {
        selectWidget->setStyleSheet("background: transparent; border: " + QString::number(selectBorder) + "px solid " + QVariant(us->panelHoverEdge).toString() + "; border-radius: " + QString::number(us->fluentRadius) + "px; ");
        selectWidget->setGeometry(selectBorder / 2, selectBorder / 2, width() - selectBorder, height() - selectBorder);
        selectWidget->raise();
        selectWidget->show();
    }
    else
    {
        selectWidget->hide();
    }
    hovered = sh;
}

void PanelItemBase::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        pressPos = event->pos();
        pressGlobalPos = pressPos + this->pos();
        dragged = false;
        event->accept();
        emit pressed();
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
        showSelect(true);
    }
}

void PanelItemBase::showEdgeEvent()
{

}

