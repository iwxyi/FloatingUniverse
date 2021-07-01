#include <QPropertyAnimation>
#include <QApplication>
#include <QScreen>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QDateTime>
#include <QMouseEvent>
#include <QMimeData>
#include <QFileIconProvider>
#include <QDesktopServices>
#include <QInputDialog>
#include <QFileDialog>
#include "universepanel.h"
#include "runtime.h"
#include "usettings.h"
#include "signaltransfer.h"
#include "facilemenu.h"
#include "fileutil.h"

UniversePanel::UniversePanel(QWidget *parent) : QWidget(parent)
{
    setObjectName("UniversePanel");
    setWindowTitle("悬浮宇宙");
    setMinimumSize(45,45);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setWindowFlag(Qt::WindowStaysOnTopHint, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAutoFillBackground(true);
    setAcceptDrops(true);

    initPanel();
}

UniversePanel::~UniversePanel()
{
    save();
}

/// 初始化面板所有数据
void UniversePanel::initPanel()
{
    QRect screen = screenGeometry();
    setFixedSize(us->panelWidth, us->panelHeight);
    move((screen.width() - width()) / 2 + us->panelCenterOffset, -height() + us->panelBangHeight); //

    readItems();
}

/// 读取设置
void UniversePanel::readItems()
{
    // 清理当前
    foreach (auto item, items)
    {
        item->deleteLater();
    }
    items.clear();

    // 读取设置
    MyJson json(readTextFile(rt->PANEL_PATH).toUtf8());
    QJsonArray array = json.a("items");
    foreach (auto ar, array)
    {
        auto item = PanelItem::fromJson(ar.toObject(), this);
        items.append(item);
        item->show();
        connectItem(item);
    }
}

PanelItem *UniversePanel::createNewItem(QPoint pos, const QIcon &icon, const QString &text)
{
    QString iconName = saveIcon(icon);
    return createNewItem(pos, iconName, text);
}

PanelItem *UniversePanel::createNewItem(QPoint pos, const QString& iconName, const QString &text)
{
    auto item = new PanelItem(this);
    // item->textLabel->setMaximumWidth(us->pannelItemSize);
    item->setIcon(iconName);
    item->setText(text);
    item->move(pos);
    items.append(item);
    item->show();
    connectItem(item);

    save();
    return item;
}

void UniversePanel::connectItem(PanelItem *item)
{
    connect(item, &PanelItem::triggered, this, [=]{
        triggerItem(item);
    });

    connect(item, &PanelItem::pressed, this, [=]{
        if (!selectedItems.contains(item))
        {
            unselectAll();
            selectItem(item);
        }
    });

    connect(item, &PanelItem::needSave, this, [=]{
        save();
    });

    connect(item, &PanelItem::moveItems, this, [=](QPoint delta) {
        foreach (auto item, selectedItems)
        {
            item->move(item->pos() + delta);
        }
    });

}

QString UniversePanel::saveIcon(const QIcon &icon) const
{
    // 保存到本地
    int val = 1;
    while (QFileInfo(rt->ICON_PATH + QString::number(val) + ".png").exists())
        val++;
    QString iconName = QString::number(val) + ".png";

    QPixmap pixmap = icon.pixmap(us->pannelItemSize, us->pannelItemSize);
    pixmap.save(rt->ICON_PATH + iconName);
    return iconName;
}

void UniversePanel::deleteItem(PanelItem *item)
{
    QString iconName = item->iconName;
    if (!iconName.isEmpty())
    {
        deleteFile(rt->ICON_PATH + iconName);
    }
    item->deleteLater();
}

/// 从收起状态展开面板
void UniversePanel::expandPanel()
{
    QPropertyAnimation* ani = new QPropertyAnimation(this, "pos");
    ani->setStartValue(pos());
    ani->setEndValue(QPoint(pos().x(), 0));
    ani->setDuration(300);
    ani->setEasingCurve(QEasingCurve::OutQuad);
    connect(ani, &QPropertyAnimation::finished, this, [=]{
        ani->deleteLater();
        animating = false;
        update();
    });
    ani->start();
    expanding = true;
    animating = true;
}

/// 从显示状态收起面板
void UniversePanel::foldPanel()
{
    if (fixing) // 固定不隐藏
        return ;

    QPropertyAnimation* ani = new QPropertyAnimation(this, "pos");
    ani->setStartValue(pos());
    ani->setEndValue(QPoint(pos().x(), -height() + us->panelBangHeight));
    ani->setDuration(300);
    ani->setEasingCurve(QEasingCurve::OutQuad);

    connect(ani, &QPropertyAnimation::finished, this, [=]{
        ani->deleteLater();
        animating = false;
        update();
    });
    ani->start();
    expanding = false;
    animating = true;
}

void UniversePanel::save()
{
    MyJson json;
    QJsonArray array;
    foreach (auto item, items)
    {
        array.append(item->toJson());
    }
    json.insert("items", array);
    writeTextFile(rt->PANEL_PATH, json.toBa());
}

void UniversePanel::selectAll()
{
    foreach (auto item, items)
        item->showSelect(true);
    selectedItems = items.toSet();
}

void UniversePanel::unselectAll()
{
    foreach (auto item, selectedItems)
        item->showSelect(false);
    selectedItems.clear();
}

void UniversePanel::selectItem(PanelItem *item)
{
    item->showSelect(true);
    selectedItems.insert(item);
}

void UniversePanel::triggerItem(PanelItem *item)
{
    if (!item->link.isEmpty())
    {
        QString link = item->link;
        if (QFileInfo(link).exists())
            link = "file:///" +link;
        QDesktopServices::openUrl(link);
    }
}

/// 选中多项，开始拖拽
void UniversePanel::startDragSelectedItems()
{

}

QRect UniversePanel::screenGeometry() const
{
    auto screens = QGuiApplication::screens();
    int index = 0;
    if (index >= screens.size())
        index = screens.size() - 1;
    if (index < 0)
        return QRect();
    return screens.at(index)->geometry();
}

void UniversePanel::closeEvent(QCloseEvent *)
{
    return ;
}

void UniversePanel::enterEvent(QEvent *event)
{
    QWidget::enterEvent(event);

    if (!expanding)
        expandPanel();
}

void UniversePanel::leaveEvent(QEvent *event)
{
    if (currentMenu && currentMenu->hasFocus())
        return ;

    QWidget::leaveEvent(event);

    if (expanding)
        foldPanel();
}

void UniversePanel::focusOutEvent(QFocusEvent *event)
{
    if (pressing)
    {
        // 拖拽的时候突然失去焦点
        pressing = false;
        update();
    }

    QWidget::focusOutEvent(event);
}

void UniversePanel::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 画主面板
    {
        QPainterPath path;
        path.addRoundedRect(QRect(0, 0, width(), height() - us->panelBangHeight), us->fluentRadius, us->fluentRadius);
        painter.fillPath(path, us->panelBg);
    }

    // 画刘海
    {
        QPainterPath path;
        path.addRect(QRect((width() - us->panelBangWidth) / 2, height() - us->panelBangHeight, us->panelBangWidth, us->panelBangHeight));
        painter.fillPath(path, us->panelBangBg);
    }

    // 画选中
    if (pressing)
    {
        if (pressPos != draggingPos)
        {
            QRect rect(pressPos, draggingPos);
            QPainterPath path;
            path.addRoundedRect(rect, us->fluentRadius, us->fluentRadius);
            painter.fillPath(path, us->panelSelectBg);
        }
    }
}

