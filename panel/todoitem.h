#ifndef TODOITEM_H
#define TODOITEM_H

#include <QListWidget>
#include <QListWidgetItem>
#include <QCheckBox>
#include <QLineEdit>
#include <QMimeData>
#include "resizeableitembase.h"
#include "watercirclebutton.h"
#include "todoline.h"

class TodoItem : public ResizeableItemBase
{
    Q_OBJECT
public:
    TodoItem(QWidget* parent);

    void fromJson(const MyJson &json) override;
    MyJson toJson() const override;

    void insertNewItem();
    void addItem(bool checked, const QString& text);
    void insertItem(int index, bool checked, const QString& text);
    void deleteItem(int index);

    virtual bool isUsing() const override;

public slots:
    void showMenu();

protected:
    void selectEvent(const QPoint &startPos) override;

    void resizeEvent(QResizeEvent *event) override;

private:
    QListWidget* listWidget;
    QList<TodoLine*> lines;
    WaterCircleButton* addButton;
};

#endif // TODOITEM_H
