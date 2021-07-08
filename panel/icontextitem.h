#ifndef ICONTEXTITEM_H
#define ICONTEXTITEM_H

#include "panelitembase.h"

class IconTextItem : public PanelItemBase
{
    Q_OBJECT
public:
    IconTextItem(QWidget* parent);

    virtual MyJson toJson() const override;
    virtual void fromJson(const MyJson& json) override;
    virtual void releaseResource() override;

    void setIcon(const QString& iconName);
    void setText(const QString& text);
    void setLink(const QString& link);
    void setFastOpen(bool fast);

    QString getText() const;
    QString getIconName() const;
    QString getLink() const;
    bool isFastOpen() const;

    static QString saveIconFile(const QIcon& icon);
    static QString saveIconFile(const QPixmap& pixmap);

    virtual void shake(int range = 5);
    virtual void nod(int range = 5);
    virtual void jump(int range = 10);
    virtual void shrink(int range = 10);
    virtual void jitter(QPoint startPos, int range = 10);

protected:
    virtual void facileMenuEvent(FacileMenu *menu) override;
    virtual void triggerEvent() override;
    virtual bool canDropEvent(const QMimeData *mime) override;
    void dropEvent(QDropEvent *event) override;
    virtual void hoverEvent(const QPoint &startPos) override;

private:
    void showFacileDir(QString path, FacileMenu *parentMenu, int level);

private:
    QLabel* iconLabel;
    QLabel* textLabel;

    QString iconName; // 图标文件名（包括后缀）
    QString text; // 显示的标题
    QString link; // 文件或者网址

    bool hideAfterTrigger = true;
    bool fastOpen = false; // 左键快速打开
    int openLevel = 3; // 打开的级别，文件多的时候越大越慢

    bool _shaking = false;
    bool _nodding = false;
    bool _jumping = false;
    bool _shrinking = false;
    bool _jittering = false;
};

#endif // ICONTEXTITEM_H
