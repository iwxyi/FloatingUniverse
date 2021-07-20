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

    void add(QPixmap pixmap, QString text, QString desc);
    void add(QPixmap pixmap, QString text, QString desc, QString key, bool* val);
    void add(QPixmap pixmap, QString text, QString desc, QString key, int* val, int min, int max, int step);
    void add(QPixmap pixmap, QString text, QString desc, QString key, double* val, double min, double max, double step);
    void add(QPixmap pixmap, QString text, QString desc, QString key, QString* val);
    void add(QPixmap pixmap, QString text, QString desc, QString key, QColor* val);
    void addOpen(QPixmap pixmap, QString text, QString desc, QString payload);
    void addOpen(QPixmap pixmap, QString text, QString desc, QUrl url);

    InteractiveButtonBase* lastItem() const;

    int setFind(QString key);

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
    QList<QString> texts; // 包含标题与描述，用于查找
    InteractiveButtonBase* _lastItem = nullptr;
};

#endif // SETTINGSITEMLISTBOX_H
