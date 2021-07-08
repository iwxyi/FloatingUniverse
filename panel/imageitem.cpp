#include <QVBoxLayout>
#include <QLabel>
#include <QDesktopServices>
#include <QUrl>
#include <QClipboard>
#include "imageitem.h"
#include "fileutil.h"
#include "runtime.h"
#include "facilemenu.h"

ImageItem::ImageItem(QWidget *parent) : ResizeableItemBase(parent)
{
    imageLabel = new QLabel(this);
    imageLabel->setScaledContents(true);
    imageLabel->setAlignment(Qt::AlignCenter);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(imageLabel);
    layout->setMargin(selectBorder);

    setType(PanelItemType::ImageView);
    setMinimumSize(8, 8);
}

MyJson ImageItem::toJson() const
{
    MyJson json = ResizeableItemBase::toJson();

    json.insert("image", imageName);
    json.insert("bottom_layer", !autoRaise);

    return json;
}

void ImageItem::fromJson(const MyJson &json)
{
    ResizeableItemBase::fromJson(json);

    QString imageName = json.s("image");
    setImage(imageName);

    autoRaise = !json.b("bottom_layer");
}

void ImageItem::releaseResource()
{
    if (!imageName.isEmpty() && isFileExist(rt->PANEL_FILE_PATH + imageName))
    {
        deleteFile(rt->PANEL_FILE_PATH + imageName);
    }
}

void ImageItem::setImage(const QString& imageName)
{
    this->imageName = imageName;
    if (imageName.isEmpty())
    {
        // TODO: 显示默认提示图片

    }
    else
    {
        originPixmap = QPixmap(rt->PANEL_FILE_PATH + imageName);
        imageLabel->setPixmap(originPixmap);
    }
}

QString ImageItem::saveImageFile(const QPixmap &pixmap)
{
    // 保存到本地
    int val = 1;
    while (QFileInfo(rt->PANEL_FILE_PATH + QString::number(val) + ".png").exists())
        val++;
    QString iconName = QString::number(val) + ".png";

    pixmap.save(rt->PANEL_FILE_PATH + iconName);
    return iconName;
}

void ImageItem::facileMenuEvent(FacileMenu *menu)
{
    menu->addAction(QIcon(":/icons/open"), "打开 (&O)", [=]{
        triggerEvent();
    });

    bool invalid = originPixmap.isNull();

    menu->addAction(QIcon(":/icons/copy"), "复制图片 (&C)", [=]{
        QApplication::clipboard()->setPixmap(originPixmap);
    })->disable(invalid);

    menu->split()->addAction(QIcon(":/icons/size_proportion"), "恢复比例 (&A)", [=]{
        QSize iSize = originPixmap.size();
        QSize lSize = imageLabel->size();
        double iProp = iSize.width() / iSize.height();
        double lProp = lSize.width() / lSize.height();
        QSize maxSize = imageLabel->maximumSize();
        if (iProp > lProp) // 高度降低
        {
            imageLabel->setFixedSize(lSize.width(), int(lSize.width() / iProp));
        }
        else // 宽度降低
        {
            imageLabel->setFixedSize(int(lSize.height() * iProp), lSize.height());
        }
        adjustSize();
        imageLabel->setMinimumSize(QSize(1, 1));
        imageLabel->setMaximumSize(maxSize);
        emit modified();
    })->disable(invalid);

    menu->addAction(QIcon(":/icons/origin_size"), "原始大小 (&O)", [=]{
        imageLabel->resize(originPixmap.size());
        adjustSize();
        emit modified();
    })->disable(invalid);

    menu->split()->addAction(QIcon(":/icons/bottom_layer"), " 置于底层", [=]{
        autoRaise = !autoRaise;
        if (!autoRaise)
            emit lowerMe();
        else
            emit modified();
    })->check(!autoRaise);
}

void ImageItem::triggerEvent()
{
    QString path = rt->PANEL_FILE_PATH + imageName;
    if (isFileExist(path))
    {
        QDesktopServices::openUrl(QUrl("file:///" + path));
    }
}
