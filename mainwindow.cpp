#include <QDebug>
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
    ui->settingsTabWidget->setAttribute(Qt::WA_StyledBackground);
    ui->settingsTabWidget->setTabPosition(QTabWidget::West);
    ui->settingsTabWidget->tabBar()->setStyle(new CustomTabStyle);
    ui->settingsTabWidget->setCurrentIndex(us->i("mainwindow/settingsTabIndex"));

    ui->auxiliaryTabWidget->setAttribute(Qt::WA_StyledBackground);
    ui->auxiliaryTabWidget->setTabPosition(QTabWidget::West);
    ui->auxiliaryTabWidget->tabBar()->setStyle(new CustomTabStyle);
    ui->auxiliaryTabWidget->setCurrentIndex(us->i("mainwindow/settingsTabIndex"));

    ui->dataTabWidget->setAttribute(Qt::WA_StyledBackground);
    ui->dataTabWidget->setTabPosition(QTabWidget::West);
    ui->dataTabWidget->tabBar()->setStyle(new CustomTabStyle);
    ui->dataTabWidget->setCurrentIndex(us->i("mainwindow/settingsTabIndex"));


    ui->settingsTabWidget->clear();
    ui->settingsTabWidget->addTab(new QWidget(this), QIcon("://icons/tab1.png"), "Tab1");

    ui->auxiliaryTabWidget->clear();
    ui->auxiliaryTabWidget->addTab(new QWidget(this), QIcon("://icons/tab2.png"), "Tab2");

    ui->dataTabWidget->clear();
    ui->dataTabWidget->addTab(new QWidget(this), QIcon("://icons/tab3.png"), "Tab3");

    ui->sideButtons->setCurrentRow(qMin(us->i("mainwindow/sideIndex"), ui->sideButtons->count() - 1));
    ui->settingsTabWidget->setCurrentIndex(qMin(us->i("mainwindow/settingsTabIndex"), ui->settingsTabWidget->count() - 1));
    ui->auxiliaryTabWidget->setCurrentIndex(qMin(us->i("mainwindow/auxiliaryTabIndex"), ui->auxiliaryTabWidget->count() - 1));
    ui->dataTabWidget->setCurrentIndex(qMin(us->i("mainwindow/dataTabIndex"), ui->dataTabWidget->count() - 1));

    connect(ui->settingsTabWidget, &QTabWidget::currentChanged, this, [=](int index) {
        us->set("mainwindow/settingsTabIndex", index);
    });
    connect(ui->auxiliaryTabWidget, &QTabWidget::currentChanged, this, [=](int index) {
        us->set("mainwindow/auxiliaryTabIndex", index);
    });
    connect(ui->dataTabWidget, &QTabWidget::currentChanged, this, [=](int index) {
        us->set("mainwindow/dataTabIndex", index);
    });

    // 应用更改按钮
    confirmButton = new InteractiveButtonBase("应用更改", this);
    confirmButton->setBorderColor(Qt::gray);
    confirmButton->setFixedForeSize();
    confirmButton->setCursor(Qt::PointingHandCursor);
    confirmButton->setFixedForePos();
    confirmButton->move(this->rect().bottomRight() - QPoint(confirmButton->width() * 1.1, confirmButton->height() + confirmButton->width()*0.1));
    confirmButton->setFocusPolicy(Qt::StrongFocus);
    if (ui->sideButtons->currentRow() == 2)
        confirmButton->hide();
}

void MainWindow::on_sideButtons_currentRowChanged(int currentRow)
{
    ui->stackedWidget->setCurrentIndex(currentRow);
    us->set("mainwindow/sideIndex", currentRow);

    // 只有设置才显示应用更改
    if (confirmButton)
    {
        if (currentRow == 0 || currentRow == 1)
            this->confirmButton->show();
        else
            this->confirmButton->hide();
    }
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

    if (confirmButton)
        confirmButton->move(this->rect().bottomRight() - QPoint(confirmButton->width() * 1.1, confirmButton->height() + confirmButton->width()*0.1));
}

void MainWindow::returnToPrevWindow()
{
#ifdef Q_OS_WIN32
        if (this->prevWindow)
            SwitchToThisWindow(prevWindow, true);
        prevWindow = nullptr;
#endif
}
