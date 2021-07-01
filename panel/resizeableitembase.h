#ifndef RESIZEABLEITEMBASE_H
#define RESIZEABLEITEMBASE_H

#include "panelitembase.h"

class ResizeableItemBase : public PanelItemBase
{
public:
    ResizeableItemBase(QWidget* parent);

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;

private:
    int boundaryWidth = 8;
};

#endif // RESIZEABLEITEMBASE_H
