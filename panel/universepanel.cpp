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
#include <QClipboard>
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
#include "carditem.h"
#include "qss_editor/qsseditdialog.h"

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
    setFocusPolicy(Qt::ClickFocus);

    initPanel();

    initAction();
}

UniversePanel::~UniversePanel()
{
    saveLater();
}

/// 初始化面板所有数据
void UniversePanel::initPanel()
{
    QRect screen = screenGeometry();
    resize(us->panelWidth, us->panelHeight);
    move((screen.width() - width()) / 2 + us->panelCenterOffset, -height() + us->panelBangHeight); //

    saveTimer = new QTimer(this);
    saveTimer->setInterval(10);
    connect(saveTimer, SIGNAL(timeout()), this, SLOT(save()));

    readItems();

    // 没有项目，展示一下这里有东西
    QTimer::singleShot(1000, [=]{
        if (!items.size())
        {
            // 自动展开，吸引用户
            expandPanel();
            QTimer::singleShot(1000, [=]{
                // 如果用户没有管它，则1秒后自动隐藏
                if (!this->geometry().contains(QCursor::pos()))
                    foldPanel();
            });
        }
    });
}

void UniversePanel::initAction()
{
    auto createAction = [=](QString key, FuncType fun){
        QAction* action = new QAction(this);
        action->setShortcut(QKeySequence(key));
        connect(action, &QAction::triggered, this, fun);
        this->addAction(action);
    };

    createAction("ctrl+a", [=]{
        auto selects = selectedItems;
        unselectAll();
        // 第一次全选，不包括忽略选择的
        selectAll(false);
        // 第二次全选，包括忽略选择
        if (selects == selectedItems)
            selectAll(true);
    });
    createAction("delete", [=]{
        foreach (auto item, selectedItems)
            deleteItem(item);
        saveLater();
    });
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
    rt->flag_readingItems = true;
    MyJson json(readTextFileIfExist(rt->PANEL_PATH).toUtf8());
    QJsonArray array = json.a("items");
    foreach (auto ar, array)
    {
        MyJson json = ar.toObject();
        PanelItemType type = PanelItemType(json.i("type"));

        PanelItemBase* item = nullptr;
        switch (type)
        {
        case DefaultItem:
            qWarning() << "未设置类型的组件";
            continue;
        case IconText:
        case LocalFile:
        case WebUrl:
            item = new IconTextItem(this);
            break;
        case LongText:
            item = new LongTextItem(this);
            break;
        case ImageView:
            item = new ImageItem(this);
            break;
        case CardView:
            item = new CardItem(this);
            break;
        case TodoList:
            item = new TodoItem(this);
            break;
        }

        if (item)
        {
            item->fromJson(json);
            item->show();
            items.append(item);
            connectItem(item);
        }
    }
    QTimer::singleShot(0, [=]{
        // widget 的 resize 可能会在 show 之后一些事件调用
        // 所以这里关闭状态有需要延迟
        rt->flag_readingItems = false;
    });
}

IconTextItem *UniversePanel::createLinkItem(QPoint pos, bool center, const QIcon &icon, const QString &text, const QString &link, PanelItemType type)
{
    QString iconName = IconTextItem::saveIconFile(icon);
    return createLinkItem(pos, center, iconName, text, link, type);
}

IconTextItem *UniversePanel::createLinkItem(QPoint pos, bool center, const QString& iconName, const QString &text, const QString &link, PanelItemType type)
{
    auto item = new IconTextItem(this);
    item->setIcon(iconName);
    item->setText(text);
    item->setLink(link);
    item->setType(type);
    item->adjustSize();

    item->show();
    if (center)
        pos -= QPoint(item->width() / 2, item->height() / 2);
    item->move(pos);

    items.append(item);
    connectItem(item);
    saveLater();
    selectItem(item);
    return item;
}

LongTextItem *UniversePanel::createTextItem(QPoint pos, const QString &text, bool enableHtml)
{
    auto item = new LongTextItem(this);
    item->setText(text, enableHtml);
    if (!text.isEmpty())
        item->adjustSizeByText(ITEM_MAX_SIZE);
    else if (!us->moduleSize_Text.isEmpty())
        item->resize(us->moduleSize_Text);

    item->show();
    QFontMetrics fm(item->font());
    item->move(pos - item->contentsRect().topLeft() - QPoint(2, fm.height() / 2 + 2));

    items.append(item);
    connectItem(item);

    saveLater();
    selectItem(item);
    return item;
}

ImageItem *UniversePanel::createImageItem(QPoint pos, const QPixmap &pixmap)
{
    QString imageName = ImageItem::saveImageFile(pixmap);
    return createImageItem(pos, imageName);
}

ImageItem *UniversePanel::createImageItem(QPoint pos, const QString &image)
{
    auto item = new ImageItem(this);
    item->setImage(image);
    item->adjustSizeByImage(ITEM_MAX_SIZE);
    if (!us->moduleSize_Image.isEmpty()
            && item->width() * us->moduleSize_Image.height()
                == item->height() * us->moduleSize_Image.width()) // 宽高比相同，调整为同一大小
        item->resize(us->moduleSize_Image);

    item->show();
    item->move(pos - QPoint(item->width() / 2, item->height() / 2));

    items.append(item);
    connectItem(item);
    saveLater();
    selectItem(item);
    return item;
}

