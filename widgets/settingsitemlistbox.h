#ifndef SETTINGSITEMLISTBOX_H
#define SETTINGSITEMLISTBOX_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include "interactivebuttonbase.h"

class SettingsItemListBox : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsItemListBox(QWidget *parent = nullptr);

    void add(QPixmap pixmap, QString text, QString desc, QString key, bool* val);
    void add(QPixmap pixmap, QString text, QString desc, QString key, int* val, int min = 0, int max = 99999, int step = 1);
    void add(QPixmap pixmap, QString text, QString desc, QString key, QString* val);
    void add(QPixmap pixmap, QString text, QString desc, QString key, QColor* val);
    void addOpen(QPixmap pixmap, QString text, QString desc, QString payload);

private:
    InteractiveButtonBase* createBg(QPixmap pixmap, QString text, QString desc);

signals:
    void openPage(QString payload);

public slots:
    void adjusItemsSize();

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QVBoxLayout* mainLayout;
    QList<InteractiveButtonBase*> items;
};

#endif // SETTINGSITEMLISTBOX_H
