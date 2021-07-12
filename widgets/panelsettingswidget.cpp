#include "panelsettingswidget.h"
#include "ui_panelsettingswidget.h"

PanelSettingsWidget::PanelSettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PanelSettingsWidget)
{
    ui->setupUi(this);
}

PanelSettingsWidget::~PanelSettingsWidget()
{
    delete ui;
}
