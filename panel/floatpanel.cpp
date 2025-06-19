#include "floatpanel.h"
#include "runtime.h"
#include "usettings.h"
#include "signaltransfer.h"
#include "universepanel.h"
#include <QScreen>
#ifdef Q_OS_WIN32
#include <windows.h>
#include <windowsx.h>
#endif

FloatPanel::FloatPanel(QWidget *parent) : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setWindowFlag(Qt::WindowStaysOnTopHint, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAutoFillBackground(true);
}

void FloatPanel::initPanel()
{
    QRect screen = screenGeometry();
    resize(us->panelWidth, us->panelHeight);
    move((screen.width() - width()) / 2 + us->panelCenterOffset, -height() + us->panelBangHeight);

    rt->panel_expading = &expanding;
    rt->panel_animating = &animating;
    rt->panel_fixing = &fixing;

    panel = new UniversePanel(this);
    panel->initPanel();
    connect(panel, SIGNAL(openSettings()), this, SIGNAL(openSettings()));
    connect(panel, &UniversePanel::signalFoldPanel, this, [=] { foldPanel(); });
    connect(panel, &UniversePanel::signalExpandPanel, this, [=] { expandPanel(); });
    connect(panel, &UniversePanel::signalSetKeepFix, this, [=](bool keep) {
        if (keep)
        {
            _prev_fixing = fixing;
            fixing = true;
        }
        else
        {
            auto f = _prev_fixing;
            QTimer::singleShot(0, [=]{
                fixing = f;
            });
        }
    });
    connect(panel, &UniversePanel::signalKeepPanelState, this, [=](FuncType func) {
        keepPanelState(func);
    });

    // 没有项目，展示一下这里有东西
    QTimer::singleShot(1000, [=]{
        if (!panel->items.size())
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

/// 从收起状态展开面板
void FloatPanel::expandPanel()
{
    if (expanding)
        return;

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
void FloatPanel::foldPanel()
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

void FloatPanel::keepPanelState(FuncType func)
{
    bool _fixing = fixing;
    fixing = true;

    func();

    fixing = _fixing;
}

void FloatPanel::setPanelFixed(bool f)
{
    fixing = f;
}

void FloatPanel::enterEvent(QEvent *event)
{
    QWidget::enterEvent(event);

    if (expanding)
        return ;

    // 根据位置，判断是否需要展示位置
    int x = QCursor::pos().x() - this->x();
    if (x >= us->panelBangMarginLeft && x <= width() - us->panelBangMarginRight)
        expandPanel();
}

void FloatPanel::leaveEvent(QEvent *event)
{
    // 拖拽到外面的时候，左键没事leave在release之后
    // 右边就在release之前leave了
    // 所以直接判断pressing状态
    if (us->allowMoveOut && panel && (panel->pressing || panel->scening))
    {
        return ;
    }

    // 是否是拖拽的时候移到了外面去了
    // 因为有人对这个操作机制感到迷惑，所以在设置里加了，默认开启
    // 拖拽到外面松开，触发了 leaveEvent
    if (us->allowMoveOut && panel->_release_outter)
    {
        panel->_release_outter = false;
        return ;
    }

    // 鼠标正在面板内，大概率是由于菜单引起的
    /* if (isMouseInPanel())
        return ; */

    // 是否有item正在使用
    if (us->keepOnItemUsing && panel->hasItemUsing())
        return ;

    // 弹出菜单，也会触发
    if (panel->currentMenu && panel->currentMenu->hasFocus())
        return ;

    QWidget::leaveEvent(event);

    if (expanding)
        foldPanel();
}

void FloatPanel::paintEvent(QPaintEvent *e)
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
    if (this->pos().y() <= -this->height() + us->panelBangHeight)
    {
        QPainterPath path;
        path.addRect(us->panelBangMarginLeft, height() - us->panelBangHeight, qAbs(width() - us->panelBangMarginLeft - us->panelBangMarginRight), us->panelBangHeight);
        // int w = qMax(16, width() - us->panelBangMarginLeft - us->panelBangMarginRight);
        // path.addRect(QRect((width() - w) / 2, height() - us->panelBangHeight, w, us->panelBangHeight));
        painter.fillPath(path, us->panelBangBg);
    }

    QWidget::paintEvent(e);
}

void FloatPanel::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (panel)
    {
        panel->resize(this->width(), this->height() - us->panelBangHeight);
    }
}

void FloatPanel::closeEvent(QCloseEvent *)
{
    return ;
}

bool FloatPanel::nativeEvent(const QByteArray &eventType, void *message, long *result)
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

QRect FloatPanel::screenGeometry() const
{
    auto screens = QGuiApplication::screens();
    int index = 0;
    if (index >= screens.size())
        index = screens.size() - 1;
    if (index < 0)
        return QRect();
    return screens.at(index)->geometry();
}
