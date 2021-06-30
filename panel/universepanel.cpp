#include <QPropertyAnimation>
#include <QApplication>
#include <QScreen>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QDateTime>
#include <QMouseEvent>
#include <QMimeData>
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
    MyJson json(readTextFile(rt->PANEL_PATH).toLocal8Bit());
    QJsonArray array = json.a("items");
    foreach (auto ar, array)
    {
        auto item = new PanelItem(this);
        item->fromJson(ar.toObject());
        item->show();
    }
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
        update();
    }

    QWidget::mousePressEvent(event);
}

void UniversePanel::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        pressing = false;

        // 批量选中

        draggingPos = pressPos;
        update();
    }

    QWidget::mouseReleaseEvent(event);
}

void UniversePanel::mouseMoveEvent(QMouseEvent *event)
{
    if (pressing)
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

    auto addMenu = menu->addMenu("添加");
    addMenu->addAction("文件", [=]{

    });
    addMenu->addAction("文件夹", [=]{

    });
    addMenu->addAction("文件链接", [=]{

    });
    addMenu->addAction("文件夹链接", [=]{

    });
    addMenu->addAction("网址", [=]{

    });

    menu->split()->addAction("固定", [=]{
        fixing = !fixing;
    })->check(fixing);

    menu->addAction("刷新", [=]{
        readItems();
    });

    menu->exec();
    menu->finished([=]{
        currentMenu = nullptr;
        if (!this->hasFocus())
            foldPanel();
    });
}

void UniversePanel::dragEnterEvent(QDragEnterEvent *event)
{
    if (!expanding)
    {
        expandPanel();
    }

    if(event->mimeData()->hasUrls())//判断数据类型
    {

    }
    else if (event->mimeData()->hasHtml())
    {

    }
    else if (event->mimeData()->hasText())
    {

    }
    else if (event->mimeData()->hasImage())
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
    if(event->mimeData()->hasUrls())//判断数据类型
    {
    }
    else if (event->mimeData()->hasHtml())
    {

    }
    else if (event->mimeData()->hasText())
    {

    }
    else if (event->mimeData()->hasImage())
    {

    }
    else
    {
        event->ignore();
        return ;
    }

    event->acceptProposedAction();//接收该数据类型拖拽事件
}

void UniversePanel::dropEvent(QDropEvent *event)
{
    if(event->mimeData()->hasUrls())//处理期望数据类型
    {
        QList<QUrl> urls = event->mimeData()->urls();//获取数据并保存到链表中
        for(int i = 0; i < urls.count(); i++)
        {
            QString path = urls.at(i).toLocalFile();
        }
    }
    else
    {
        event->ignore();
        return ;
    }
}
