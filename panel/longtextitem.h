#ifndef LONGTEXTITEM_H
#define LONGTEXTITEM_H

#include <QTextEdit>
#include "resizeableitembase.h"
#include "customedit.h"

class LongTextItem : public ResizeableItemBase
{
    Q_OBJECT
public:
    LongTextItem(QWidget* parent);

    virtual MyJson toJson() const override;
    virtual void fromJson(const MyJson& json) override;
    virtual bool isUsing() const override;

    void setText(const QString& text);
    void setHtml(const QString& html);
    void setText(const QString& text, bool enHtml);

    QString getText() const;
    bool isHtml() const;

    void adjustSizeByText();
    void editText();
    void cancelEdit();

public slots:
    void showEditMenu();

protected:
    virtual void selectEvent() override;
    virtual void unselectEvent() override;

private:
    CustomEdit* edit;
    bool enableHtml = false;
};

#endif // LONGTEXTITEM_H
