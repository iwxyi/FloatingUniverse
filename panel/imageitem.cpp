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
    setMinimumSize(ITEM_MIN_SIZE);
}

MyJson ImageItem::toJson() const
{
    MyJson json = ResizeableItemBase::toJson();

    json.insert("image", imageName);
    json.insert("bottom_layer", !autoRaise);
    json.insert("ignore_select", ignoreSelect);
    json.insert("hide_after_trigger", hideAfterTrigger);

    return json;
}

void ImageItem::fromJson(const MyJson &json)
{
    ResizeableItemBase::fromJson(json);

    QString imageName = json.s("image");
    setImage(imageName);

    autoRaise = !json.b("bottom_layer");
    ignoreSelect = json.b("ignore_select", ignoreSelect);
    hideAfterTrigger = json.b("hide_after_trigger", hideAfterTrigger);
    if (ignoreSelect)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
    }
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

void ImageItem::adjustSizeByImage(QSize maxxSize)
{
    QSize size = originPixmap.size();
    if (size.width() > maxxSize.width() || size.height() > maxxSize.height())
    {
        size.scale(maxxSize, Qt::KeepAspectRatio);
        QSize maxSize = imageLabel->maximumSize();
        imageLabel->setFixedSize(size);
        adjustSize();
        imageLabel->setMinimumSize(QSize(1, 1));
        imageLabel->setMaximumSize(maxSize);
    }
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
        iSize.scale(imageLabel->size(), Qt::KeepAspectRatio);
        QSize maxSize = imageLabel->maximumSize();
        imageLabel->setFixedSize(iSize);
        adjustSize();
        imageLabel->setMinimumSize(QSize(1, 1));
        imageLabel->setMaximumSize(maxSize);

        /* QSize lSize = imageLabel->size();
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
        imageLabel->setMaximumSize(maxSize); */
        emit modified();
    })->disable(invalid);

    menu->addAction(QIcon(":/icons/origin_size"), "原始大小 (&O)", [=]{
        imageLabel->resize(originPixmap.size());
        adjustSize();
        emit modified();
    })->disable(invalid);

    menu->split()->addAction(QIcon(":/icons/bottom_layer"), "置于底层", [=]{
        autoRaise = !autoRaise;
        if (!autoRaise)
            emit lowerMe();
        else
            emit modified();
    })->check(!autoRaise);

    menu->addAction(QIcon(":/icons/ignore_select"), "不被选中", [=]{
        ignoreSelect = !ignoreSelect;
        emit modified();
        setAttribute(Qt::WA_TransparentForMouseEvents, ignoreSelect);
        if (isSelected())
            emit unselectMe();
    })->check(ignoreSelect)->tooltip("屏蔽鼠标左键点击或者框选；不影响显示右键菜单");

    menu->addAction(QIcon(":/icons/eye"), "打开后隐藏", [=]{
        hideAfterTrigger = !hideAfterTrigger;
        emit modified();
    })->check(hideAfterTrigger)->disable(ignoreSelect)->tooltip("打开此项目后，自动隐藏悬浮面板");
}

void ImageItem::triggerEvent()
{
    QString path = rt->PANEL_FILE_PATH + imageName;
    if (isFileExist(path))
    {
        QDesktopServices::openUrl(QUrl("file:///" + path));

        if (hideAfterTrigger)
            emit hidePanel();
    }
}