CardItem *UniversePanel::createCardItem(QPoint pos)
{
    auto item = new CardItem(this);
    if (us->contains("recent/cardRadius"))
        item->setRadius(us->i("recent/cardRadius"));
    else
        item->setRadius(us->fluentRadius);
    if (us->contains("recent/cardColor"))
        item->setColor(us->value("recent/cardColor").toString());


    if (!us->moduleSize_Card.isEmpty())
        item->resize(us->moduleSize_Card);
    else
        item->resize(us->panelItemSize * 3, us->panelItemSize * 2);

    item->show();
    item->move(pos);

    items.append(item);
    connectItem(item);
    saveLater();
    selectItem(item);
    return item;
}

TodoItem *UniversePanel::createTodoItem(QPoint pos)
{
    auto item = new TodoItem(this);

    if (!us->moduleSize_Todo.isEmpty())
        item->resize(us->moduleSize_Todo);

    item->show();
    QFontMetrics fm(item->font());
    item->move(pos - item->contentsRect().topLeft() - QPoint(2, 2));

    items.append(item);
    connectItem(item);

    saveLater();
    selectItem(item);
    item->insertAndFocusItem(0);
    return item;
}

void UniversePanel::connectItem(PanelItemBase *item)
{
    connect(item, &PanelItemBase::triggered, this, [=]{
        triggerItem(item);
    });

    connect(item, &PanelItemBase::pressed, this, [=](const QPoint& pos){
        if (item->isAutoRaise())
            raiseItem(item);

        if (QGuiApplication::keyboardModifiers() & Qt::ControlModifier) // 多选
        {
            QPoint pos = mapFromGlobal(QCursor::pos());
            eachitem(
                if (item->geometry().contains(pos))
                {
                    if (selectedItems.contains(item))
                        unselectItem(item);
                    else
                        selectItem(item, pos);
                    break;
                }
            )
        }
        else
        {
            if (!selectedItems.contains(item))
            {
                unselectAll();
                selectItem(item, pos);
            }
        }
    });

    connect(item, &PanelItemBase::selectMe, this, [=]{
        if (!(selectedItems.size() == 1 && selectedItems.contains(item)))
        {
            unselectAll();
            selectItem(item);
        }
    });

    connect(item, &PanelItemBase::modified, this, [=]{
        saveLater();
    });

    connect(item, &PanelItemBase::moveItems, this, [=](QPoint delta) {
        foreach (auto item, selectedItems)
        {
            item->move(item->pos() + delta);
        }
        saveLater();
    });

    connect(item, &PanelItemBase::facileMenuUsed, this, [=](FacileMenu* menu) {
        currentMenu = menu;
        menu->finished([=]{
            if (!this->hasItemUsing()
                    && !menu->isClosedByClick())
            {
                foldPanel();
            }
            currentMenu = nullptr;
        });
    });

    connect(item, &PanelItemBase::hidePanel, this, [=]{
        foldPanel();
    });

    connect(item, &PanelItemBase::useFinished, this, [=]{
//        if (!isItemUsing())
//            foldPanel(); // 不能隐藏，否则会出大事！
    });

    connect(item, &PanelItemBase::deleteMe, this, [=]{
        deleteItem(item);
        saveLater();
    });

    connect(item, &PanelItemBase::raiseMe, this, [=]{
        raiseItem(item);
        saveLater();
    });

    connect(item, &PanelItemBase::lowerMe, this, [=]{
        lowerItem(item);
        saveLater();
    });

    connect(item, &PanelItemBase::keepPanelFixing, this, [=]{
        _prev_fixing = fixing;
        fixing = true;
    });

    connect(item, &PanelItemBase::restorePanelFixing, this, [=]{
        auto f = _prev_fixing;
        QTimer::singleShot(0, [=]{
            fixing = f;
        });
    });
}

void UniversePanel::deleteItem(PanelItemBase *item)
{
    items.removeOne(item);
    selectedItems.remove(item);
    item->releaseResource();
    item->deleteLater();
}

/// 判断item是否正在使用
/// 有些情况会误判为leaveEvent，可能是：
/// - 弹出的右键菜单
/// - 输入法候选框
bool UniversePanel::hasItemUsing() const
{
    if (currentMenu && currentMenu->hasFocus())
        return true;

    bool ct = geometry().contains(QCursor::pos()); // 鼠标是否在面板上
    foreach (auto item, items)
        if (item->isUsing() && ct)
            return true;
    return false;
}

void UniversePanel::keepPanelState(FuncType func)
{
    bool _fixing = fixing;
    fixing = true;

    func();

    fixing = _fixing;
}

