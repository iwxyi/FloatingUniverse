#ifndef CLOCKDIALOG_H
#define CLOCKDIALOG_H

#include <QWidget>

namespace Ui {
class ClockDialog;
}

class ClockDialog : public QWidget
{
    Q_OBJECT

public:
    explicit ClockDialog(QWidget *parent = nullptr);
    ~ClockDialog();

private:
    Ui::ClockDialog *ui;
};

#endif // CLOCKDIALOG_H
