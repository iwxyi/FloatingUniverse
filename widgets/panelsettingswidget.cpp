#include <QGraphicsDropShadowEffect>
#include <QScrollBar>
#include "panelsettingswidget.h"
#include "ui_panelsettingswidget.h"
#include "usettings.h"

PanelSettingsWidget::PanelSettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PanelSettingsWidget)
{
    ui->setupUi(this);

    initItems();
}

PanelSettingsWidget::~PanelSettingsWidget()
{
    delete ui;
}

void PanelSettingsWidget::initItems()
{
    auto w = new SettingsItemListBox(ui->scrollAreaWidgetContents);
    w->add(QPixmap(":/icons/file"), "文本", "这是一个描述", &us->useFacileDirMenu);
    w->add(QPixmap(":/icons/file"), "文本", "这是一个描述", &us->themeMainColor);
    w->add(QPixmap(":/icons/file"), "文本", "这是一个描述", &us->themeMainColor);
    w->add(QPixmap(":/icons/file"), "文本", "这是一个描述", &us->themeMainColor);
    w->add(QPixmap(":/icons/file"), "文本", "这是一个描述", &us->themeMainColor);
    addGroup(w, "悬浮面板");

    addGroup(new QWidget(ui->scrollAreaWidgetContents), "宇宙传送");
    addGroup(new QWidget(ui->scrollAreaWidgetContents), "交互优化");
    addGroup(new QWidget(ui->scrollAreaWidgetContents), "自动填充");
    addGroup(new QWidget(ui->scrollAreaWidgetContents), "使用数据");
    addGroup(new QWidget(ui->scrollAreaWidgetContents), "关于程序");
}

void PanelSettingsWidget::addGroup(QWidget *w, QString name)
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

void PanelSettingsWidget::focusGroup(int index)
{
    if (index < 0 || index >= labels.size())
        return ;

    int margin = labels.first()->y();
    int top = labels.at(index)->y() - margin;
    ui->scrollArea->verticalScrollBar()->setSliderPosition(top);
}

void PanelSettingsWidget::adjustGroupSize()
{
    int margin = 12;
    const int fixedWidth = qMin(qMax(ui->scrollAreaWidgetContents->width() - margin * 2, 300), 680);
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

void PanelSettingsWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    adjustGroupSize();
}

