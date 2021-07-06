#include <QApplication>
#ifdef Q_OS_WIN32
#include <windows.h>
#include <windowsx.h>
#endif
#include "resizeableitembase.h"

ResizeableItemBase::ResizeableItemBase(QWidget *parent) : PanelItemBase(parent)
{

}