/// 从收起状态展开面板
void UniversePanel::expandPanel()
{
    // 动态背景
    if (us->panelBlur && this->pos().y() <= -this->height() + 1)
    {
        // 截图
        int radius = us->panelBlurRadius;
        QScreen* screen = QApplication::screenAt(QCursor::pos());
        QPixmap bg = screen->grabWindow(0,
                                        pos().x() - radius,
                                        0 - radius,
                                        width() + radius * 2,
                                        height() + radius * 2);

        // 模糊
        QT_BEGIN_NAMESPACE
            extern Q_WIDGETS_EXPORT void qt_blurImage( QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0 );
        QT_END_NAMESPACE

        QPixmap pixmap = bg;
        QPainter painter( &pixmap );
        QImage img = pixmap.toImage(); // img -blur-> painter(pixmap)
        qt_blurImage( &painter, img, radius, true, false );

        QPixmap blured(pixmap.size());
        blured.fill(Qt::transparent);
        QPainter painter2(&blured);
        painter2.setOpacity(us->panelBlurOpacity / 255.0);
        painter2.drawPixmap(blured.rect(), pixmap);

        // 裁剪掉边缘（模糊后会有黑边）
        int c = qMin(bg.width(), bg.height());
        c = qMin(c/2, radius);
        panelBlurPixmap = blured.copy(c, c, blured.width()-c*2, blured.height()-c*2);
    }

    // 展示动画
    QPropertyAnimation* ani = new QPropertyAnimation(this, "pos");
    ani->setStartValue(pos());
    ani->setEndValue(QPoint(pos().x(), 0));
    ani->setDuration(300);
    ani->setEasingCurve(QEasingCurve::OutCubic);
    if (us->panelBlur)
    {
        connect(ani, &QPropertyAnimation::valueChanged, this, [=]{
            update();
        });
    }
    connect(ani, &QPropertyAnimation::finished, this, [=]{
        ani->deleteLater();
        PanelItemBase::_blockPress = animating = false;
        update();
    });
    ani->start();
    expanding = true;
    PanelItemBase::_blockPress = animating = true;
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
    if (us->panelBlur)
    {
        connect(ani, &QPropertyAnimation::valueChanged, this, [=]{
            update();
        });
    }
    connect(ani, &QPropertyAnimation::finished, this, [=]{
        ani->deleteLater();
        PanelItemBase::_blockPress = animating = false;
        update();
    });
    ani->start();
    expanding = false;
    PanelItemBase::_blockPress = animating = true;
}

