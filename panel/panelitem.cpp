#include <QVBoxLayout>
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

    selectWidget = new QWidget(this);
    selectWidget->hide();
    selectWidget->setStyleSheet("background: transparent; border: " + QString::number(selectBorder) + "px solid #FF0000");
}

PanelItem::PanelItem(const QPixmap &pixmap, const QString &text, QWidget *parent)
    : PanelItem(parent)
{
    iconLabel->setPixmap(pixmap);
    textLabel->setText(text);
}

MyJson PanelItem::toJson() const
{
    MyJson json;

    QRect rect(this->geometry());
    json.insert("left", rect.left());
    json.insert("top", rect.top());
    json.insert("width", rect.width());
    json.insert("height", rect.height());

    json.insert("icon", "");
    json.insert("text", textLabel->text());

    return json;
}

void PanelItem::fromJson(MyJson json)
{
    // 位置
    QRect rect(json.i("left"), json.i("top"), json.i("width"), json.i("height"));
    this->move(rect.topLeft());

    // 数据
    QString iconName = json.s("icon");
    QPixmap pixmap(rt->ICON_PATH + iconName);
    if (!pixmap.isNull())
        iconLabel->setPixmap(pixmap);
    textLabel->setText(json.s("text"));
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
