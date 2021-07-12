#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include "facilemenu.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "usettings.h"
#include "myjson.h"
#include "fileutil.h"
#include "imageutil.h"
#include "netutil.h"
#include "signaltransfer.h"
#include "windows.h"
#include "widgets/customtabstyle.h"
#include "runtime.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initView();
    initTray();
    initPanel();
}

MainWindow::~MainWindow()
{
    delete ui;

    deleteDir(rt->CACHE_PATH);
}

void MainWindow::initView()
{
    auto setShadow = [=](QWidget* w, int x, int y){
        QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(w);
        shadow_effect->setOffset(x, y);
        shadow_effect->setColor(Qt::gray);
        shadow_effect->setBlurRadius(32);
        w->setGraphicsEffect(shadow_effect);
    };

    auto setButton = [=](WaterCircleButton* btn) {
        btn->setHoverColor(QColor(0xff, 0xff, 0xff, 0x32));
        btn->setPressColor(QColor(0xff, 0xff, 0xff, 0x64));
        btn->setFixedForePos();
        btn->setFixedForeSize();
        btn->setIconPaddingProper(0.25);
        btn->setCursor(Qt::PointingHandCursor);
    };

    // AppBar
    ui->drawerButton->setFixedSize(us->widgetSize, us->widgetSize);
    ui->menuButton->setFixedSize(us->widgetSize, us->widgetSize);
    setButton(ui->drawerButton);
    setButton(ui->menuButton);
    setShadow(ui->appbarWidget, 0, 3);

    // 应用更改按钮
    confirmButton = new WaterCircleButton(this);
    confirmButton->setBgColor(us->themeMainColor);
    confirmButton->setFixedSize(us->widgetSize * 1.5, us->widgetSize * 1.5);
    confirmButton->setIcon(QIcon(":/icons/apply"));
    setButton(confirmButton);
    setShadow(confirmButton, 0, 5);

    // sidebar
    auto addGroupItem = [=](QPixmap pixmap, QString groupName) {
        InteractiveButtonBase* w = new InteractiveButtonBase(ui->sidebarList);
        QLabel* pixmapLabel = new QLabel(w);
        QLabel* titleLabel = new QLabel(w);
        QHBoxLayout* layout = new QHBoxLayout(w);
        layout->addWidget(pixmapLabel);
        layout->addWidget(titleLabel);
        pixmapLabel->setPixmap(pixmap);
        pixmapLabel->setScaledContents(true);
        titleLabel->setText(groupName);
        titleLabel->adjustSize();
        int sz = titleLabel->height();
        pixmapLabel->setFixedSize(sz, sz);
        layout->setMargin(layout->margin() * 1.3);

        auto item = new QListWidgetItem(ui->sidebarList);
        ui->sidebarList->setItemWidget(item, w);
        w->adjustSize();
        w->setCursor(Qt::PointingHandCursor);
        item->setSizeHint(QSize(0, sz + layout->margin() * 2));
    };
    addGroupItem(QPixmap(":/icons/panel"), "悬浮面板");
    addGroupItem(QPixmap(":/icons/universe2"), "宇宙传送");
    addGroupItem(QPixmap(":/icons/interaction"), "交互优化");
    addGroupItem(QPixmap(":/icons/auto_fill"), "自动填充");
    addGroupItem(QPixmap(":/icons/used_data"), "使用数据");
    addGroupItem(QPixmap(":/icons/about"), "关于程序");

    // body
    auto addSettingsGroupWidget = [=](QWidget* w, QString name){
        QLabel* label = new QLabel(name, ui->scrollAreaWidgetContents);
        label->setStyleSheet("color: gray;");
        w->setObjectName("SettingsGroup");
        w->setStyleSheet("#SettingsGroup{ background: white; border: none; border-radius: 5px; }");

        groupLabels.append(label);
        groupBoxes.append(w);
        setShadow(w, 0, 8);
    };

    addSettingsGroupWidget(new QWidget(ui->scrollAreaWidgetContents), "测试");

    // 调整body的大小
    adjustSettingsGroupSize();

}