/// 延迟保存
/// 因为很多动作都会触发修改，导致重复保存
void UniversePanel::saveLater()
{
    saveTimer->start();
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

void UniversePanel::selectAll(bool containIgnored)
{
    if (containIgnored)
    {
        foreach (auto item, items)
            item->setSelect(true);
        selectedItems = items.toSet();
    }
    else
    {
        selectedItems.clear();
        foreach (auto item, items)
        {
            if (item->isIgnoreSelect())
                continue;
            item->setSelect(true);
            selectedItems.insert(item);
        }
    }
}

void UniversePanel::unselectAll()
{
    foreach (auto item, selectedItems)
        item->setSelect(false);
    selectedItems.clear();
}

void UniversePanel::selectItem(PanelItemBase *item, const QPoint &pos)
{
    item->setSelect(true, pos);
    selectedItems.insert(item);
}

void UniversePanel::unselectItem(PanelItemBase *item)
{
    item->setSelect(false);
    selectedItems.remove(item);
}

void UniversePanel::triggerItem(PanelItemBase *item)
{
    item->triggerEvent();
}

void UniversePanel::raiseItem(PanelItemBase *item)
{
    item->raise();
    items.removeOne(item);
    items.append(item);
}

void UniversePanel::lowerItem(PanelItemBase *item)
{
    item->lower();
    items.removeOne(item);
    items.insert(0, item);
}

/// 选中多项，开始拖拽
void UniversePanel::startDragSelectedItems()
{

}

void UniversePanel::pasteFromClipboard(QPoint pos)
{
    auto mime = QApplication::clipboard()->mimeData();
    insertMimeData(mime, pos);
}

void UniversePanel::insertMimeData(const QMimeData *mime, QPoint pos)
{
    // 要选中添加的item，就先取消其他所有的
    unselectAll();

    // 处理期望数据类型
    if(mime->hasUrls())
    {
        QFileIconProvider icon_provider;
        QList<QUrl> urls = mime->urls();//获取数据并保存到链表中
        qInfo() << "拖拽 URLs:" << urls;

        for(int i = 0; i < urls.count(); i++)
        {
            IconTextItem* item = nullptr;
            if (urls.at(i).isLocalFile()) // 拖拽本地文件
            {
                QString path = urls.at(i).toLocalFile();
                QIcon icon = icon_provider.icon(QFileInfo(path));
                QFileInfo info(path);
                QString link = path;
                link.replace(rt->PANEL_FILE_PATH, FILE_PREFIX);
                item = createLinkItem(pos, !i, icon, info.isDir() ? info.fileName() : info.baseName(), link, PanelItemType::LocalFile);
            }
            else // 拖拽网络URL
            {
                QString path = urls.at(i).url();
                QString pageName;
                QPixmap pixmap;
                keepPanelState([&]{
                    getWebPageNameAndIcon(path, pageName, pixmap);
                    if (pageName.isEmpty() && urls.count() == 1) // 空名字，并且获取不到
                    {
                        // 智能获取名字
                        QString defaultName = path;
                        if (defaultName.contains("?")) // 去掉URL参数
                            defaultName = defaultName.left(defaultName.indexOf("?"));
                        if (defaultName.contains("/")) // 获取action
                            defaultName = defaultName.right(defaultName.length() - defaultName.lastIndexOf("/") - 1);
                        if (defaultName.contains(".")) // 去掉文件后缀
                            defaultName = defaultName.left(defaultName.indexOf("."));
                        // 输入名字
                        bool ok = false;
                        this->activateWindow(); // 不激活当前窗口，inputDialog就不会获取焦点
                        QString text = QInputDialog::getText(this, "书签标题", "无法自动获取标题，请手动输入", QLineEdit::Normal, defaultName, &ok);
                        if (ok)
                        {
                            pageName = text;
                        }
                    }
                });
                item = createLinkItem(pos, !i,
                                      IconTextItem::saveIconFile(pixmap.isNull() ? QPixmap(":/icons/link") : pixmap),
                                      pageName.isEmpty() ? path : pageName,
                                      path, PanelItemType::WebUrl);
            }
            if (!i)
            {
                pos.rx() += item->width() / 2;
                pos.ry() = item->y();
            }
            else
                pos.rx() += item->width();
        }
    }
    else if (mime->hasHtml())
    {
        QString html = mime->html();
        qInfo() << "拖拽 HTML" << html.left(200);

        createTextItem(pos, html, true);
    }
    else if (mime->hasText())
    {
        QString text = mime->text();
        qInfo() << "拖拽 TEXT" << text.left(200);

        // 链接
        if (!text.contains("\n"))
        {
            // 处理网址
            if (text.startsWith("http://") || text.startsWith("https://"))
            {
                QString pageName;
                QPixmap pixmap;
                getWebPageNameAndIcon(text, pageName, pixmap);
                createLinkItem(pos, true, QPixmap(":/icons/link"), pageName.isEmpty() ? text : pageName, text, PanelItemType::WebUrl);
                return ;
            }

            // 处理本地文件
            else if (QFileInfo(text).exists())
            {
                QFileInfo info(text);
                QString path = info.absoluteFilePath();
                QIcon icon = QFileIconProvider().icon(QFileInfo(path));
                QString link = path;
                link.replace(rt->PANEL_FILE_PATH, FILE_PREFIX);
                createLinkItem(pos, true, icon, info.fileName(), link, PanelItemType::LocalFile);
                return ;
            }
        }

        // 普通文本
        createTextItem(pos, text, false);
    }
    else if (mime->hasImage())
    {
        QImage image = qvariant_cast<QImage>(mime->imageData());
        QPixmap pixmap(image.size());
        createImageItem(pos, pixmap.fromImage(image));
    }
    else if (mime->hasColor())
    {
        qInfo() << "TODO: 插入 Color";
    }
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
            QString fullTitle = match.captured(1).trimmed();
            qInfo() << "网页标题：" << fullTitle;
            if (fullTitle.contains("-") && !fullTitle.startsWith("-"))
            {
                pageName = fullTitle.left(fullTitle.lastIndexOf("-")).trimmed();
                if (url.contains(pageName.toLower())) // 左边的是名字，要用右边的
                    pageName = fullTitle.right(fullTitle.length() - fullTitle.indexOf("-") - 1).trimmed();
            }
            else
            {
                pageName = fullTitle;
            }

            if (pageName.isEmpty())
                pageName = fullTitle;
            if (pageName.contains(":") && !pageName.startsWith(":"))
                pageName = pageName.left(pageName.indexOf(":")).trimmed();
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
    int port = urlObj.port(-1);
    // if (!host.contains(QRegularExpression("(\\d+\\.){3}\\d+"))) // 内网大概率是测试的，没有网站图标
    {
        QString faviconUrl = url.left(url.indexOf(host)) + host + (port > 0 ? QString(":%1").arg(port) : "") + "/favicon.ico";
        QByteArray ba = NetUtil::getWebBa(faviconUrl);
        if (ba.size())
        {
             pageIcon = QPixmap(us->panelIconSize, us->panelIconSize);
             pageIcon.loadFromData(ba, nullptr, Qt::AutoColor);
        }
        else
        {
            qWarning() << "获取网页图标失败：" << faviconUrl;
        }
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
    // 拖拽到外面的时候，左键没事leave在release之后
    // 右边就在release之前leave了
    // 所以直接判断pressing状态
    if (us->allowMoveOut && (pressing || scening))
    {
        return ;
    }

    // 是否是拖拽的时候不小心移到了外面去了
    // 因为有人对这个操作机制感到迷惑，所以在设置里加了，默认开启
    if (us->allowMoveOut && _release_outter)
    {
        _release_outter = false;
        return ;
    }

    // 是否有item正在使用
    if (hasItemUsing())
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
        painter.fillPath(path, us->panelBgColor);

        if (us->panelBlur && us->panelBlurOpacity && !panelBlurPixmap.isNull())
        {
            QRect rect = this->rect();
            rect.moveTop(-this->y());
            painter.drawPixmap(rect, panelBlurPixmap);
        }
    }

    // 画刘海
    if (this->pos().y() <= -this->height() + 1)
    {
        QPainterPath path;
        int w = us->panelBangWidth;
        path.addRect(QRect((width() - w) / 2, height() - us->panelBangHeight, w, us->panelBangHeight));
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
            painter.fillPath(path, us->panelSelectRectColor);
        }
    }
}

