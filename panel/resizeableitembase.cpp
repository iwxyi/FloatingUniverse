#include <QApplication>
#ifdef Q_OS_WIN32
#include <windows.h>
#include <windowsx.h>
#endif
#include "resizeableitembase.h"

ResizeableItemBase::ResizeableItemBase(QWidget *parent) : PanelItemBase(parent)
{

}

bool ResizeableItemBase::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
#ifdef Q_OS_WIN32
    Q_UNUSED(eventType)
    MSG* msg = static_cast<MSG*>(message);
    switch(msg->message)
    {
    case WM_NCHITTEST:
        const auto ratio = devicePixelRatioF(); // 解决4K下的问题
        int xPos = static_cast<int>(GET_X_LPARAM(msg->lParam) / ratio - this->frameGeometry().x());
        int yPos = static_cast<int>(GET_Y_LPARAM(msg->lParam) / ratio - this->frameGeometry().y());
        if(xPos < boundaryWidth && yPos < boundaryWidth)                    //左上角
            *result = HTTOPLEFT;
        else if(xPos >= width() - boundaryWidth && yPos < boundaryWidth)          //右上角
            *result = HTTOPRIGHT;
        else if(xPos < boundaryWidth && yPos >= height() - boundaryWidth)         //左下角
            *result = HTBOTTOMLEFT;
        else if(xPos >= width() - boundaryWidth && yPos >= height() - boundaryWidth)//右下角
            *result = HTBOTTOMRIGHT;
        else if(xPos < boundaryWidth)                                     //左边
            *result =  HTLEFT;
        else if(xPos >= width() - boundaryWidth)                              //右边
            *result = HTRIGHT;
        /*else if(yPos<boundaryWidth)                                       //上边
            *result = HTTOP;*/
        else if(yPos >= height() - boundaryWidth)                             //下边
        {
            *result = HTBOTTOM;
        }
        else              //其他部分不做处理，返回false，留给其他事件处理器处理
           return false;
        return true;
    }
#else
    return QWidget::nativeEvent(eventType, message, result);
#endif
    return false;         //此处返回false，留给其他事件处理器处理
}