void UniversePanel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        pressing = true;
        pressPos = draggingPos = event->pos();

        // 判断点击位置
        bool inSelectItem = false; // 点在选中图形上
        foreach (auto item, selectedItems)
        {
            if (item->geometry().contains(pressPos))
            {
                inSelectItem = true;
                break;
            }
        }

        if (!inSelectItem) // 不在选中图形上
        {
            // 取消选中
            unselectAll();
        }

        update();
    }
    else if (event->button() == Qt::RightButton) // 即将显示菜单
    {
        // 自动选中鼠标下面的菜单
        QPoint pos = event->pos();
        foreach (auto item, items)
        {
            if (item->geometry().contains(pos))
            {
                if (!selectedItems.contains(item))
                    unselectAll();
                selectItem(item);
                break;
            }
        }
    }

    QWidget::mousePressEvent(event);
}

void UniversePanel::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (pressing)
        {
            pressing = false;
            draggingPos = event->pos();

            // 批量选中
            if ((pressPos - draggingPos).manhattanLength() > QApplication::startDragDistance())
            {
                QRect range(pressPos, draggingPos);
                foreach (auto item, items)
                {
                    if (range.contains(item->geometry().center()))
                    {
                        item->showSelect(true);
                        selectedItems.insert(item);
                    }
                }
            }
            else // 单击
            {
                // 什么都不做
            }
        }

        pressPos = draggingPos = QPoint();
        update();
    }

    QWidget::mouseReleaseEvent(event);
}

