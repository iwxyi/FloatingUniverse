#include <QHBoxLayout>
#include <QLabel>
#include <QStyleOption>
#include <QInputDialog>
#include <QColorDialog>
#include <QDesktopServices>
#include "settingsitemlistbox.h"
#include "usettings.h"
#include "sapid_switch/boundaryswitchbase.h"
#include "sapid_switch/lovelyheartswitch.h"
#include "sapid_switch/normalswitch.h"
#include "anicirclelabel.h"
#include "aninumberlabel.h"

SettingsItemListBox::SettingsItemListBox(QWidget *parent) : QWidget(parent)
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
}

void SettingsItemListBox::add(QPixmap pixmap, QString text, QString desc)
{
    createBg(pixmap, text, desc);
}

void SettingsItemListBox::add(QPixmap pixmap, QString text, QString desc, QString key, bool *val)
{
    auto btn = createBg(pixmap, text, desc);
    auto layout = static_cast<QHBoxLayout*>(btn->layout());
    auto swtch = new NormalSwitch(*val, btn);
    swtch->setSuitableWidth(us->widgetSize);
    layout->addWidget(swtch);

    auto changed = [=](bool v){
        us->set(key, *val = v);
    };
    connect(btn, &InteractiveButtonBase::clicked, btn, [=]{
        changed(!*val);
        swtch->setState(*val);
    });
    connect(swtch, &SapidSwitchBase::stateChanged, btn, [=](bool v){ changed(v); });
}

void SettingsItemListBox::add(QPixmap pixmap, QString text, QString desc, QString key, int *val, int min, int max, int step)
{
    auto btn = createBg(pixmap, text, desc);
    auto layout = static_cast<QHBoxLayout*>(btn->layout());
    auto label = new AniNumberLabel(*val, btn);
    label->setAlignment(Qt::AlignCenter);
    label->setFixedWidth(us->widgetSize);
    layout->addWidget(label);

    auto changed = [=](int v) {
        us->set(key, *val = v);
    };
    connect(btn, &InteractiveButtonBase::clicked, btn, [=]{
        bool ok = false;
        int x = QInputDialog::getInt(this, desc.isEmpty() ? "输入数值" : text,
                                     desc.isEmpty() ? text : desc,
                                     *val, min, max, step, &ok);
        if (!ok)
            return ;
        changed(x);
        label->setShowNum(x);
    });
}

void SettingsItemListBox::add(QPixmap pixmap, QString text, QString desc, QString key, double *val, double min, double max, double step)
{
    auto btn = createBg(pixmap, text, desc);
    auto layout = static_cast<QHBoxLayout*>(btn->layout());
    auto label = new AniNumberLabel(btn);
    label->setNum(*val);
    label->setAlignment(Qt::AlignCenter);
    label->setFixedWidth(us->widgetSize);
    layout->addWidget(label);

    auto changed = [=](double v) {
        us->set(key, *val = v);
    };
    connect(btn, &InteractiveButtonBase::clicked, btn, [=]{
        bool ok = false;
        double x = QInputDialog::getDouble(this, desc.isEmpty() ? "输入数值" : text,
                                           desc.isEmpty() ? text : desc,
                                           *val, min, max, 2, &ok);
        if (!ok)
            return ;
        changed(x);
        label->setNum(x);
    });
}

void SettingsItemListBox::add(QPixmap pixmap, QString text, QString desc, QString key, QString *val)
{
    auto btn = createBg(pixmap, text, desc);
    auto layout = static_cast<QHBoxLayout*>(btn->layout());
    QLabel* label = new QLabel(*val, btn);
    layout->addWidget(label);

    auto changed = [=](QString v) {
        us->set(key, *val = v);
    };
    connect(btn, &InteractiveButtonBase::clicked, btn, [=]{
        bool ok = false;
        QString s = QInputDialog::getText(this, desc.isEmpty() ? "输入文字" : text,
                                     desc.isEmpty() ? text : desc, QLineEdit::Normal, *val, &ok);
        if (!ok)
            return ;
        changed(s);
        label->setText(s);
    });
}

