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
    QPixmap getPixmap() const;
    static QString saveImageFile(const QPixmap& pixmap);

    void adjustSizeByImage(QSize maxxSize);

protected:
    virtual void facileMenuEvent(FacileMenu* menu) override;
    virtual void triggerEvent() override;
    virtual void saveMyModuleSize() override;

private:
    QLabel* imageLabel;

    QString imageName; // 文件名（包括后缀）
    QPixmap originPixmap;

    bool hideAfterTrigger = true;
};

#endif // IMAGEITEM_H