void UniversePanel::mouseMoveEvent(QMouseEvent *event)
{
    if (moving)
    {

    }
    else if (pressing)
    {
        draggingPos = event->pos();
        update();
    }

    QWidget::mouseMoveEvent(event);
}

void UniversePanel::mouseDoubleClickEvent(QMouseEvent *event)
{
    QWidget::mouseDoubleClickEvent(event);
}

void UniversePanel::contextMenuEvent(QContextMenuEvent *)
{
    newFacileMenu;
    currentMenu = menu;
    QPoint cursorPos = mapFromGlobal(QCursor::pos());

    // 选中一个或者多个
    if (selectedItems.size())
    {
        menu->addAction(QIcon(":/icons/open"), "打开", [=]{
            foreach (auto item, selectedItems)
            {
                triggerItem(item);
            }
            unselectAll();
        });

        menu->addAction(QIcon(":/icons/delete"), "删除", [=]{
            foreach (auto item, selectedItems)
            {
                items.removeOne(item);
                deleteItem(item);
            }
            selectedItems.clear();
            save();
        });
    }

    // 选中一个
    if (selectedItems.size() == 1)
    {
        auto item = selectedItems.toList().first();
        menu->addAction(QIcon(":/icons/rename"), "重命名", [=]{
            bool ok = false;
            QString newName = QInputDialog::getText(this, "修改名字", "请输入新的名字", QLineEdit::Normal, item->text, &ok);
            if (!ok)
                return ;
            item->setText(newName);
            item->adjustSize();
            save();
        });

        menu->addAction(QIcon(":/icons/link"), "链接", [=]{
            bool ok = false;
            QString newLink = QInputDialog::getText(this, "修改链接", "可以是文件路径、网址，点击项目后立即打开", QLineEdit::Normal, item->link, &ok);
            if (!ok)
                return ;
            item->setLink(newLink);
            save();
        });
    }

    if (selectedItems.size())
        menu->split();

    auto addMenu = menu->addMenu(QIcon(":/icons/add"), "添加");
    addMenu->addAction("文件", [=]{

    })->disable();
    addMenu->addAction("文件夹", [=]{

    })->disable();
    addMenu->split()->addAction("文件链接", [=]{
        menu->close();
        QString prevPath = us->s("recent/selectFile");
        QString path = QFileDialog::getOpenFileName(this, "添加文件", prevPath);
        if (path.isEmpty())
            return ;
        us->set("recent/selectFile", path);
        QIcon icon = QFileIconProvider().icon(QFileInfo(path));
        auto item = createNewItem(cursorPos, icon, QFileInfo(path).fileName());
        item->setLink(path);
        save();
    });
    addMenu->addAction("文件夹链接", [=]{
        menu->close();
        QString prevPath = us->s("recent/selectFile");
        QString path = QFileDialog::getExistingDirectory(this, "添加文件夹", prevPath);
        if (path.isEmpty())
            return ;
        us->set("recent/selectFile", path);
        QIcon icon = QFileIconProvider().icon(QFileInfo(path));
        auto item = createNewItem(cursorPos, icon, QFileInfo(path).fileName());
        item->setLink(path);
        save();
    });
    addMenu->addAction("网址", [=]{
        menu->close();
        QString url = QInputDialog::getText(this, "添加网址", "请输入URL", QLineEdit::Normal);
        if (url.isEmpty())
            return ;
        // TODO：获取网页名字
        auto item = createNewItem(cursorPos, ":/icons/link", url);
        item->setLink(url);
        save();
    });

    menu->split()->addAction(QIcon(":/icons/fix"), "固定", [=]{
        fixing = !fixing;
    })->check(fixing);

    menu->addAction(QIcon(":/icons/refresh"), "刷新", [=]{
        readItems();
    });

    menu->exec();
    menu->finished([=]{
        currentMenu = nullptr;
        if (!this->hasFocus())
            foldPanel();
    });
}

