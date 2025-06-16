#ifndef CARDITEM_H
#define CARDITEM_H

#include "resizeableitembase.h"

/**
 * 卡片组件
 * 提供了一个可视化的圆角矩形
 * 可以设置背景颜色、圆角半径等
 */
class CardItem : public ResizeableItemBase
{
public:
    CardItem(QWidget* parent);

    virtual MyJson toJson() const override;
    virtual void fromJson(const MyJson &json) override;

    void setColor(QColor c);
    void setRadius(int r);

protected:
    void paintEvent(QPaintEvent*) override;
    virtual void facileMenuEvent(FacileMenu* menu) override;
    virtual void saveMyModuleSize() override;

private:
    QColor bg = QColor(0xff, 0xff, 0xff, 192);
    int radius = 5;
};

#endif // CARDITEM_H
