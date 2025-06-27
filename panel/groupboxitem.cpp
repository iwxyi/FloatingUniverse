#include "groupboxitem.h"
#include "pointmenubutton.h"
#include "facilemenu.h"
#include "runtime.h"
#include <QVBoxLayout>

GroupBoxItem::GroupBoxItem(QWidget* parent) : ResizeableItemBase(parent)
{
    setType(PanelItemType::GroupBox);
    label = new QLabel(this);
    PointMenuButton* menu_button = new PointMenuButton(this);
    label->setText(title);
    label->setAlignment(Qt::AlignLeft);

    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    contentWidget = new QWidget(scrollArea);
    scrollArea->setWidget(contentWidget);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(9,9,9,9);
    layout->setSpacing(6);
    layout->setAlignment(Qt::AlignTop);
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    QHBoxLayout* h_layout = new QHBoxLayout();
    h_layout->addWidget(label);
    h_layout->addWidget(menu_button);
    layout->addLayout(h_layout);
    layout->addWidget(scrollArea);

    menu_button->setSquareSize();
    menu_button->setLeaveAfterClick(true);

    connect(menu_button, &InteractiveButtonBase::clicked, this, &GroupBoxItem::slotShowGroupMenu);
}

void GroupBoxItem::initResource()
{
    if (itemId == 0)
    {
        itemId = rt->getRandomId();
        qInfo() << "创建分组，ID=" << itemId;
    }
}

MyJson GroupBoxItem::toJson() const
{
    MyJson json = ResizeableItemBase::toJson();
    json.insert("title", title);
    return json;
}

void GroupBoxItem::fromJson(const MyJson &json)
{
    ResizeableItemBase::fromJson(json);
    title = json.s("title");
    label->setText(title);
}

QString GroupBoxItem::getTitle() const
{
    return title;
}

void GroupBoxItem::setTitle(const QString& title)
{
    this->title = title;
    this->label->setText(title);
}

QWidget *GroupBoxItem::getGroupArea() const
{
    return contentWidget;
}
/**
 * 获取里面的所有子项
 */
QList<PanelItemBase*> GroupBoxItem::getSubItems() const
{
    QList<PanelItemBase*> list;
    emit signalGetSubItems(&list);
    return list;
}

/**
 * 自动排布内部的item
 */
void GroupBoxItem::autoArrange()
{
}

void GroupBoxItem::slotShowGroupMenu()
{
    newFacileMenu;

    menu->addAction("折叠/展开", [=]{

    })->disable();
    menu->addAction("修改标题", [=]{

    })->disable();
    menu->split()->addAction("整理", [=]{

    })->disable();
    menu->addAction("自动整理", [=]{

    })->disable()->check(false);

    menu->exec();
}
