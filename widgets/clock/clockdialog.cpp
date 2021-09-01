#include "clockdialog.h"
#include "ui_clockdialog.h"

ClockDialog::ClockDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClockDialog)
{
    ui->setupUi(this);
}

ClockDialog::~ClockDialog()
{
    delete ui;
}
