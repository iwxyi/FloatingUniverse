#include <QGraphicsDropShadowEffect>
#include <QScrollBar>
#include <QDir>
#include "settingscontroller.h"
#include "ui_settingscontroller.h"
#include "usettings.h"
#include "escape_dialog/escapedialog.h"

#define UPDATE_PANEL connect(w->lastItem(), SIGNAL(clicked()), this, SIGNAL(updatePanel()))

SettingsController::SettingsController(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PanelSettingsWidget)
{
    ui->setupUi(this);

    initItems();
}

SettingsController::~SettingsController()
{
    delete ui;
}

void SettingsController::initItems()
{
    // 悬浮面板设置
    auto w = new SettingsItemListBox(ui->scrollAreaWidgetContents);
    w->add(QPixmap(":/icons/st/bgColor"), "背景颜色", "", "panel/bgColor", &us->panelBgColor);
    UPDATE_PANEL;
    w->add(QPixmap(":/icons/st/selectBgColor"), "选择区域颜色", "", "panel/selectRectColor", &us->panelSelectRectColor);
    UPDATE_PANEL;
    w->add(QPixmap(":/icons/st/selectEdgeColor"), "选中项边框颜色", "", "panel/selectEdgeColor", &us->panelSelectEdgeColor);
    UPDATE_PANEL;
    w->add(QPixmap(":/icons/st/hoverEdgeColor"), "候选项边框颜色", "", "panel/hoverEdgeColor", &us->panelHoverEdgeColor);
    UPDATE_PANEL;
    // w->add(QPixmap(":/icons/st/bangBar"), "交互边缘宽度", "贴边呼出面板的有效宽度，不会超过面板宽度", "panel/bangWidth", &us->panelBangWidth, 100, 9999, 100);
    // UPDATE_PANEL;
    w->add(QPixmap(":/icons/st/blurRadius"), "毛玻璃模糊半径", "", "panel/blurRadius", &us->panelBlurRadius, 0, 255, 16);
    UPDATE_PANEL;
    w->add(QPixmap(":/icons/st/blurOpacity"), "毛玻璃透明度", "", "panel/blurOpacity", &us->panelBlurOpacity, 0, 255, 16);
    UPDATE_PANEL;
    w->add(QPixmap(":/icons/st/barHeight"), "触发条高度", "", "panel/bangHeight", &us->panelBangHeight, 0, 100, 1);
    UPDATE_PANEL;
    w->add(QPixmap(":/icons/st/barColor"), "触发条颜色", "", "panel/bangBg", &us->panelBangBg);
    UPDATE_PANEL;
    w->add(QPixmap(":/icons/st/marginLeft"), "触发条左边距", "", "panel/bangMarginLeft", &us->panelBangMarginLeft, 0, 9999, 50);
    UPDATE_PANEL;
    w->add(QPixmap(":/icons/st/marginRight"), "触发条右边距", "", "panel/bangMarginRight", &us->panelBangMarginRight, 0, 9999, 50);
    UPDATE_PANEL;
    addGroup(w, "悬浮面板");

    // 一些尺寸
    w = new SettingsItemListBox(ui->scrollAreaWidgetContents);
    w->add(QPixmap(":/icons/st/iconSize"), "图标大小", "最大尺寸(非强制,可能更小)，新图标生效", "panel/iconSize", &us->panelIconSize, 16, 255, 16);
    w->add(QPixmap(":/icons/st/spatialMoveRatio"), "空间移动倍率", "右键移动画布的速度的几何倍率", "interactive/spatialMoveRatio", &us->spatialMoveRatio, 1, 10, 1);
    addGroup(w, "宇宙荧光");

    // 一些位置
    w = new SettingsItemListBox(ui->scrollAreaWidgetContents);
    w->add(QPixmap(":/icons/st/TODO"), "待添加", "");
    addGroup(w, "空间传送");

    // 用户交互
    w = new SettingsItemListBox(ui->scrollAreaWidgetContents);
    w->add(QPixmap(":/icons/st/moveOut"), "允许拖拽到外面", "鼠标拖拽到悬浮面板外面，不自动隐藏面板", "interactive/allowMoveOut", &us->allowMoveOut);
    w->add(QPixmap(":/icons/st/editing"), "使用时保持显示", "编辑文本时鼠标移到面板外面也保持显示", "interactive/keepOnItemUsing", &us->keepOnItemUsing);
    w->add(QPixmap(":/icons/st/clickGesture"), "交换托盘手势", "右下角托盘单击时打开面板，菜单中唤出设置", "interactive/trayClickOpenPanel", &us->trayClickOpenPanel);
    addGroup(w, "面板交互");

    // 使用数据
    w = new SettingsItemListBox(ui->scrollAreaWidgetContents);
    w->add(QPixmap(":/icons/st/fileName"), "同步修改文件名", "修改文件快捷方式的名字时，询问修改文件名", "interactive/modifyFileNameSync", &us->modifyFileNameSync);
    w->add(QPixmap(":/icons/st/fastOpenDir"), "快速展开目录", "文件夹链接默认使用菜单的形式打开，可单独设置", "interactive/fastOpenDir", &us->fastOpenDir);
    w->add(QPixmap(":/icons/st/fastOpenDirLevel"), "展开目录级别", "快速展开目录时菜单的层级数量上限", "interactive/fastOpenDirLevel", &us->fastOpenDirLevel, 1, 10, 1);
    w->add(QPixmap(":/icons/st/fileCount"), "目录文件数量", "快速展开目录时菜单的文件数量上限", "interactive/fastOpenDirFileCount", &us->fastOpenDirFileCount, 1, 100, 1);
    addGroup(w, "文件操作");

    // 使用数据
    w = new SettingsItemListBox(ui->scrollAreaWidgetContents);
    w->add(QPixmap(":/icons/st/bootCount"), "程序启动次数", "", "panel/bootCount", &us->bootCount, 0, 0, 1);
    w->lastItem()->setEnabled(false);
    w->add(QPixmap(":/icons/st/linkOpenCount"), "打开链接次数", "", "panel/linkOpenCount", &us->linkOpenCount, 0, 0, 1);
    w->lastItem()->setEnabled(false);
    w->add(QPixmap(":/icons/st/reboot"), "开机自启", "", "interactive/autoReboot", &us->autoReboot);
    connect(w->lastItem(), &InteractiveButtonBase::clicked, this, [=]{
        QString appName = QApplication::applicationName();
        QString appPath = QDir::toNativeSeparators(QApplication::applicationFilePath());
        QSettings *reg = new QSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
        QString val = reg->value(appName).toString();// 如果此键不存在，则返回的是空字符串
        if (us->autoReboot)
            reg->setValue(appName, appPath);
        else
            reg->remove(appName);
        qInfo() << "设置自启：" << us->autoReboot;
        reg->sync();
        reg->deleteLater();
    });
    addGroup(w, "使用数据");

    // 关于程序
    w = new SettingsItemListBox(ui->scrollAreaWidgetContents);
    w->addOpen(QPixmap(":/icons/lyixi"), "开发团队", "本程序由杭州懒一夕智能科技有限公司“混吃等死部”开发", QUrl("http://lyixi.com"));
    w->addOpen(QPixmap(":/icons/st/QQ"), "交流反馈", "QQ群：280517990", QUrl("https://qm.qq.com/cgi-bin/qm/qr?k=a3rJlTLgGAhgx5PqvHz0RjinfHDpl4Ll&jump_from=webapi"));
    w->addPage(QPixmap(":/icons/st/donate"), "捐赠赞助", "好心的老板请开发者吃顿饭吧~");
    connect(w->lastItem(), &InteractiveButtonBase::clicked, this, [=]{
        EscapeDialog* dialog = new EscapeDialog("友情赞助", "您的支持是开发者为爱发电的最大动力！", "不想付钱", "感谢支持", this);
        dialog->exec();
    });
    w->addOpen(QPixmap(":/icons/st/GitHub"), "GitHub", "开源仓库：iwxyi/FloatingUniverse", QUrl("https://github.com/iwxyi/FloatingUniverse"));
    addGroup(w, "关于程序");
}