void UniversePanel::mousePressEvent(QMouseEvent *event)
{
    if (animating) // 动画中禁止按下的误操作
        return ;
    moving = false;
    scening = false;

    if (event->button() == Qt::LeftButton)
    {
        pressing = true;
        pressPos = draggingPos = event->pos();
        scening = false;

        // 判断点击位置
        // 此时已经确保不是点在item上了
        bool inSelectItem = false; // 点在选中图形上
        foreach (auto item, selectedItems)
        {
            if (item->isIgnoreSelect())
                continue;
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
    else if (event->button() == Qt::RightButton) // 移动画面，或者即将显示菜单
    {
        pressing = true;
        pressPos = draggingPos = event->pos();
        scening = false;

        // 自动选中鼠标下面的菜单
        QPoint pos = event->pos();
        eachitem( // 越显示在前面的优先判断
            if (item->geometry().contains(pos))
            {
                if (!selectedItems.contains(item))
                    unselectAll();
                selectItem(item, item->mapFromParent(pressPos));
                return ;
            }
        )
        // 没有选中的，可能是移动画面，或者空白菜单
    }

    QWidget::mousePressEvent(event);
}

void UniversePanel::mouseMoveEvent(QMouseEvent *event)
{
    if (moving) // 拖着组件移动
    {

    }
    else if (pressing) // 拖拽
    {
        draggingPos = event->pos();
        if (event->buttons() & Qt::RightButton ||
                (event->buttons() & Qt::LeftButton && QGuiApplication::keyboardModifiers() & Qt::AltModifier)) // 拖拽移动空间
        {
            if (!scening)
            {
                if ((draggingPos - pressPos).manhattanLength() > QApplication::startDragDistance())
                    scening = true;
            }
            if (scening)
            {
                QPoint delta = draggingPos - pressPos;
                delta *= us->spatialMoveRatio;
                foreach (auto item, items)
                {
                    item->move(item->pos() + delta);
                }
                pressPos = draggingPos;
            }
        }
        else if (event->buttons() & Qt::LeftButton) // 拖拽出区域
        {
            update();

            // 显示hover
            QRect range(pressPos, draggingPos);
            foreach (auto item, items)
            {
                if (item->isSelected() || item->isIgnoreSelect())
                    continue;

                if (range.contains(item->geometry().center()))
                {
                    item->setHover(true, item->mapFromParent(draggingPos));
                }
                else
                {
                    item->setHover(false);
                }
            }
        }
    }

    QWidget::mouseMoveEvent(event);
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
            if (scening) // 移动面板位置
            {
                saveLater();
                scening = false;
            }
            else if ((pressPos - draggingPos).manhattanLength() > QApplication::startDragDistance()) // 拖拽选中结束
            {
                // 取消hover状态
                foreach (auto item, items)
                {
                    if (!item->isSelected() && item->isHovered())
                        item->setHover(false);
                }

                // 选中
                QRect range(pressPos, draggingPos);
                foreach (auto item, items)
                {
                    if (item->isIgnoreSelect())
                        continue;
                    if (range.contains(item->geometry().center()))
                    {
                        item->setSelect(true);
                        selectedItems.insert(item);
                    }
                }
            }
            else // 单击
            {
                // item自己的trigger事件
                // 自己什么都不做
            }
        }

        pressPos = draggingPos = QPoint();
        update();
    }
    else if (event->button() == Qt::RightButton)
    {
        pressing = false;
        draggingPos = event->pos();
        event->accept();
        if (scening) // 拖拽完毕，不显示菜单
        {
            scening = false;
            _block_menu = true;
            QTimer::singleShot(10, [=]{
                _block_menu = false;
            });

            // 拖拽到外面，必定会触发 leaveEvent
            // 右键很坑，事件顺序是：leave(pressing) -> release -> leave(unpressing)
            if (!this->geometry().contains(QCursor::pos()))
                _release_outter = true;
            return ;
        }
    }

    // 拖拽到外面，必定会触发 leaveEvent
    if (!this->geometry().contains(QCursor::pos()))
        _release_outter = true;

    QWidget::mouseReleaseEvent(event);
}

void UniversePanel::mouseDoubleClickEvent(QMouseEvent *event)
{
    newFacileMenu;

    showAddMenu(menu);

    Q_UNUSED(event)
    // addPastAction(menu, event->pos(), true);

    currentMenu = menu;
    menu->exec();
    menu->finished([=]{
        currentMenu = nullptr;
        if (!this->hasFocus())
            foldPanel();
    });
}

