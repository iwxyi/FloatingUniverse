#ifndef RESIZEABLEITEMBASE_H
#define RESIZEABLEITEMBASE_H

#include "panelitembase.h"
#include "moveablewidget.h"

class ResizeableItemBase : public PanelItemBase
{
public:
    ResizeableItemBase(QWidget* parent);

public slots:
    void adjustCornerPos();

protected:
    MyJson toJson() const override;
    void fromJson(const MyJson &json) override;

    void resizeEvent(QResizeEvent *event) override;
    void selectEvent() override;

protected:
    int boundaryWidth = 8;

    MoveableWidget* resizeCorner[4]; // 从左上角开始，顺时针遍历
};

#endif // RESIZEABLEITEMBASE_H