void SettingsController::addGroup(QWidget *w, QString name)
{
    QLabel* label = new QLabel(name, ui->scrollAreaWidgetContents);
    label->setStyleSheet("color: #202020; margin: 0.3px;");
    w->setObjectName("SettingsGroup");
    w->setStyleSheet("#SettingsGroup, SettingsItemListBox{ background: white; border: none; border-radius: 5px; }");
    w->adjustSize();

    labels.append(label);
    boxes.append(w);

    QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(w);
    shadow_effect->setOffset(0, 1);
    shadow_effect->setColor(Qt::lightGray);
    shadow_effect->setBlurRadius(8);
    w->setGraphicsEffect(shadow_effect);
}

void SettingsController::focusGroup(int index)
{
    if (index < 0 || index >= labels.size())
        return ;

    int margin = labels.first()->y();
    int top = labels.at(index)->y() - margin;
    ui->scrollArea->scrollTo(top);
}

void SettingsController::adjustGroupSize()
{
    int margin = us->settingsMargin;
    const int fixedWidth = qMin(qMax(ui->scrollAreaWidgetContents->width() - margin * 2, us->settingsMinWidth), us->settingsMaxWidth);
    const int groupSpacing = 18;
    const int labelSpacing = 9;
    const int left = qMax(margin, (ui->scrollAreaWidgetContents->width() - fixedWidth) / 2);
    int top = groupSpacing;
    for (int i = 0; i < labels.size(); i++)
    {
        auto label = labels.at(i);
        auto box = boxes.at(i);
        label->setFixedWidth(fixedWidth);
        box->setFixedWidth(fixedWidth);
        label->adjustSize();
        box->adjustSize();
        box->setMinimumHeight(64);

        label->move(left, top);
        top += label->height() + labelSpacing;

        box->move(left, top);
        top += box->height() + groupSpacing;
    }

    ui->scrollAreaWidgetContents->setFixedHeight(top + labelSpacing);

    emit boxH(left, fixedWidth);
}

void SettingsController::setFind(QString key)
{
    bool finded = false;
    for (int i = 0; i < boxes.size(); i++)
    {
        auto w = qobject_cast<SettingsItemListBox*>(boxes.at(i));
        if (!w)
            continue;
        int rst = w->setFind(key);
        if (!finded && rst > -1)
        {
            focusGroup(i);
            finded = true;
        }
    }
}

void SettingsController::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    adjustGroupSize();
}

