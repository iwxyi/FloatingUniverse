#include <QVBoxLayout>
#include <QIcon>
#include <QMouseEvent>
#include <QApplication>
#include <QDebug>
#include "panelitem.h"
#include "usettings.h"
#include "runtime.h"

PanelItem::PanelItem(QWidget *parent) : QWidget(parent)
{
    iconLabel = new QLabel(this);
    textLabel = new QLabel(this);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(iconLabel);
    layout->addWidget(textLabel);

    iconLabel->setScaledContents(false);
    iconLabel->setAlignment(Qt::AlignCenter);
    textLabel->setAlignment(Qt::AlignCenter);
    textLabel->setWordWrap(true);

    // textLabel->setMaximumWidth(us->pannelItemSize);

    selectWidget = new QWidget(this);
    selectWidget->hide();
    selectWidget->setStyleSheet("background: transparent; border: " + QString::number(selectBorder) + "px solid " + QVariant(us->panelSelectEdge).toString() + ";");

    setCursor(Qt::PointingHandCursor);
}

MyJson PanelItem::toJson() const
{
    MyJson json;

    QRect rect(this->geometry());
    json.insert("left", rect.left());
    json.insert("top", rect.top());
    json.insert("width", rect.width());
    json.insert("height", rect.height());

    json.insert("icon", iconName);
    json.insert("text", textLabel->text());

    if (!link.isEmpty())
        json.insert("link", link);

    return json;
}

PanelItem *PanelItem::fromJson(const MyJson &json, QWidget *parent)
{
    PanelItem* item = new PanelItem(parent);

    // 位置
    QRect rect(json.i("left"), json.i("top"), json.i("width"), json.i("height"));
    item->move(rect.topLeft());

    // 基础数据
    QString iconName = json.s("icon");
    item->setIcon(iconName);
    item->setText(json.s("text"));

    // 扩展数据
    item->link = json.s("link");

    return item;
}

void PanelItem::setIcon(const QString &iconName)
{
    if (iconName.isEmpty())
        return ;
    this->iconName = iconName;
    QIcon icon(iconName.startsWith(":") ? iconName : rt->ICON_PATH + iconName);
    if (!icon.isNull())
        iconLabel->setPixmap(icon.pixmap(us->pannelItemSize, us->pannelItemSize));
}

void PanelItem::setText(const QString &text)
{
    textLabel->setText(text);
    this->text = text;
}

void PanelItem::setLink(const QString &link)
{
    this->link = link;
}

void PanelItem::showSelect(bool sh)
{
    if (sh)
    {
        selectWidget->setGeometry(selectBorder / 2, selectBorder / 2, width() - selectBorder, height() - selectBorder);
        selectWidget->raise();
        selectWidget->show();
    }
    else
    {
        selectWidget->hide();
    }
}

void PanelItem::mousePressEvent(QMouseEvent *event)
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

void PanelItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (dragged) // 拖拽移动结束
        {
            emit needSave();
        }
        else // 点击事件
        {
            emit triggered();
        }
        return ;
    }

    QWidget::mouseReleaseEvent(event);
}

void PanelItem::mouseMoveEvent(QMouseEvent *event)
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

void PanelItem::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (!selectWidget->isHidden())
    {
        showSelect(true);
    }
}

