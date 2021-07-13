#ifndef PANELSETTINGSWIDGET_H
#define PANELSETTINGSWIDGET_H

#include <QLabel>
#include "settingsitemlistbox.h"
#include "interactivebuttonbase.h"

namespace Ui {
class PanelSettingsWidget;
}

class PanelSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PanelSettingsWidget(QWidget *parent = nullptr);
    ~PanelSettingsWidget() override;

    void initItems();

    void addGroup(QWidget* w, QString name);
    void focusGroup(int index);
    void adjustGroupSize();

signals:
    void boxH(int left, int width);

protected:
    void resizeEvent(QResizeEvent *event) override;

    /* void addBool(QIcon icon, QString text, QString desc, bool* val);
    void addInt(QIcon icon, QString text, QString desc, int* val);
    void addString(QIcon icon, QString text, QString desc, QString* val);
    void addColor(QIcon icon, QString text, QString desc, QColor* val);
    void addOpen(QIcon icon, QString text, QString desc); */

private:
    QList<QLabel*> labels;
    QList<QWidget*> boxes;

    Ui::PanelSettingsWidget *ui;
};

#endif // PANELSETTINGSWIDGET_H