void UniversePanel::keyPressEvent(QKeyEvent *event)
{
    auto key = event->key();
    if (key == Qt::Key_Escape)
    {
        if (selectedItems.size())
            unselectAll();

        event->ignore();
    }

    QWidget::keyPressEvent(event);
}

void UniversePanel::dragEnterEvent(QDragEnterEvent *event)
{
    auto mime = event->mimeData();
    if (!expanding)
    {
        expandPanel();
    }

    if(mime->hasUrls())//判断数据类型
    {

    }
    else if (mime->hasHtml())
    {

    }
    else if (mime->hasText())
    {

    }
    else if (mime->hasImage())
    {

    }
    else
    {
        event->ignore();//忽略
        return ;
    }

    event->acceptProposedAction(); // 接收该数据类型拖拽事件
}

void UniversePanel::dragMoveEvent(QDragMoveEvent *event)
{
    auto mime = event->mimeData();
    if(mime->hasUrls())//判断数据类型
    {
    }
    else if (mime->hasHtml())
    {

    }
    else if (mime->hasText())
    {

    }
    else if (mime->hasImage())
    {

    }
    else
    {
        event->ignore();
        return ;
    }

    event->acceptProposedAction();//接收该数据类型拖拽事件
}

/// 拖动到上面
/// 创建对应的对象
/// !注意可能是自己拖动的，作为移动
void UniversePanel::dropEvent(QDropEvent *event)
{
    QPoint pos = event->pos();
    auto mime = event->mimeData();
    if(mime->hasUrls())//处理期望数据类型
    {
        QFileIconProvider icon_provider;
        QList<QUrl> urls = mime->urls();//获取数据并保存到链表中
        for(int i = 0; i < urls.count(); i++)
        {
            qInfo() << urls.at(i);
            PanelItem* item = nullptr;
            if (urls.at(i).isLocalFile())
            {
                QString path = urls.at(i).toLocalFile();
                QIcon icon = icon_provider.icon(QFileInfo(path));
                item = createNewItem(pos, icon, urls.at(i).fileName());
                item->setLink(path);
            }
            else
            {
                QString path = urls.at(i).url();
                item = createNewItem(pos, ":/icons/link", path);
                item->setLink(path);
            }
            pos.rx() += item->width();
        }
    }
    else if (mime->hasHtml())
    {

    }
    else if (mime->hasText())
    {
        QString text = mime->text();
        if (!text.contains("\n"))
        {
            if (text.startsWith("http://") || text.startsWith("https://"))
            {
                auto item = createNewItem(pos, ":/icons/link", text);
                item->setLink(text);
            }
            else if (QFileInfo(text).exists())
            {
                QFileInfo info(text);
                QString path = info.absoluteFilePath();
                QIcon icon = QFileIconProvider().icon(QFileInfo(path));
                auto item = createNewItem(pos, icon, info.fileName());
                item->setLink(path);
            }
        }

    }
    else if (mime->hasImage())
    {

    }
    else
    {
        event->ignore();
        return ;
    }
}
