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

    void addItem(bool checked, const QString& text);
    QListWidgetItem *insertItem(int index, bool checked, const QString& text);
    void insertAndFocusItem(int index = 0);
    void deleteItem(int index);

    virtual bool isUsing() const override;

public slots:
    void showMenu();
    void actionDelete();

protected:
    void selectEvent(const QPoint &startPos) override;
    void resizeEvent(QResizeEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *e) override;
    virtual void saveMyModuleSize() override;

private:
    QListWidget* listWidget;
    QList<TodoLine*> lines;
    WaterCircleButton* addButton;
};

#endif // TODOITEM_H
