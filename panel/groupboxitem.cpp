#include "groupboxitem.h"
#include "pointmenubutton.h"
#include "facilemenu.h"
#include "runtime.h"
#include <QVBoxLayout>
#include <QInputDialog>

GroupBoxItem::GroupBoxItem(QWidget* parent) : ResizeableItemBase(parent)
{
    setType(PanelItemType::GroupBox);
    label = new QLabel(this);
    label->setText(title);
    label->setAlignment(Qt::AlignLeft);

    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    contentWidget = new QWidget(scrollArea);
    scrollArea->setWidget(contentWidget);

    scrollArea->setMinimumSize(0, 0);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(9,9,9,9);
    layout->setSpacing(6);
    layout->setAlignment(Qt::AlignTop);
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    layout->addWidget(label);
    layout->addWidget(scrollArea);
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
    if (isFold())
        json.insert("fold", true);
    json.insert("scroll_height", scrollHeight);
    return json;
}

void GroupBoxItem::fromJson(const MyJson &json)
{
    ResizeableItemBase::fromJson(json);
    title = json.s("title");
    label->setText(title);

    bool fold = json.b("fold");
    if (fold)
    {
        scrollArea->hide();
    }
    scrollHeight = json.i("scroll_height");
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

bool GroupBoxItem::isFold() const
{
    return !scrollArea->isVisible();
}

void GroupBoxItem::facileMenuEvent(FacileMenu *menu)
{
    menu->addAction(QIcon(":/icons/fold"), "折叠/展开", [=]{
            if (isFold())
                unfold();
            else
                fold();
        });
    menu->addAction(QIcon(":/icons/header"), "修改标题", [=]{
            bool ok;
            QString title = QInputDialog::getText(this, "修改标题", "请输入新的标题", QLineEdit::Normal, getTitle(), &ok);
            if (!ok)
                return;
            setTitle(title);
            label->setVisible(!title.isEmpty());
            emit modified();
        });
    menu->split()->addAction(QIcon(":/icons/layout"), "整理", [=]{

        })->disable();
}

void GroupBoxItem::resizeEvent(QResizeEvent *e)
{
    ResizeableItemBase::resizeEvent(e);

    if (!isFold())
        scrollHeight = layout()->spacing() + scrollArea->height();
}

void GroupBoxItem::fold()
{
    if (isFold())
        return;
    if (scrollHeight <= 0)
        scrollHeight = layout()->spacing() + scrollArea->height();
    qInfo() << "折叠分组  高度-" + QString::number(scrollHeight);
    scrollArea->hide();
    QTimer::singleShot(0, this, [=]{
        setGeometry(pos().x(), pos().y(), width(), height() - scrollHeight);
    });
    emit modified();
}

void GroupBoxItem::unfold()
{
    if (!isFold())
        return;
    if (scrollHeight <= 0)
        scrollHeight = layout()->spacing() + scrollArea->height();
    qInfo() << "展开分组  高度+" + QString::number(scrollHeight);
    int currentHeight = height(); // show之后高度会自动变，需要先保存一下
    scrollArea->show();
    QTimer::singleShot(0, this, [=]{
        setGeometry(pos().x(), pos().y(), width(), currentHeight + scrollHeight);
    });
    emit modified();
}
