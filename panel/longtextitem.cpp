#include <QVBoxLayout>
#include "longtextitem.h"

LongTextItem::LongTextItem(QWidget *parent) : ResizeableItemBase(parent)
{
    setType(LongText);

    edit = new QTextEdit(this);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(edit);

    layout->setMargin(this->boundaryWidth);
    edit->setStyleSheet("QTextEdit { background: transparent; border: none; }");

    connect(edit, &QTextEdit::textChanged, this, [=]{
        emit modified();
    });
}

MyJson LongTextItem::toJson() const
{
    MyJson json = ResizeableItemBase::toJson();

    json.insert("enableHtml", enableHtml);
    json.insert("text", getText());

    return json;
}

void LongTextItem::fromJson(const MyJson &json)
{
    ResizeableItemBase::fromJson(json);

    int w = json.i("width", this->width());
    int h = json.i("height", this->height());
    resize(w, h);

    enableHtml = json.b("enableHtml", enableHtml);
    setText(json.s("text"), enableHtml);
}

void LongTextItem::setText(const QString &text)
{
    edit->setPlainText(text);
    enableHtml = false;
}

void LongTextItem::setHtml(const QString &html)
{
    edit->setHtml(html);
    enableHtml = true;
}

void LongTextItem::setText(const QString &text, bool enHtml)
{
    if (enHtml)
        setHtml(text);
    else
        setText(text);
}

QString LongTextItem::getText() const
{
    return enableHtml ? edit->toHtml() : edit->toPlainText();
}

bool LongTextItem::isHtml() const
{
    return enableHtml;
}

void LongTextItem::editText()
{
    edit->setFocus();
}
