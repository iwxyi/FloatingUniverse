#ifndef PANELSETTINGSWIDGET_H
#define PANELSETTINGSWIDGET_H

#include <QLabel>
#include "settingsitemlistbox.h"
#include "interactivebuttonbase.h"

namespace Ui {
class PanelSettingsWidget;
}

class SettingsController : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsController(QWidget *parent = nullptr);
    ~SettingsController() override;

    void initItems();

    void addGroup(QWidget* w, QString name);
    void focusGroup(int index);
    void adjustGroupSize();

    void setFind(QString key);

signals:
    void boxH(int left, int width);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QList<QLabel*> labels;
    QList<QWidget*> boxes;

    Ui::PanelSettingsWidget *ui;
};

#endif // PANELSETTINGSWIDGET_H