void UniversePanel::contextMenuEvent(QContextMenuEvent *)
{
    if (_block_menu)
    {
        _block_menu = false;
        return ;
    }
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

        menu->split();
        auto alignMenu = menu->addMenu(QIcon(":/icons/align"), "对齐 (&Q)");
        alignMenu->addAction("上对齐 (&T)", [=]{
            auto sItems = selectedItems.toList();
            // 寻找基准线
            int minY = sItems.first()->pos().y();
            foreach (auto item, sItems)
                if (minY > item->pos().y())
                {
                    minY = item->pos().y();
                }

            // 批量移动
            foreach (auto item, sItems)
            {
                item->move(item->pos().x(), minY);
            }
            saveLater();
        });
        alignMenu->addAction("行中对齐 (&H)", [=]{
            auto sItems = selectedItems.toList();
            // 以最上面的元素为基准线
            PanelItemBase* tItem = sItems.first();
            int minLeft = tItem->pos().x();
            foreach (auto item, sItems)
                if (minLeft > item->pos().x())
                {
                    minLeft = item->pos().x();
                    tItem = item;
                }
            int alignY = tItem->geometry().center().y();

            // 批量移动
            foreach (auto item, sItems)
            {
                item->move(item->pos().x(), alignY - item->height() / 2);
            }
            saveLater();
        });
        alignMenu->addAction("下对齐 (&B)", [=]{
            auto sItems = selectedItems.toList();
            // 寻找基准线
            int maxY = sItems.first()->geometry().bottom();
            foreach (auto item, sItems)
                if (maxY < item->geometry().bottom())
                {
                    maxY = item->geometry().bottom();
                }

            // 批量移动
            foreach (auto item, sItems)
            {
                item->move(item->pos().x(), maxY - item->height());
            }
            saveLater();
        });
        alignMenu->split()->addAction("左对齐 (&L)", [=]{
            auto sItems = selectedItems.toList();
            // 寻找基准线
            int minX = sItems.first()->pos().x();
            foreach (auto item, sItems)
                if (minX > item->pos().x())
                {
                    minX = item->pos().x();
                }

            // 批量移动
            foreach (auto item, sItems)
            {
                item->move(minX, item->pos().y());
            }
            saveLater();
        });
        alignMenu->addAction("列中对齐 (&V)", [=]{
            auto sItems = selectedItems.toList();
            // 以最上面的元素为基准线
            PanelItemBase* tItem = sItems.first();
            int minTop = tItem->pos().y();
            foreach (auto item, sItems)
                if (minTop > item->pos().y())
                {
                    minTop = item->pos().y();
                    tItem = item;
                }
            int alignX = tItem->geometry().center().x();

            // 批量移动
            foreach (auto item, sItems)
            {
                item->move(alignX - item->width() / 2, item->pos().y());
            }
            saveLater();
        });
        alignMenu->addAction("右对齐 (&R)", [=]{
            auto sItems = selectedItems.toList();
            // 寻找基准线
            int maxX = sItems.first()->geometry().right();
            foreach (auto item, sItems)
                if (maxX < item->geometry().right())
                {
                    maxX = item->geometry().right();
                }

            // 批量移动
            foreach (auto item, sItems)
            {
                item->move(maxX - item->width(), item->pos().y());
            }
            saveLater();
        });

        auto spaceMenu = menu->addMenu(QIcon(":/icons/spacing"), "分布 (&F)");
        spaceMenu->addAction("水平等间距 (&H)", [=]{
            auto sItems = selectedItems.toList();
            std::sort(sItems.begin(), sItems.end(), [=](PanelItemBase* item1, PanelItemBase* item2){
                return item1->x() < item2->x();
            });

            int barLen = sItems.last()->geometry().right() - sItems.first()->geometry().left();
            foreach (auto item, sItems)
                barLen -= item->width();
            int each = barLen / qMax(1, sItems.count() - 1); // 每两个item之间的间距

            // 批量移动
            int left = sItems.first()->x();
            foreach (auto item, sItems)
            {
                item->move(left, item->y());
                left += item->width() + each;
            }
            saveLater();
        });
        spaceMenu->addAction("垂直等间距 (&V)", [=]{
            auto sItems = selectedItems.toList();
            std::sort(sItems.begin(), sItems.end(), [=](PanelItemBase* item1, PanelItemBase* item2){
                return item1->y() < item2->y();
            });

            int barLen = sItems.last()->geometry().bottom() - sItems.first()->geometry().top();
            foreach (auto item, sItems)
                barLen -= item->height();
            int each = barLen / qMax(1, sItems.count() - 1); // 每两个item之间的间距

            // 批量移动
            int top = sItems.first()->y();
            foreach (auto item, sItems)
            {
                item->move(item->x(), top);
                top += item->height() + each;
            }
            saveLater();
        });
        spaceMenu->split()->addAction("自动分布 (&A)", [=]{

        })->disable();
        spaceMenu->addAction("网格分布 (&G)", [=]{

        })->disable();
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
        menu->split();

        menu->addAction(QIcon(":/icons/style"), "样式 (&Y)", [=]{
            QString text = (*selectedItems.begin())->getCustomQss();
            bool ok;
            QString qss = QssEditDialog::getText(this, "自定义样式", "设置多个部件的CSS样式", text, &ok);
            if (!ok)
                return ;
            foreach (auto item, selectedItems)
            {
                item->setCustomQss(qss);
            }
            saveLater();
        });

        menu->addAction(QIcon(":/icons/delete"), "删除 (&D)", [=]{
            foreach (auto item, selectedItems)
            {
                deleteItem(item);
            }
            saveLater();
        });
    }

    if (!selectedItems.size())
    {
        // 创建新的
        auto addMenu = menu->addMenu(QIcon(":/icons/add"), "新建 (&A)");
        showAddMenu(addMenu);

        // 剪贴板
        addPastAction(menu, cursorPos);

        // 面板固定操作
        menu->split()->addAction(QIcon(":/icons/fix"), "固定 (&F)", [=]{
            fixing = !fixing;
        })->check(fixing);

        menu->addAction(QIcon(":/icons/config"), "设置 (&S)", [=]{
            emit openSettings();
        });

        menu->addAction(QIcon(":/icons/refresh"), "刷新 (&R)", [=]{
            // 其实刷新并没有任何用处
            // 但会让人觉得高大上起来
            readItems();
        })->hide();
    }

    menu->exec();
    menu->finished([=]{
        currentMenu = nullptr;
        if (!geometry().contains(QCursor::pos()) // 应该使用的，但实测这句加不加问题不大
                && !menu->geometry().contains(QCursor::pos())
                && !menu->isClosedByClick()
                && !this->hasItemUsing()
                && !this->hasFocus()) // 这个判断并没有任何用处，就意思一下
        {
            // 鼠标外面隐藏菜单，隐藏面板
            foldPanel();
        }

        // 获取焦点后，鼠标外面操作菜单后，点击外面区域会触发 leaveEvent
        // this->setFocus();
//        if (!this->hasFocus())
//            foldPanel();
    });
}

