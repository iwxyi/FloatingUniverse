#ifndef RESIZEABLEITEMBASE_H
#define RESIZEABLEITEMBASE_H

#include "panelitembase.h"

class ResizeableItemBase : public PanelItemBase
{
public:
    ResizeableItemBase(QWidget* parent);

protected:

protected:
    int boundaryWidth = 8;
};

#endif // RESIZEABLEITEMBASE_H
