#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QFileIconProvider>
#include <QDesktopServices>
#include <QInputDialog>
#include <QFileDialog>
#include <QProcess>
#include "icontextitem.h"
#include "runtime.h"
#include "usettings.h"
#include "facilemenu.h"
#include "fileutil.h"

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
    json.insert("hide_after_trigger", hideAfterTrigger);
    json.insert("fast_open", fastOpen);
    json.insert("open_level", openLevel);

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
    hideAfterTrigger = json.b("hide_after_trigger", hideAfterTrigger);
    fastOpen = json.b("fast_open", fastOpen);
    openLevel = json.i("open_level", openLevel);
}

/// 释放所有的资源
/// 删除item的时候必须执行，否则变为垃圾文件
void IconTextItem::releaseResource()
{
    PanelItemBase::releaseResource();

    if (!iconName.isEmpty())
    {
        deleteFile(rt->ICON_PATH + iconName);
    }

    if (link.startsWith(rt->PANEL_FILE_PATH) && isFileExist(link))
    {
        qInfo() << "删除文件到回收站：" << link;
        recycleFile(link);
    }
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
        iconLabel->setPixmap(icon.pixmap(us->panelIconSize, us->panelIconSize));
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

void IconTextItem::setLink(const QString &link)
{
    this->link = link;
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

QString IconTextItem::saveIconFile(const QIcon &icon)
{
    return saveIconFile(icon.pixmap(us->panelIconSize, us->panelIconSize));
}

QString IconTextItem::saveIconFile(const QPixmap &pixmap)
{
    // 保存到本地
    int val = 1;
    while (QFileInfo(rt->ICON_PATH + QString::number(val) + ".png").exists())
        val++;
    QString iconName = QString::number(val) + ".png";

    pixmap.save(rt->ICON_PATH + iconName);
    return iconName;
}

/// 左右震动
/// 拒绝 drop
void IconTextItem::shake(int range)
{
    if (_shaking)
        return ;

    int nX = this->x();
    int nY = this->y();
    QPropertyAnimation *ani = new QPropertyAnimation(this, "geometry");
    ani->setEasingCurve(QEasingCurve::InOutSine);
    ani->setDuration(300);
    ani->setStartValue(QRect(QPoint(nX,nY), this->size()));

    int nShakeCount = 20; //抖动次数
    double nStep = 1.0 / nShakeCount;
    for(int i = 1; i < nShakeCount; i++){
        range = i & 1 ? -range : range;
        ani->setKeyValueAt(nStep * i, QRect(QPoint(nX + range, nY), this->size()));
    }

    ani->setEndValue(QRect(QPoint(nX,nY), this->size()));
    ani->start(QAbstractAnimation::DeleteWhenStopped);

    _shaking = true;
    connect(ani, &QPropertyAnimation::stateChanged, this, [=](QAbstractAnimation::State state){
        if (state == QPropertyAnimation::State::Stopped)
        {
            _shaking = false;
        }
    });
}

/// 整体点头
/// 单个 menu
void IconTextItem::nod(int range)
{
    int nX = this->x();
    int nY = this->y();
    QPropertyAnimation *ani = new QPropertyAnimation(this, "geometry");
    ani->setEasingCurve(QEasingCurve::InOutSine);
    ani->setDuration(300);
    ani->setStartValue(QRect(QPoint(nX,nY), this->size()));

    int nShakeCount = 3;
    double nStep = 1.0 / nShakeCount;
    for(int i = 1; i < nShakeCount; i++){
        range = i & 1 ? -range : range;
        ani->setKeyValueAt(nStep * i, QRect(QPoint(nX, nY + range), this->size()));
    }

    ani->setEndValue(QRect(QPoint(nX,nY), this->size()));
    ani->start(QAbstractAnimation::DeleteWhenStopped);

    _nodding = true;
    connect(ani, &QPropertyAnimation::stateChanged, this, [=](QAbstractAnimation::State state){
        if (state == QPropertyAnimation::State::Stopped)
        {
            _nodding = false;
        }
    });
}

/// 图标跳跃
/// trigger
void IconTextItem::jump(int range)
{
    QWidget* w = iconLabel->pixmap() ? static_cast<QWidget*>(iconLabel) : this;
    int nX = w->x();
    int nY = w->y();
    QPropertyAnimation *ani = new QPropertyAnimation(w, "geometry");
    ani->setEasingCurve(QEasingCurve::InOutSine);
    ani->setDuration(500);
    ani->setStartValue(QRect(QPoint(nX,nY), w->size()));

    int nShakeCount = 5;
    double nStep = 1.0 / nShakeCount;
    for(int i = 1; i < nShakeCount; i++){
        range = i & 1 ? -range : range;
        ani->setKeyValueAt(nStep * i, QRect(QPoint(nX, nY + range), w->size()));
    }

    ani->setEndValue(QRect(QPoint(nX,nY), w->size()));
    ani->start(QAbstractAnimation::DeleteWhenStopped);

    _jumping = true;
    connect(ani, &QPropertyAnimation::stateChanged, this, [=](QAbstractAnimation::State state){
        if (state == QPropertyAnimation::State::Stopped)
        {
            _jumping = false;
        }
    });
}

/// 整体缩放
/// drop 修改属性
void IconTextItem::shrink(int range)
{
    int nX = this->x();
    int nY = this->y();
    QPropertyAnimation *ani = new QPropertyAnimation(this, "geometry");
    ani->setEasingCurve(QEasingCurve::InOutSine);
    ani->setDuration(300);
    ani->setStartValue(QRect(QPoint(nX,nY), this->size()));

    int nShakeCount = 3;
    double nStep = 1.0 / nShakeCount;
    for(int i = 1; i < nShakeCount; i++){
        range = i & 1 ? -range : range;
        ani->setKeyValueAt(nStep * i, QRect(nX - range / 2, nY - range / 2, width() + range, height() + range));
    }

    ani->setEndValue(QRect(QPoint(nX,nY), this->size()));
    ani->start(QAbstractAnimation::DeleteWhenStopped);

    layout()->setEnabled(false);
    _shrinking = true;
    connect(ani, &QPropertyAnimation::stateChanged, this, [=](QAbstractAnimation::State state){
        if (state == QPropertyAnimation::State::Stopped)
        {
            layout()->setEnabled(true);
            _shrinking = false;
        }
    });
}

/// 图标抖动
/// hover
/// @param startPos 相对于本控件的鼠标位置，图标向反方向移动
void IconTextItem::jitter(QPoint startPos, int range)
{
    if (_jittering)
        return ;
    if (startPos == UNDEFINED_POS)
        return ;
    if (iconLabel->pixmap()->isNull())
        return ;

    QWidget* w = iconLabel;
    int nX = w->x();
    int nY = w->y();
    QPoint effectPos = startPos - rect().center();
    if (effectPos == QPoint(0, 0))
        return ;
    double xie = sqrt(effectPos.x() * effectPos.x() + effectPos.y() * effectPos.y());
    double dx = effectPos.x() / xie; // 最长边为1的直角三角形的x
    double dy = effectPos.y() / xie; // 同上的y

    QPropertyAnimation *ani = new QPropertyAnimation(iconLabel, "geometry");
    ani->setEasingCurve(QEasingCurve::InOutSine);
    ani->setDuration(300);
    ani->setStartValue(QRect(QPoint(nX,nY), w->size()));

    int nShakeCount = 3;
    double nStep = 1.0 / nShakeCount;
    for(int i = 1; i < nShakeCount; i++){
        range = i & 1 ? -range : range;
        ani->setKeyValueAt(nStep * i, QRect(nX - dx * range, nY - dy * range, w->width(), w->height()));
    }
    ani->setEndValue(QRect(QPoint(nX,nY), w->size()));
    ani->start(QAbstractAnimation::DeleteWhenStopped);

    // 避免重复动画
    _jittering = true;
    this->layout()->setEnabled(false);
    connect(ani, &QPropertyAnimation::stateChanged, this, [=](QAbstractAnimation::State state){
        if (state == QPropertyAnimation::State::Stopped)
        {
            _jittering = false;
            this->layout()->setEnabled(true);
        }
    });
}

void IconTextItem::facileMenuEvent(FacileMenu *menu)
{
    nod();

    menu->addAction(QIcon(":/icons/open"), "打开 (&O)", [=]{
        triggerEvent();
    });

    if (type == LocalFile)
    {
        QFileInfo info(link);
        if (info.exists())
        {
            menu->addAction(QIcon(":/icons/folder"), "打开文件夹", [=]{
                QDesktopServices::openUrl("file:///" + info.dir().path());
                if (hideAfterTrigger)
                    emit hidePanel();
            });
        }
    }

    menu->split()->addAction(QIcon(":/icons/rename"), "重命名 (&N)", [=]{
        menu->close();
        bool ok = false;
        QString oldName = text;
        emit keepPanelFixing();
        QString newName = QInputDialog::getText(this, "修改名字", "请输入新的名字", QLineEdit::Normal, text, &ok);
        emit restorePanelFixing();
        if (!ok)
            return ;
        this->setText(newName);
        adjustSize();

        // 询问是否修改文件名
        if (us->modifyFileNameSync && isFileExist(link))
        {
            QFileInfo info(link);
            if (info.exists())
            {
                QString fileName = info.isDir() ? info.fileName() : info.baseName();
                if (fileName == oldName && canBeFileName(newName))
                {
                    int recentRename = us->i("recent/renameFileSync", 0);
                    recentRename = 0; // 强制默认修改，而不是上一次的选项
                    if ((recentRename = QMessageBox::question(this, "重命名", "是否同步修改文件名", "修改", "取消", nullptr, recentRename)) == 0)
                    {
                        QString newLink;
                        if (info.isDir())
                        {
                            QDir dir(link);
                            dir.cdUp();
                            newLink = dir.absoluteFilePath(newName);
                            if (!renameDir(link, newLink))
                                qWarning() << "重命名失败：" << link << "  ->  " <<newLink;
                        }
                        else
                        {
                            QDir dir(info.absoluteDir());
                            newLink = dir.absoluteFilePath(newName) + (info.suffix().isEmpty() ? "" : "." + info.suffix());
                            if (!renameFile(link, newLink))
                                qWarning() << "重命名失败：" << link << "  ->  " <<newLink;
                        }
                        setLink(newLink);
                    }
                    us->set("recent/renameFileSync", recentRename);
                }
            }
        }

        emit modified();
    });

    menu->addAction(QIcon(":/icons/link"), "链接 (&L)", [=]{
        menu->close();
        bool ok = false;
        emit keepPanelFixing();
        QString newLink = QInputDialog::getText(this, "修改链接", "可以是文件路径、网址，点击项目后立即打开", QLineEdit::Normal, link, &ok);
        emit restorePanelFixing();
        if (!ok)
            return ;
        setLink(newLink);
        emit modified();
    });

    menu->split();

    if (type == LocalFile || type == WebUrl)
    {
        menu->addAction(QIcon(":/icons/eye"), "打开后隐藏", [=]{
            hideAfterTrigger = !hideAfterTrigger;
            emit modified();
        })->check(hideAfterTrigger)->tooltip("打开此项目后，自动隐藏悬浮面板");
    }

    if (type == LocalFile)
    {
        QFileInfo info(link);
        bool isDir = info.exists() && info.isDir();

        if (isDir)
        {
            // 这个命令行显示不了
            menu->addAction(QIcon(), "命令行", [=]{
                QProcess p(this);
                p.setProgram("cmd");
                p.setArguments(QStringList{"/c", "cd", "/d", link});
                p.start();
                p.waitForStarted();
                p.waitForFinished();
            })->hide();

            menu->addAction(QIcon(":/icons/fast_open"), "快速打开", [=]{
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
    }

    auto moreMenu = menu->addMenu(QIcon(":/icons/more"), "更多");

    moreMenu->addAction(QIcon(":/icons/happy"), "更换图标", [=]{
        menu->close();
        emit keepPanelFixing();
        QString prevPath = us->s("recent/iconPath");
        QString path = QFileDialog::getOpenFileName(this, "更换图标", prevPath, tr("Images (*.png *.xpm *.jpg *.jpeg *.gif *.ico)"));
        emit restorePanelFixing();
        if (path.isEmpty())
            return ;
        us->set("recent/iconPath", path);

        // 读取图标并保存
        QPixmap pixmap(path);
        if (pixmap.width() > us->panelIconSize || pixmap.height() > us->panelIconSize)
            pixmap = pixmap.scaled(us->panelIconSize, us->panelIconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        if (!pixmap.save(rt->ICON_PATH + this->iconName))
            qWarning() << "保存新图标失败" << path << "  ->  " << (rt->ICON_PATH + this->iconName);
        iconLabel->setPixmap(pixmap);
        this->adjustSize();
        jump();
    });

    moreMenu->addAction(QIcon(":/icons/update"), "刷新图标", [=]{
        if (!QFileInfo(link).exists())
            return ;
        QIcon icon = QFileIconProvider().icon(QFileInfo(link));
        auto pixmap = icon.pixmap(us->panelIconSize, us->panelIconSize);
        pixmap.save(rt->ICON_PATH + iconName);
        iconLabel->setPixmap(pixmap);
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
        us->set("usage/linkOpenCount", ++us->linkOpenCount);

        jump();
        if (hideAfterTrigger)
            emit hidePanel();
    }
}

bool IconTextItem::canDropEvent(const QMimeData *mime)
{
    if ((mime->hasUrls() && mime->urls().size() == 1)
            || (mime->hasText() && !mime->text().contains("\n")))
    {
        jitter(mapFromGlobal(QCursor::pos()), -10);
        return true;
    }
    shake();
    return false;
}

void IconTextItem::dropEvent(QDropEvent *event)
{
    auto mime = event->mimeData();

    // 设置链接
    auto setMyLink = [=](QString link) {
        emit selectMe();
        setLink(link);
        qInfo() << "setLink:" << link;
        shrink();
    };

    if (mime->hasUrls())
    {
        auto urls = mime->urls();
        if (!urls.size())
            return PanelItemBase::dropEvent(event);
        QUrl url = urls.first();
        if (url.isLocalFile())
        {
            setMyLink(url.toLocalFile());
            setType(PanelItemType::LocalFile);
        }
        else
        {
            setMyLink(url.url());
            setType(PanelItemType::WebUrl);
        }
    }
    else if (mime->hasText())
    {
        QString text = mime->text();
        if (text.contains("\n"))
        {
            shake();
            return PanelItemBase::dropEvent(event);
        }
        setMyLink(text);
        if (isFileExist(text))
            setType(PanelItemType::LocalFile);
        else
            setType(PanelItemType::WebUrl);
    }
    else
    {
        shake();
        return PanelItemBase::dropEvent(event);
    }
    emit modified();
}

void IconTextItem::hoverEvent(const QPoint &startPos)
{
    PanelItemBase::hoverEvent(startPos);

    if (!hovered)
    {
        jitter(startPos);
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
                if (hideAfterTrigger)
                    emit hidePanel();
            });
            showFacileDir(info.absoluteFilePath(), m, level+1);
        }
        else
        {
            menu->addAction(provicer.icon(info), info.fileName(), [=]{
                QDesktopServices::openUrl("file:///" + info.absoluteFilePath());
                if (hideAfterTrigger)
                    emit hidePanel();
            });
        }
    }

    if (!parentMenu)
    {
        emit facileMenuUsed(menu);
        menu->exec();
    }
}