void UniversePanel::showAddMenu(FacileMenu *addMenu)
{
    QPoint cursorPos = mapFromGlobal(QCursor::pos());
    addMenu->addRow([=]{
        addMenu->addAction(QIcon(":icons/txt"), "文本 (&E)", [=]{
            auto item = createTextItem(cursorPos, "", false);
            item->editText();
        });
        addMenu->addAction(QIcon(":icons/url"), "网址 (&U)", [=]{
            if (currentMenu)
                currentMenu->close();
            QString url = QInputDialog::getText(this, "添加网址", "请输入URL", QLineEdit::Normal);
            if (url.isEmpty())
                return ;
            QString pageName;
            QPixmap pixmap;
            getWebPageNameAndIcon(url, pageName, pixmap);
            createLinkItem(cursorPos, true,
                           IconTextItem::saveIconFile(pixmap.isNull() ? QPixmap(":/icons/link") : pixmap),
                           pageName.isEmpty() ? url : pageName,
                           url, PanelItemType::WebUrl);
        });
    });

    addMenu->addRow([=]{
        addMenu->addAction(QIcon(":icons/todo"), "待办 (&T)", [=]{
            if (currentMenu)
                currentMenu->close();
            createTodoItem(cursorPos);
        });
        addMenu->addAction(QIcon(":icons/remind"), "提醒 (&R)", [=]{

        })->disable();
    });

    addMenu->split()->addRow([=]{
        addMenu->split()->addAction(QIcon(":icons/file"), "文件 (&F)", [=]{
            QString name = QInputDialog::getText(this, "创建文件", "请输入文件名，包括后缀");
            if (name.isEmpty())
                return ;

            // 获取可创建的文件名，去掉特殊符号
            QString fileName = name;
            QChar cs[] = {'\\', '/', ':', '*', '?', '"', '<', '>', '|', '\'', '\n', '\t'};
            for (int i = 0; i < 12; i++)
                fileName.replace(cs[i], "");

            // 获取合适的文件路径
            QString suffix;
            if (fileName.contains("."))
            {
                int pos = fileName.lastIndexOf(".");
                suffix = fileName.right(fileName.length() - pos);
                fileName = fileName.left(pos);
                if (fileName.isEmpty())
                    fileName = QString::number(QDateTime::currentMSecsSinceEpoch());
            }
            QString path = getPathWithIndex(rt->PANEL_FILE_PATH, fileName, suffix);

            // 创建item
            QIcon icon = QFileIconProvider().icon(QFileInfo(path));
            QString link = path;
            link.replace(rt->PANEL_FILE_PATH, FILE_PREFIX);
            createLinkItem(cursorPos, true, icon, name, link, PanelItemType::LocalFile);

            // 创建并打开文件
            ensureFileExist(path);
            QDesktopServices::openUrl("file:///" + path);
        });
        addMenu->addAction(QIcon(":icons/folder"), "文件夹 (&D)", [=]{
            QString name = QInputDialog::getText(this, "创建文件夹", "请输入文件夹名");
            if (name.isEmpty())
                return ;

            // 获取可创建的文件名，去掉特殊符号
            QString fileName = name;
            QChar cs[] = {'\\', '/', ':', '*', '?', '"', '<', '>', '|', '\'', '\n', '\t'};
            for (int i = 0; i < 12; i++)
                fileName.replace(cs[i], "");
            if (fileName.isEmpty())
                fileName = QString::number(QDateTime::currentMSecsSinceEpoch());

            QString path = rt->PANEL_FILE_PATH + fileName;

            // 创建item
            QIcon icon = QFileIconProvider().icon(QFileInfo(path));
            QString link = path;
            link.replace(rt->PANEL_FILE_PATH, FILE_PREFIX);
            createLinkItem(cursorPos, true, icon, name, link, PanelItemType::LocalFile);

            // 创建并打开文件
            ensureDirExist(path);
            QDesktopServices::openUrl("file:///" + path);
        });
    });

    addMenu->addRow([=]{
        addMenu->addAction(QIcon(":icons/link"), "文件链接 (&K)", [=]{
            if (currentMenu)
                currentMenu->close();
            QString prevPath = us->s("recent/selectFile");
            QString path = QFileDialog::getOpenFileName(this, "添加文件快捷方式", prevPath);
            if (path.isEmpty())
                return ;
            us->set("recent/selectFile", path);
            QIcon icon = QFileIconProvider().icon(QFileInfo(path));
            createLinkItem(cursorPos, true, icon, QFileInfo(path).fileName(), path, PanelItemType::LocalFile);
        });
        addMenu->addAction(QIcon(":icons/link"), "文件夹链接 (&L)", [=]{
            if (currentMenu)
                currentMenu->close();
            QString prevPath = us->s("recent/selectFile");
            QString path = QFileDialog::getExistingDirectory(this, "添加文件夹快捷方式", prevPath);
            if (path.isEmpty())
                return ;
            us->set("recent/selectFile", path);
            QIcon icon = QFileIconProvider().icon(QFileInfo(path));
            createLinkItem(cursorPos, true, icon, QFileInfo(path).fileName(), path, PanelItemType::LocalFile);
        });
    });

    addMenu->split()->addRow([=]{
        addMenu->addAction(QIcon(":icons/image"), "图像 (&P)", [=]{
            if (currentMenu)
                currentMenu->close();
            QString prevPath = us->s("recent/selectFile");
            QString path = QFileDialog::getOpenFileName(this, "选择图片文件", prevPath, tr("Images (*.png *.xpm *.jpg *.jpeg *.gif)"));
            if (path.isEmpty())
                return ;
            us->set("recent/selectFile", path);

            // 插入图像
            QPixmap pixmap(path);
            createImageItem(cursorPos, pixmap);
        });
        addMenu->addAction(QIcon(":icons/rounded_rect"), "矩形 (&B)", [=]{
            createCardItem(cursorPos);
        });
    });
}

