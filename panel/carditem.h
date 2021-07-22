#ifndef CARDITEM_H
#define CARDITEM_H

#include "resizeableitembase.h"

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

private:
    QColor bg = QColor(0xff, 0xff, 0xff, 192);
    int radius = 5;
};

#endif // CARDITEM_H
