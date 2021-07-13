#include <QHBoxLayout>
#include <QLabel>
#include <QStyleOption>
#include "settingsitemlistbox.h"
#include "usettings.h"

SettingsItemListBox::SettingsItemListBox(QWidget *parent) : QWidget(parent)
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
}

void SettingsItemListBox::add(QPixmap pixmap, QString text, QString desc, bool *val)
{
    auto btn = createBg(pixmap, text, desc);

}

void SettingsItemListBox::add(QPixmap pixmap, QString text, QString desc, int *val)
{
    auto btn = createBg(pixmap, text, desc);
}

void SettingsItemListBox::add(QPixmap pixmap, QString text, QString desc, QString *val)
{
    auto btn = createBg(pixmap, text, desc);
}

void SettingsItemListBox::add(QPixmap pixmap, QString text, QString desc, QColor *val)
{
    auto btn = createBg(pixmap, text, desc);
}

void SettingsItemListBox::addOpen(QPixmap pixmap, QString text, QString desc)
{
    auto btn = createBg(pixmap, text, desc);
}

InteractiveButtonBase *SettingsItemListBox::createBg(QPixmap pixmap, QString text, QString desc)
{
    InteractiveButtonBase* btn = new InteractiveButtonBase(this);
    items.append(btn);

    btn->setCursor(Qt::PointingHandCursor);

    const int iconSpacing = 16;
    auto hlayout = new QHBoxLayout(btn);
    hlayout->setMargin(12);
    hlayout->setSpacing(iconSpacing);
    hlayout->addSpacing(iconSpacing - hlayout->margin() + 2);

    QLabel* titleLabel = new QLabel(text, btn);
    titleLabel->adjustSize();
    int sz = titleLabel->height();
    int hintHeight = sz;

    if (!pixmap.isNull())
    {
        QLabel* label = new QLabel(btn);
        label->setScaledContents(true);
        label->setPixmap(pixmap);
        label->setFixedSize(sz, sz);
        hlayout->addWidget(label);
    }

    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->addWidget(titleLabel);
    vlayout->setSpacing(0);
    if (!desc.isEmpty())
    {
        QLabel* descLabel = new QLabel(desc, btn);
        vlayout->addWidget(descLabel);
        descLabel->setStyleSheet("color: #666");
        descLabel->adjustSize();
        hintHeight += descLabel->height() + vlayout->spacing();
    }
    hlayout->addLayout(vlayout);
    btn->setFixedHeight(hintHeight += hlayout->margin() * 2);
    mainLayout->addWidget(btn);
    return btn;
}

void SettingsItemListBox::adjusItemsSize()
{

}

void SettingsItemListBox::paintEvent(QPaintEvent *)
{
    // 自定义控件的QSS无效，需要使用这个办法
    QStyleOption option;
    option.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &option, &p, this);

}
