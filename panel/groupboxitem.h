#ifndef GROUPBOXITEM_H
#define GROUPBOXITEM_H

#include "resizeableitembase.h"
#include <QScrollArea>

/**
 * 组框组件
 * 提供了一个可视化的组框
 * 其他组件中心点在组框上的，视为被包含在其中
 * 移动组框的时候，其他组件会跟随移动
 */
class GroupBoxItem : public ResizeableItemBase
{
    Q_OBJECT
public:
    GroupBoxItem(QWidget* parent);

    virtual void initResource() override;
    virtual MyJson toJson() const override;
    virtual void fromJson(const MyJson &json) override;

    QString getTitle() const;
    void setTitle(const QString& title);
    QWidget* getGroupArea() const;
    QList<PanelItemBase*> getSubItems() const;
    void autoArrange();

signals:
    void signalGetSubItems(QList<PanelItemBase*>* items) const;

public slots:
    void slotShowGroupMenu();

private:
    QString title;
    QLabel* label = nullptr;
    QScrollArea* scrollArea = nullptr;
    QWidget* contentWidget = nullptr;
};

#endif // GROUPBOXITEM_H