QRect MainWindow::screenGeometry() const
{
    auto screens = QGuiApplication::screens();
    int index = 0;
    if (index >= screens.size())
        index = screens.size() - 1;
    if (index < 0)
        return QRect();
    return screens.at(index)->geometry();
}

void MainWindow::showEvent(QShowEvent *e)
{
    QMainWindow::showEvent(e);
    us->set("mainwindow/hide", false);

    static bool first = true;
    if (first)
    {
        first = false;
        restoreGeometry(us->value("mainwindow/geometry").toByteArray());
        restoreState(us->value("mainwindow/state").toByteArray());
    }
}

void MainWindow::initTray()
{
    QSystemTrayIcon* tray = new QSystemTrayIcon(this);
    tray->setIcon(QIcon(":/icons/appicon"));
    tray->setToolTip(APPLICATION_NAME);
    tray->show();

    connect(tray,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(trayAction(QSystemTrayIcon::ActivationReason)));
}

void MainWindow::trayAction(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
    case QSystemTrayIcon::Trigger:
        if (!this->isHidden())
            this->hide();
        else
        {
            this->showNormal();
            this->activateWindow();
        }
        break;
    case QSystemTrayIcon::MiddleClick:
        break;
    case QSystemTrayIcon::Context:
    {
        FacileMenu* menu = new FacileMenu;



        menu->split()->addAction(QIcon("://icons/quit.png"), "退出", [=] {
            qApp->quit();
        });

        menu->exec(QCursor::pos());
    }
        break;
    default:
        break;
    }
}

void MainWindow::initKey()
{
#if defined(ENABLE_SHORTCUT)
    editShortcut = new QxtGlobalShortcut(this);
    QString def_key = us->value("banner/replyKey", "shift+alt+x").toString();
    editShortcut->setShortcut(QKeySequence(def_key));
    connect(editShortcut, &QxtGlobalShortcut::activated, this, [=]() {
#if defined(Q_OS_WIN32)

#endif
        // this->activateWindow();
    });
#endif
}

void MainWindow::initPanel()
{
    panel = new UniversePanel(nullptr);
    connect(panel, SIGNAL(openSettings()), this, SLOT(show()));
    panel->show();
}

void MainWindow::adjustSettingsGroupSize()
{
    int margin = 12;
    const int fixedWidth = qMin(qMax(ui->scrollAreaWidgetContents->width() - margin * 2, 300), 680);
    const int groupSpacing = 12;
    const int labelSpacing = 9;
    const int left = qMax(margin, (ui->scrollAreaWidgetContents->width() - fixedWidth) / 2);
    int top = 9;
    for (int i = 0; i < groupLabels.size(); i++)
    {
        auto label = groupLabels.at(i);
        auto box = groupBoxes.at(i);
        label->setFixedWidth(fixedWidth);
        box->setFixedWidth(fixedWidth);
        label->adjustSize();
        box->adjustSize();
        box->setMinimumHeight(32);

        label->move(left, top);
        top += label->height() + labelSpacing;

        box->move(left, top);
        top += box->height() + groupSpacing;
    }

    ui->scrollAreaWidgetContents->setFixedHeight(top);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    us->setValue("mainwindow/geometry", this->saveGeometry());
    us->setValue("mainwindow/state", this->saveState());

#if defined(ENABLE_TRAY)
    e->ignore();
    this->hide();

    // 因为关闭程序也会触发这个，所以需要定时一下
    QTimer::singleShot(5000, [=]{
        us->set("mainwindow/hide", true);
    });

    QTimer::singleShot(5000, [=]{
        if (!this->isHidden())
            return ;
        us->setValue("mainwindow/autoShow", false);
    });
#else
    QMainWindow::closeEvent(e);
#endif
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);

    adjustSettingsGroupSize();
    if (confirmButton)
        confirmButton->move(this->rect().bottomRight() - QPoint(confirmButton->width() * 1.3, confirmButton->width()*1.3));
}

void MainWindow::returnToPrevWindow()
{
#ifdef Q_OS_WIN32
        if (this->prevWindow)
            SwitchToThisWindow(prevWindow, true);
        prevWindow = nullptr;
#endif
}
