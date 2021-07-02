#ifndef LONGTEXTITEM_H
#define LONGTEXTITEM_H

#include <QTextEdit>
#include "resizeableitembase.h"

class LongTextItem : public ResizeableItemBase
{
public:
    LongTextItem(QWidget* parent);

    virtual MyJson toJson() const;
    virtual void fromJson(const MyJson& json);

    void setText(const QString& text);
    void setHtml(const QString& html);
    void setText(const QString& text, bool enHtml);

    QString getText() const;
    bool isHtml() const;

    void editText();

private:
    QTextEdit* edit;
    bool enableHtml = false;
};

#endif // LONGTEXTITEM_H
