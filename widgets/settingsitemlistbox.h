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

    void add(QPixmap pixmap, QString text, QString desc, bool* val);
    void add(QPixmap pixmap, QString text, QString desc, int* val);
    void add(QPixmap pixmap, QString text, QString desc, QString* val);
    void add(QPixmap pixmap, QString text, QString desc, QColor* val);
    void addOpen(QPixmap pixmap, QString text, QString desc);

private:
    InteractiveButtonBase* createBg(QPixmap pixmap, QString text, QString desc);

signals:

public slots:
    void adjusItemsSize();

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QVBoxLayout* mainLayout;
    QList<InteractiveButtonBase*> items;
};

#endif // SETTINGSITEMLISTBOX_H
