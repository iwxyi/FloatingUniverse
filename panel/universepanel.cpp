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
#ifdef Q_OS_WIN32
#include <windows.h>
#include <windowsx.h>
#endif
#include "universepanel.h"
#include "runtime.h"
#include "usettings.h"
#include "signaltransfer.h"
#include "facilemenu.h"
#include "fileutil.h"
#include "netutil.h"
#include "icontextitem.h"

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
    resize(us->panelWidth, us->panelHeight);
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
    selectedItems.clear();

    // 读取设置
    MyJson json(readTextFile(rt->PANEL_PATH).toUtf8());
    QJsonArray array = json.a("items");
    foreach (auto ar, array)
    {
        MyJson json = ar.toObject();
        PanelItemType type = PanelItemType(json.i("type"));
        switch (type)
        {
        case DefaultItem:
            continue;
        case IconText:
        case LocalFile:
        case WebUrl:
        {
            auto item = new IconTextItem(this);
            item->fromJson(json);
            items.append(item);
            item->show();
            connectItem(item);
            break;
        }
        case LongText:
        case ImageView:
            break;
        }
    }
}

IconTextItem *UniversePanel::createNewItem(QPoint pos, const QIcon &icon, const QString &text)
{
    QString iconName = saveIcon(icon);
    return createNewItem(pos, iconName, text);
}

IconTextItem *UniversePanel::createNewItem(QPoint pos, const QString& iconName, const QString &text)
{
    auto item = new IconTextItem(this);
    // item->textLabel->setMaximumWidth(us->pannelItemSize);
    item->setIcon(iconName);
    item->setText(text);
    item->move(pos - QPoint(item->width() / 2, item->height() / 2));
    items.append(item);
    item->show();
    connectItem(item);

    save();
    return item;
}

void UniversePanel::connectItem(PanelItemBase *item)
{
    connect(item, &PanelItemBase::triggered, this, [=]{
        triggerItem(item);
    });

    connect(item, &PanelItemBase::pressed, this, [=]{
        if (QGuiApplication::keyboardModifiers() & Qt::ControlModifier) // 多选
        {
            QPoint pos = mapFromGlobal(QCursor::pos());
            foreach (auto item, items)
            {
                if (item->geometry().contains(pos))
                {
                    if (selectedItems.contains(item))
                        unselectItem(item);
                    else
                        selectItem(item);
                    break;
                }
            }
        }
        else
        {
            if (!selectedItems.contains(item))
            {
                unselectAll();
                selectItem(item);
            }
        }
    });

    connect(item, &PanelItemBase::modified, this, [=]{
        save();
    });

    connect(item, &PanelItemBase::moveItems, this, [=](QPoint delta) {
        foreach (auto item, selectedItems)
        {
            item->move(item->pos() + delta);
        }
    });

    connect(item, &PanelItemBase::facileMenuUsed, this, [=](FacileMenu* menu) {
        currentMenu = menu;
        menu->finished([=]{
            currentMenu = nullptr;
            if (!this->hasFocus())
                foldPanel();
        });
    });

    connect(item, &PanelItemBase::hidePanel, this, [=] {
        foldPanel();
    });

}

QString UniversePanel::saveIcon(const QIcon &icon) const
{
    QPixmap pixmap = icon.pixmap(us->pannelItemSize, us->pannelItemSize);
    return saveIcon(pixmap);
}

QString UniversePanel::saveIcon(const QPixmap &pixmap) const
{
    // 保存到本地
    int val = 1;
    while (QFileInfo(rt->ICON_PATH + QString::number(val) + ".png").exists())
        val++;
    QString iconName = QString::number(val) + ".png";

    pixmap.save(rt->ICON_PATH + iconName);
    return iconName;
}

