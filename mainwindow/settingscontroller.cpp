#include <QGraphicsDropShadowEffect>
#include <QScrollBar>
#include "settingscontroller.h"
#include "ui_panelsettingswidget.h"
#include "usettings.h"

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
    auto w = new SettingsItemListBox(ui->scrollAreaWidgetContents);
    w->add(QPixmap(":/icons/st/bgColor"), "背景颜色", "", "panel/bgColor", &us->panelBgColor);
    w->add(QPixmap(":/icons/st/selectBgColor"), "选择区域颜色", "", "panel/selectRectColor", &us->panelSelectRectColor);
    w->add(QPixmap(":/icons/st/selectEdgeColor"), "选中项边框颜色", "", "panel/selectEdgeColor", &us->panelSelectEdgeColor);
    w->add(QPixmap(":/icons/st/hoverEdgeColor"), "候选项边框颜色", "", "panel/hoverEdgeColor", &us->panelHoverEdgeColor);
    w->add(QPixmap(":/icons/st/blurRadius"), "毛玻璃模糊半径", "", "panel/blurRadius", &us->panelBlurRadius, 0, 255, 16);
    w->add(QPixmap(":/icons/st/blurOpacity"), "毛玻璃透明度", "", "panel/blurOpacity", &us->panelBlurOpacity, 0, 255, 16);
    addGroup(w, "悬浮面板");

    w = new SettingsItemListBox(ui->scrollAreaWidgetContents);
    w->add(QPixmap(":/icons/st/iconSize"), "图标大小", "最大尺寸(非强制,可能更小)，新图标生效", "panel/iconSize", &us->panelIconSize, 16, 255, 16);
    w->add(QPixmap(":/icons/st/spatialMoveRatio"), "空间移动倍率", "右键移动画布的速度的几何倍率", "interactive/spatialMoveRatio", &us->spatialMoveRatio, 1, 10, 1);
    addGroup(w, "宇宙荧光");

    w = new SettingsItemListBox(ui->scrollAreaWidgetContents);
    w->add(QPixmap(":/icons/st/TODO"), "待添加", "");
    addGroup(w, "空间传送");

    w = new SettingsItemListBox(ui->scrollAreaWidgetContents);
    w->add(QPixmap(":/icons/st/fileName"), "同步修改文件名", "修改文件快捷方式的名字时，询问修改文件名", "interactive/modifyFileNameSync", &us->modifyFileNameSync);
    w->add(QPixmap(":/icons/st/fastOpenDir"), "快速展开目录", "默认使用菜单的形式打开文件夹", "interactive/fastOpenDir", &us->fastOpenDir);
    w->add(QPixmap(":/icons/st/fastOpenDirLevel"), "展开目录级别", "", "interactive/fastOpenDirLevel", &us->fastOpenDirLevel, 1, 10, 1);
    addGroup(w, "交互优化");

    w = new SettingsItemListBox(ui->scrollAreaWidgetContents);
    w->add(QPixmap(":/icons/st/bootCount"), "程序启动次数", "", "panel/bootCount", &us->bootCount, 0, 0, 1);
    w->lastItem()->setEnabled(false);
    w->add(QPixmap(":/icons/st/linkOpenCount"), "打开链接次数", "", "panel/linkOpenCount", &us->linkOpenCount, 0, 0, 1);
    w->lastItem()->setEnabled(false);
    addGroup(w, "使用数据");

    w = new SettingsItemListBox(ui->scrollAreaWidgetContents);
    w->addOpen(QPixmap(":/icons/appicon"), "悬浮宇宙", "本程序由杭州懒一夕智能科技有限公司开发", QUrl("http://lyixi.com"));
    w->addOpen(QPixmap(":/icons/st/GitHub"), "GitHub", "点击前往开源仓库：iwxyi/FloatingUniverse", QUrl("https://github.com/iwxyi/FloatingUniverse"));
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

