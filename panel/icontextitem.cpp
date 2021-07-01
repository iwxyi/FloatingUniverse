#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QFileIconProvider>
#include <QDesktopServices>
#include <QInputDialog>
#include <QFileDialog>
#include "icontextitem.h"
#include "runtime.h"
#include "usettings.h"
#include "facilemenu.h"

IconTextItem::IconTextItem(QWidget *parent) : PanelItemBase(parent)
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

    type = PanelItemType::IconText;
}

MyJson IconTextItem::toJson() const
{
    MyJson json = PanelItemBase::toJson();

    json.insert("icon", iconName);
    json.insert("text", textLabel->text());
    json.insert("link", link);
    json.insert("fastOpen", fastOpen);
    json.insert("openLevel", openLevel);

    return json;
}

void IconTextItem::fromJson(const MyJson &json)
{
    PanelItemBase::fromJson(json);

    // 基础数据
    QString iconName = json.s("icon");
    setIcon(iconName);
    setText(json.s("text"));

    // 扩展数据
    link = json.s("link");
    fastOpen = json.b("fastOpen");
    if (json.contains("openLevel"))
        openLevel = json.i("openLevel");
}

void IconTextItem::setIcon(const QString &iconName)
{
    this->iconName = iconName;
    if (iconName.isEmpty())
    {
        iconLabel->hide();
        return ;
    }
    else
    {
        iconLabel->show();
    }

    QIcon icon(iconName.startsWith(":") ? iconName : rt->ICON_PATH + iconName);
    if (!icon.isNull())
        iconLabel->setPixmap(icon.pixmap(us->pannelItemSize, us->pannelItemSize));
}

void IconTextItem::setText(const QString &text)
{
    textLabel->setText(text);
    this->text = text;

    if (text.isEmpty())
        textLabel->hide();
    else
        textLabel->show();
}

void IconTextItem::setLink(const QString &link, PanelItemType type)
{
    this->link = link;
    if (type != DefaultItem)
        setType(type);

    setToolTip(link);
}

void IconTextItem::setFastOpen(bool fast)
{
    this->fastOpen = fast;
}

QString IconTextItem::getText() const
{
    return text;
}

QString IconTextItem::getIconName() const
{
    return iconName;
}

QString IconTextItem::getLink() const
{
    return link;
}

bool IconTextItem::isFastOpen() const
{
    return fastOpen;
}

void IconTextItem::facileMenuEvent(FacileMenu *menu)
{
    menu->addAction(QIcon(":/icons/rename"), "重命名 (&N)", [=]{
        bool ok = false;
        QString newName = QInputDialog::getText(this, "修改名字", "请输入新的名字", QLineEdit::Normal, text, &ok);
        if (!ok)
            return ;
        this->setText(newName);
        adjustSize();
        emit modified();
    });

    menu->addAction(QIcon(":/icons/link"), "链接 (&L)", [=]{
        bool ok = false;
        QString newLink = QInputDialog::getText(this, "修改链接", "可以是文件路径、网址，点击项目后立即打开", QLineEdit::Normal, link, &ok);
        if (!ok)
            return ;
        setLink(newLink);
        emit modified();
    });

    menu->addAction(QIcon(":/icons/fast_open"), "快速打开 (&F)", [=]{
        fastOpen = !fastOpen;
        emit modified();
    })->check(fastOpen);

    auto levelMenu = menu->addMenu(QIcon(":/icons/open_level"), "加载层数");
    menu->lastAction()->hide(!fastOpen);
    levelMenu->addNumberedActions("%1层", 1, 11, [=](FacileMenuItem* item){
        item->check(item->getText() == QString::number(openLevel) + "层");
    }, [=](int val){
        openLevel = val;
        emit modified();
    });
}

void IconTextItem::triggerEvent()
{
    if (!getLink().isEmpty())
    {
        QString link = getLink();
        if (QFileInfo(link).exists())
        {
            if (QFileInfo(link).isDir() && isFastOpen())
            {
                showFacileDir(link, nullptr, 0);
                return ;
            }
            else
            {
                link = "file:///" +link;
            }
        }
        QDesktopServices::openUrl(link);
    }
}

void IconTextItem::showFacileDir(QString path, FacileMenu *parentMenu, int level)
{
    if (level >= openLevel)
        return ;

    FacileMenu* menu = parentMenu;
    if (!parentMenu)
    {
        menu = new FacileMenu(this);
    }

    auto infos = QDir(path).entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    QFileIconProvider provicer;
    foreach (auto info, infos)
    {
        if (info.isDir())
        {
            auto m = menu->addMenu(provicer.icon(info), info.fileName(), [=]{
                QDesktopServices::openUrl("file:///" + info.absoluteFilePath());
            });
            showFacileDir(info.absoluteFilePath(), m, level+1);
        }
        else
        {
            menu->addAction(provicer.icon(info), info.fileName(), [=]{
                QDesktopServices::openUrl("file:///" + info.absoluteFilePath());
            });
        }
    }

    if (!parentMenu)
    {
        emit facileMenuUsed(menu);
        menu->exec();
    }
}