void UniversePanel::addPastAction(FacileMenu *menu, QPoint pos, bool split)
{
    auto clipboard = QApplication::clipboard();
    auto mime = clipboard->mimeData();
    bool canPaste = true;
    QString pasteName = "";
    if (mime->hasUrls())
        pasteName = "URL";
    else if (mime->hasText())
    {
        auto text = mime->text();
        if (!text.contains("\n"))
        {
            if (text.startsWith("http://") || text.startsWith("https://"))
                pasteName = "网址";
            else if (QFileInfo(text).exists())
                pasteName = "文件";
        }
        if (pasteName.isEmpty())
            pasteName = "文本";
    }
    else if (mime->hasHtml())
        pasteName = "富文本";
    else if (mime->hasImage())
        pasteName = "图像";
    else if (mime->hasColor())
        pasteName = "颜色";
    else
        canPaste = false;

    if (canPaste)
    {
        if (split)
            menu->split();
        menu->addAction(QIcon(":/icons/paste"), "粘贴 " + pasteName + " (&V)", [=]{
            pasteFromClipboard(pos);
        });
    }
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
    // 取消选中
    // 仅drag的对象才显示hover状态
    unselectAll();

    auto mime = event->mimeData();
    if (!expanding)
    {
        expandPanel();
    }

    if(mime->hasUrls())//判断数据类型
    {
        event->setDropAction(Qt::LinkAction);
    }
    else if (mime->hasHtml())
    {
        event->setDropAction(Qt::CopyAction);
    }
    else if (mime->hasText())
    {
        event->setDropAction(Qt::CopyAction);
    }
    else if (mime->hasImage())
    {
        event->setDropAction(Qt::CopyAction);
    }
    else if (mime->hasColor())
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
    insertMimeData(mime, pos);
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
            /* if (xPos >= (width() - us->panelBangWidth) / 2 && xPos <= (width() + us->panelBangWidth) / 2) // 刘海部分
                return false; */
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
