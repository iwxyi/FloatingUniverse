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

    void setIcon(const QString& iconName);
    void setText(const QString& text);
    void setLink(const QString& link);
    void setFastOpen(bool fast);

    QString getText() const;
    QString getIconName() const;
    QString getLink() const;
    bool isFastOpen() const;

    virtual void jump(int range = 10) override;

protected:
    virtual void facileMenuEvent(FacileMenu *menu) override;
    virtual void triggerEvent() override;
    virtual bool canDrop(const QMimeData *mime) const override;
    void dropEvent(QDropEvent *event) override;

private:
    void showFacileDir(QString path, FacileMenu *parentMenu, int level);

private:
    QLabel* iconLabel;
    QLabel* textLabel;

    QString iconName;
    QString text;
    QString link; // 文件或者网址

    bool closeAfterClick = true;
    bool fastOpen = false; // 左键快速打开
    int openLevel = 3; // 打开的级别，文件多的时候越大越慢
};

#endif // ICONTEXTITEM_H