void UniversePanel::deleteItem(PanelItemBase *item)
{
    if (qobject_cast<IconTextItem*>(item))
    {
        auto it = qobject_cast<IconTextItem*>(item);
        QString iconName = it->getIconName();
        if (!iconName.isEmpty())
        {
            deleteFile(rt->ICON_PATH + iconName);
        }
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
    ani->setEasingCurve(QEasingCurve::OutCubic);
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
    ani->setEasingCurve(QEasingCurve::InOutCubic);

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

void UniversePanel::selectItem(PanelItemBase *item)
{
    item->showSelect(true);
    selectedItems.insert(item);
}

void UniversePanel::unselectItem(PanelItemBase *item)
{
    item->showSelect(false);
    selectedItems.remove(item);
}

void UniversePanel::triggerItem(PanelItemBase *item)
{
    item->triggerEvent();
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

bool UniversePanel::getWebPageNameAndIcon(QString url, QString &pageName, QPixmap &pageIcon)
{
    // 获取网页标题
    QString source = NetUtil::getWebData(url);
    if (!source.isEmpty())
    {
        QRegularExpressionMatch match;
        if (source.indexOf(QRegularExpression("<(?:title|TITLE)>(.+)</\\s*(?:title|TITLE)>"), 0, &match) > -1)
        {
            pageName = match.captured(1).trimmed();
            qInfo() << "网页标题：" << pageName;
            if (pageName.contains("-") && !pageName.startsWith("-"))
                pageName = pageName.left(pageName.lastIndexOf("-")).trimmed();
        }
        else
        {
            qWarning() << "未找到标题" << source.indexOf(QRegularExpression("<(title|TITLE)>(.+)</\\s*(title|TITLE)>"));
        }
    }
    else
    {
        qWarning() << "获取网页信息失败：" << url;
    }

    // 获取网页图标
    QUrl urlObj(url);
    QString host = urlObj.host();
    QString faviconUrl = url.left(url.indexOf(host)) + host + "/favicon.ico";
    QByteArray ba = NetUtil::getWebBa(faviconUrl);
    if (ba.size())
    {
         pageIcon = QPixmap(us->pannelItemSize, us->pannelItemSize);
         pageIcon.loadFromData(ba, nullptr, Qt::AutoColor);
    }
    else
    {
        qWarning() << "获取网页图标失败：" << faviconUrl;
    }

    return true;
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
        // 此时已经确保不是点在item上了
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
            if (QGuiApplication::keyboardModifiers() & Qt::AltModifier) // 移动面板位置
            {
                save();
            }
            else if ((pressPos - draggingPos).manhattanLength() > QApplication::startDragDistance()) // 拖拽选中结束
            {
                // 取消hover状态
                foreach (auto item, items)
                {
                    if (!item->isSelected() && item->isHovered())
                        item->showHover(false);
                }

                // 选中
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
    if (moving) // 拖着组件移动
    {

    }
    else if (pressing) // 拖拽
    {
        draggingPos = event->pos();
        if (QGuiApplication::keyboardModifiers() & Qt::AltModifier) // 拖拽移动
        {
            QPoint delta = draggingPos - pressPos;
            foreach (auto item, items)
            {
                item->move(item->pos() + delta);
            }
            pressPos = draggingPos;
        }
        else // 拖拽出区域
        {
            update();

            // 显示hover
            QRect range(pressPos, draggingPos);
            foreach (auto item, items)
            {
                if (item->isSelected())
                    continue;

                if (range.contains(item->geometry().center()))
                {
                    item->showHover(true);
                }
                else
                {
                    item->showHover(false);
                }
            }
        }
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

    // 选中多个，使用批量打开（暂且不管是不是选中的item都能打开）
    if (selectedItems.size() > 1)
    {
        menu->addAction(QIcon(":/icons/open"), "打开 (&O)", [=]{
            foreach (auto item, selectedItems)
            {
                triggerItem(item);
            }
            unselectAll();
        });
    }

    // 选中一个
    if (selectedItems.size() == 1)
    {
        // 使用这个item的自定义menu
        auto item = selectedItems.toList().first();
        item->facileMenuEvent(menu);
    }

    // 选中一个或者多个
    if (selectedItems.size())
    {
        menu->split()->addAction(QIcon(":/icons/delete"), "删除 (&D)", [=]{
            foreach (auto item, selectedItems)
            {
                items.removeOne(item);
                deleteItem(item);
            }
            selectedItems.clear();
            save();
        });
    }

    if (!selectedItems.size())
    {
        auto addMenu = menu->addMenu(QIcon(":/icons/add"), "创建 (&A)");
        addMenu->addAction("文件 (&F)", [=]{

        })->disable();
        addMenu->addAction("文件夹 (&D)", [=]{

        })->disable();
        addMenu->split()->addAction("文件链接 (&K)", [=]{
            menu->close();
            QString prevPath = us->s("recent/selectFile");
            QString path = QFileDialog::getOpenFileName(this, "添加文件快捷方式", prevPath);
            if (path.isEmpty())
                return ;
            us->set("recent/selectFile", path);
            QIcon icon = QFileIconProvider().icon(QFileInfo(path));
            auto item = createNewItem(cursorPos, icon, QFileInfo(path).fileName());
            item->setLink(path, PanelItemType::LocalFile);
            save();
        });
        addMenu->addAction("文件夹链接 (&L)", [=]{
            menu->close();
            QString prevPath = us->s("recent/selectFile");
            QString path = QFileDialog::getExistingDirectory(this, "添加文件夹快捷方式", prevPath);
            if (path.isEmpty())
                return ;
            us->set("recent/selectFile", path);
            QIcon icon = QFileIconProvider().icon(QFileInfo(path));
            auto item = createNewItem(cursorPos, icon, QFileInfo(path).fileName());
            item->setLink(path, PanelItemType::LocalFile);
            save();
        });
        addMenu->addAction("网址 (&U)", [=]{
            menu->close();
            QString url = QInputDialog::getText(this, "添加网址", "请输入URL", QLineEdit::Normal);
            if (url.isEmpty())
                return ;
            QString pageName;
            QPixmap pixmap;
            getWebPageNameAndIcon(url, pageName, pixmap);
            auto item = createNewItem(cursorPos,
                                      pixmap.isNull() ? ":/icons/link" : saveIcon(pixmap),
                                      pageName.isEmpty() ? url : pageName);
            item->setLink(url, PanelItemType::WebUrl);
            save();
        });

        menu->split()->addAction(QIcon(":/icons/fix"), "固定 (&F)", [=]{
            fixing = !fixing;
        })->check(fixing);

        menu->addAction(QIcon(":/icons/refresh"), "刷新 (&R)", [=]{
            readItems();
        });
    }

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
            qInfo() << "drop url:" << urls.at(i);
            IconTextItem* item = nullptr;
            if (urls.at(i).isLocalFile()) // 拖拽本地文件
            {
                QString path = urls.at(i).toLocalFile();
                QIcon icon = icon_provider.icon(QFileInfo(path));
                item = createNewItem(pos, icon, urls.at(i).fileName());
                item->setLink(path, PanelItemType::LocalFile);
            }
            else // 拖拽网络URL
            {
                QString path = urls.at(i).url();
                QString pageName;
                QPixmap pixmap;
                getWebPageNameAndIcon(path, pageName, pixmap);
                item = createNewItem(pos,
                                     pixmap.isNull() ? ":/icons/link" : saveIcon(pixmap),
                                     pageName.isEmpty() ? path : pageName);
                item->setLink(path, PanelItemType::WebUrl);
            }
            pos.rx() += item->width();
        }
        save();
    }
    else if (mime->hasHtml())
    {

    }
    else if (mime->hasText())
    {
        QString text = mime->text();
        if (!text.contains("\n"))
        {
            // 处理网址
            if (text.startsWith("http://") || text.startsWith("https://"))
            {
                QString pageName;
                QPixmap pixmap;
                getWebPageNameAndIcon(text, pageName, pixmap);
                auto item = createNewItem(pos, ":/icons/link", pageName.isEmpty() ? text : pageName);
                item->setLink(text, PanelItemType::WebUrl);
            }

            // 处理本地文件
            else if (QFileInfo(text).exists())
            {
                QFileInfo info(text);
                QString path = info.absoluteFilePath();
                QIcon icon = QFileIconProvider().icon(QFileInfo(path));
                auto item = createNewItem(pos, icon, info.fileName());
                item->setLink(path, PanelItemType::LocalFile);
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

bool UniversePanel::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
#ifdef Q_OS_WIN32
    Q_UNUSED(eventType)
    MSG* msg = static_cast<MSG*>(message);
    switch(msg->message)
    {
    case WM_NCHITTEST:
        if (!fixing) // 只有固定的时候才能调整
            return false;
        const auto ratio = devicePixelRatioF(); // 解决4K下的问题
        int xPos = static_cast<int>(GET_X_LPARAM(msg->lParam) / ratio - this->frameGeometry().x());
        int yPos = static_cast<int>(GET_Y_LPARAM(msg->lParam) / ratio - this->frameGeometry().y());
        if(xPos < boundaryWidth && yPos < boundaryWidth)                    //左上角
            *result = HTTOPLEFT;
        else if(xPos >= width() - boundaryWidth && yPos < boundaryWidth)          //右上角
            *result = HTTOPRIGHT;
        else if(xPos < boundaryWidth && yPos >= height() - boundaryWidth)         //左下角
            *result = HTBOTTOMLEFT;
        else if(xPos >= width() - boundaryWidth && yPos >= height() - boundaryWidth)//右下角
            *result = HTBOTTOMRIGHT;
        else if(xPos < boundaryWidth)                                     //左边
            *result =  HTLEFT;
        else if(xPos >= width() - boundaryWidth)                              //右边
            *result = HTRIGHT;
        /*else if(yPos<boundaryWidth)                                       //上边
            *result = HTTOP;*/
        else if(yPos >= height() - boundaryWidth)                             //下边
        {
            if (xPos >= (width() - us->panelBangWidth) / 2 && xPos <= (width() + us->panelBangWidth) / 2) // 刘海部分
                return false;
            *result = HTBOTTOM;
        }
        else              //其他部分不做处理，返回false，留给其他事件处理器处理
           return false;

        QRect screen = screenGeometry();
        us->set("panel/centerOffset", geometry().center().x() - screen.center().x());
        us->set("panel/width", width());
        us->set("panel/height", height());

        return true;
    }
#else
    return QWidget::nativeEvent(eventType, message, result);
#endif
    return false;         //此处返回false，留给其他事件处理器处理
}
