#ifndef PANELSETTINGSWIDGET_H
#define PANELSETTINGSWIDGET_H

#include <QWidget>

namespace Ui {
class PanelSettingsWidget;
}

class PanelSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PanelSettingsWidget(QWidget *parent = nullptr);
    ~PanelSettingsWidget();

private:
    Ui::PanelSettingsWidget *ui;
};

#endif // PANELSETTINGSWIDGET_H