void SettingsItemListBox::add(QPixmap pixmap, QString text, QString desc, QString key, QColor *val)
{
    auto btn = createBg(pixmap, text, desc);
    auto layout = static_cast<QHBoxLayout*>(btn->layout());
    auto label = new AniCircleLabel(*val, btn);
    layout->addWidget(label);
    label->setFixedWidth(us->widgetSize);

    auto changed = [=](QColor v) {
        us->set(key, *val = v);
    };
    connect(btn, &InteractiveButtonBase::clicked, btn, [=]{
        QColor c = QColorDialog::getColor(*val, this, text, QColorDialog::ShowAlphaChannel);
        if (!c.isValid())
            return ;
        changed(c);
        label->setMainColor(c);
    });
}

void SettingsItemListBox::addOpen(QPixmap pixmap, QString text, QString desc, QString payload)
{
    auto btn = createBg(pixmap, text, desc);
    auto layout = static_cast<QHBoxLayout*>(btn->layout());
    auto label = new QLabel(btn);
    label->setPixmap(QPixmap(":/icons/sub_menu_arrow"));
    label->setScaledContents(true);
    label->setFixedSize(20, 20);
    layout->addWidget(label);
    connect(btn, &InteractiveButtonBase::clicked, btn, [=]{
        emit openPage(payload);
    });
}

void SettingsItemListBox::addOpen(QPixmap pixmap, QString text, QString desc, QUrl url)
{
    auto btn = createBg(pixmap, text, desc);
    auto layout = static_cast<QHBoxLayout*>(btn->layout());
    auto label = new QLabel(btn);
    label->setPixmap(QPixmap(":/icons/st/open_url.png"));
    label->setScaledContents(true);
    label->setFixedSize(20, 20);
    layout->addWidget(label);
    connect(btn, &InteractiveButtonBase::clicked, btn, [=]{
        QDesktopServices::openUrl(url);
    });
}

void SettingsItemListBox::addPage(QPixmap pixmap, QString text, QString desc)
{
    auto btn = createBg(pixmap, text, desc);
    auto layout = static_cast<QHBoxLayout*>(btn->layout());
    auto label = new QLabel(btn);
    label->setPixmap(QPixmap(":/icons/sub_menu_arrow"));
    label->setScaledContents(true);
    label->setFixedSize(20, 20);
    layout->addWidget(label);
}

InteractiveButtonBase *SettingsItemListBox::lastItem() const
{
    return _lastItem;
}

int SettingsItemListBox::setFind(QString key)
{
    const QColor normalColor = Qt::transparent;
    const QColor selectColor = Qt::lightGray;
    if (key.isEmpty())
    {
        foreach (auto item, items)
        {
            item->setNormalColor(normalColor);
        }
        return -1;
    }

    int index = -1;
    for (int i = 0; i < texts.size(); i++)
    {
        if (texts.at(i).contains(key))
        {
            if (index == -1)
                index = i;
            items.at(i)->setNormalColor(selectColor);
        }
        else
        {
            items.at(i)->setNormalColor(normalColor);
        }
    }

    return index;
}

InteractiveButtonBase *SettingsItemListBox::createBg(QPixmap pixmap, QString text, QString desc)
{
    InteractiveButtonBase* btn = new InteractiveButtonBase(this);

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
    hlayout->setStretch(hlayout->count() - 1, 1);
    btn->setFixedHeight(hintHeight += hlayout->margin() * 2);

    if (items.size())
    {
        // 添加分割线
        QWidget* w = new QWidget(this);
        w->setFixedHeight(1);
        w->setStyleSheet("background-color: #20888888");
        mainLayout->addWidget(w);
        w->setCursor(Qt::PointingHandCursor);
    }
    mainLayout->addWidget(btn);
    _lastItem = btn;
    items.append(btn);
    texts.append(text + "\n" + desc);
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
