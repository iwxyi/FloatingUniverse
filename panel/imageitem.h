#ifndef IMAGEITEM_H
#define IMAGEITEM_H

#include "resizeableitembase.h"

class ImageItem : public ResizeableItemBase
{
public:
    ImageItem(QWidget* parent);

    virtual MyJson toJson() const override;
    virtual void fromJson(const MyJson &json) override;
    virtual void releaseResource() override;

    void setImage(const QString &imageName);

    static QString saveImageFile(const QPixmap& pixmap);

protected:
    virtual void facileMenuEvent(FacileMenu* menu) override;
    virtual void triggerEvent() override;

private:
    QLabel* imageLabel;

    QString imageName; // 文件名（包括后缀）
    QPixmap originPixmap;
};

#endif // IMAGEITEM_H
