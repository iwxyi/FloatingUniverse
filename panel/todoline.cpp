#include <QHBoxLayout>
#include "todoline.h"

TodoLine::TodoLine(QWidget *parent) : TodoLine(false, "", parent)
{

}

TodoLine::TodoLine(bool state, QString text, QWidget *parent)
    : QWidget(parent)
{
    check = new QCheckBox(this);
    edit = new TodoLineEdit(this);

    auto layout = new QHBoxLayout(this);
    layout->addWidget(check);
    layout->addWidget(edit);
    layout->setMargin(2);

    check->setCursor(Qt::PointingHandCursor);
    edit->setStyleSheet("QLineEdit { background: transparent; border: none; }");

    setChecked(state);
    setText(text);

    connect(check, SIGNAL(stateChanged(int)), this, SIGNAL(modified()));
    connect(edit, SIGNAL(customContextMenuRequested(const QPoint&)), this, SIGNAL(customMenu()));
    connect(edit, SIGNAL(editingFinished()), this, SIGNAL(modified()));
    connect(edit, SIGNAL(focusIn()), this, SIGNAL(focused()));
}

MyJson TodoLine::toJson() const
{
    MyJson json;
    json.insert("checked", check->isChecked());
    json.insert("text", edit->text());
    return json;
}

void TodoLine::fromJson(const MyJson &json)
{
    JB(json, checked);
    JS(json, text);
    check->setChecked(checked);
    edit->setText(text);
}

void TodoLine::setChecked(bool ch)
{
    check->setChecked(ch);
}

void TodoLine::setText(const QString &text)
{
    edit->setText(text);
}

QString TodoLine::getText() const
{
    return edit->text();
}

void TodoLine::setEdit()
{
    edit->setFocus();
}
