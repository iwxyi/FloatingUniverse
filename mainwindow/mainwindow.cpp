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
#ifdef Q_OS_WIN32
#include "windows.h"
#endif
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

    us->set("usage/bootCount", ++us->bootCount);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete panel;
    delete tray;
    deleteDir(rt->CACHE_PATH);
}

void MainWindow::initView()
{
    auto setShadow = [=](QWidget* w, int x, int y){
        QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(w);
        shadow_effect->setOffset(x, y);
        shadow_effect->setColor(Qt::gray);
        shadow_effect->setBlurRadius(20);
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
    setButton(ui->drawerButton);
    setShadow(ui->appbarWidget, 0, 3);
    setButton(ui->searchButton);

    // 应用更改按钮
    confirmButton = new WaterCircleButton(this);
    confirmButton->setBgColor(us->themeMainColor);
    confirmButton->setFixedSize(us->widgetSize * 1.5, us->widgetSize * 1.5);
    confirmButton->setIcon(QIcon(":/icons/apply"));
    setButton(confirmButton);
    setShadow(confirmButton, 0, 5);
    confirmButton->hide();

    // sidebar
    auto addGroupItem = [=](QPixmap pixmap, QString groupName) {
        const int margin = 12;
        const int spacingAdd = 6;

        InteractiveButtonBase* w = new InteractiveButtonBase(ui->sidebarList);
        QLabel* pixmapLabel = new QLabel(w);
        QLabel* titleLabel = new QLabel(w);
        QHBoxLayout* layout = new QHBoxLayout(w);
        layout->addSpacing(spacingAdd);
        layout->addWidget(pixmapLabel);
        layout->addSpacing(spacingAdd);
        layout->addWidget(titleLabel);
        pixmapLabel->setPixmap(pixmap);
        pixmapLabel->setScaledContents(true);
        titleLabel->setText(groupName);
        titleLabel->adjustSize();
        int sz = titleLabel->height();
        pixmapLabel->setFixedSize(sz, sz);

        layout->setMargin(margin);
        layout->setSpacing(margin);

        auto item = new QListWidgetItem(ui->sidebarList);
        ui->sidebarList->setItemWidget(item, w);
        w->adjustSize();
        w->setCursor(Qt::PointingHandCursor);
        item->setSizeHint(QSize(0, sz + layout->margin() * 2));

        connect(w, &InteractiveButtonBase::clicked, this, [=]{
            int row = ui->sidebarList->row(item);
            ui->settingsBody->focusGroup(row);
        });
    };
    addGroupItem(QPixmap(":/icons/panel"), "悬浮面板");
    addGroupItem(QPixmap(":/icons/universe2"), "宇宙荧光");
    addGroupItem(QPixmap(":/icons/space_transmission"), "空间传送");
    addGroupItem(QPixmap(":/icons/interaction"), "面板交互");
    addGroupItem(QPixmap(":/icons/st/file"), "文件操作");
    addGroupItem(QPixmap(":/icons/used_data"), "使用数据");
    addGroupItem(QPixmap(":/icons/about"), "关于程序");

    // body
    connect(ui->settingsBody, &SettingsController::boxH, this, [=](int left, int width) {
        ui->searchBox->setMaximumWidth(width);
        ui->spacerWidget->setFixedWidth(qMax(0, ui->settingsBody->x() + left - ui->spacerWidget->x()));
    });
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

void MainWindow::showDrawer()
{
    ui->sidebarList->show();
    drawering = true;
}

void MainWindow::hideDrawer()
{
    ui->sidebarList->hide();
    drawering = false;
}

void MainWindow::showEvent(QShowEvent *e)
{
    QMainWindow::showEvent(e);
    us->set("mainwindow/hide", false);

    QTimer::singleShot(0, [=]{
        ui->settingsBody->adjustGroupSize();
    });

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
    tray = new QSystemTrayIcon(this);
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
        if (!us->trayClickOpenPanel)
        {
            if (!this->isHidden())
                this->hide();
            else
            {
                this->showNormal();
                this->activateWindow();
            }
        }
        else
        {
            panel->hide();
            panel->show();
            panel->expandPanel();
        }
        break;
    case QSystemTrayIcon::MiddleClick:
        break;
    case QSystemTrayIcon::Context:
    {
        FacileMenu* menu = new FacileMenu;

        menu->addAction(QIcon(":/icons/show_panel"), "唤出", [=]{
            if (!us->trayClickOpenPanel)
            {
                panel->hide();
                panel->show();
                panel->expandPanel();
            }
            else
            {
                this->showNormal();
                this->activateWindow();
            }
        });

        menu->addAction(QIcon(":/icons/hide"), "隐藏", [=]{
            if (panel->isHidden())
                panel->show();
            else
                panel->hide();
        })->check(panel->isHidden());

        menu->split()->addAction(QIcon(":/icons/quit"), "退出", [=]{
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
    connect(ui->settingsBody, SIGNAL(updatePanel()), panel, SLOT(update()));
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    us->setValue("mainwindow/geometry", this->saveGeometry());
    us->setValue("mainwindow/state", this->saveState());
	us->sync();

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
#endif
    QMainWindow::closeEvent(e);
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);

    // 判断 FloatingActionButton
    if (confirmButton)
        confirmButton->move(this->rect().bottomRight() - QPoint(confirmButton->width() * 1.4, confirmButton->width()*1.4));

    // 判断 Drawer
    bool allBody = width() - ui->sidebarList->width() - ui->centralwidget->layout()->spacing() > us->settingsMinWidth + us->settingsMargin * 2;
    if (!allBody && drawering)
    {
        hideDrawer();
    }
    else if (allBody && !drawering)
    {
        showDrawer();
    }
}

void MainWindow::returnToPrevWindow()
{
#ifdef Q_OS_WIN32
        if (this->prevWindow)
            SwitchToThisWindow(prevWindow, true);
        prevWindow = nullptr;
#endif
}

void MainWindow::on_searchEdit_textEdited(const QString &arg1)
{
    ui->settingsBody->setFind(arg1);
}

void MainWindow::on_searchButton_clicked()
{
    ui->searchEdit->selectAll();
    ui->searchEdit->setFocus();
}

void MainWindow::on_drawerButton_clicked()
{
    if (drawering)
        hideDrawer();
    else
        showDrawer();
}
