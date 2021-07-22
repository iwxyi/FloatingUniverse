#include <QDebug>
#include <QStyleOption>
#include <QPainter>
#include <QPainterPath>
#include <QColorDialog>
#include <QInputDialog>
#include "carditem.h"
#include "facilemenu.h"
#include "usettings.h"

CardItem::CardItem(QWidget *parent) : ResizeableItemBase(parent)
{
    setType(PanelItemType::CardView);
}

MyJson CardItem::toJson() const
{
    MyJson json = ResizeableItemBase::toJson();

    json.insert("color", QVariant(bg).toString());
    json.insert("radius", radius);
    json.insert("bottom_layer", !autoRaise);
    json.insert("ignore_select", ignoreSelect);

    return json;
}

void CardItem::fromJson(const MyJson &json)
{
    ResizeableItemBase::fromJson(json);

    bg = QColor(json.s("color"));
    radius = json.i("radius", radius);
    autoRaise = !json.b("bottom_layer");
    ignoreSelect = json.b("ignore_select", ignoreSelect);

    if (ignoreSelect)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
    }
}

void CardItem::setColor(QColor c)
{
    this->bg = c;
    emit modified();
    update();
}

void CardItem::setRadius(int r)
{
    this->radius = r;
    emit modified();
    update();
}

void CardItem::paintEvent(QPaintEvent *)
{
    if (!customQss.isEmpty())
    {
        QStyleOption option;
        option.init(this);
        QPainter p(this);
        style()->drawPrimitive(QStyle::PE_Widget, &option, &p, this);
        return ;
    }

    QPainterPath path;
    path.addRoundedRect(rect(), radius, radius);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillPath(path, bg);
}

void CardItem::facileMenuEvent(FacileMenu *menu)
{
    ResizeableItemBase::facileMenuEvent(menu);

    menu->addAction(QIcon(":/icons/select_color"), "设置颜色", [=]{
        emit keepPanelFixing();
        QColor c = QColorDialog::getColor(bg, this, "选择颜色", QColorDialog::ShowAlphaChannel);
        emit restorePanelFixing();
        if (!c.isValid())
            return ;
        setColor(c);
        us->setValue("recent/cardColor", c);
    });

    menu->addAction(QIcon(":/icons/round_corner"), "设置圆角", [=]{
        emit keepPanelFixing();
        bool ok = false;
        int r = QInputDialog::getInt(this, "设置圆角", "请输入圆角半径", radius, 0, 0x3f3f3f3f, 5, &ok);
        emit restorePanelFixing();
        if (!ok)
            return ;
        setRadius(r);
        us->setValue("recent/cardRadius", r);
    });

    menu->split()->addAction(QIcon(":/icons/bottom_layer"), "置于底层", [=]{
        autoRaise = !autoRaise;
        if (!autoRaise)
            emit lowerMe();
        else
            emit modified();
    })->check(!autoRaise)->tooltip("放到所有项目的最底下");

    menu->addAction(QIcon(":/icons/ignore_select"), "不被选中", [=]{
        ignoreSelect = !ignoreSelect;
        emit modified();
        setAttribute(Qt::WA_TransparentForMouseEvents, ignoreSelect);
        if (isSelected())
            emit unselectMe();
    })->check(ignoreSelect)->tooltip("屏蔽鼠标左键点击或者框选；不影响显示右键菜单");
}
