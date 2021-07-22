#include <QHBoxLayout>
#include <QKeyEvent>
#include <QDebug>
#include <QStyleOption>
#include <QPainter>
#include "todoline.h"

TodoLine::TodoLine(QWidget *parent) : TodoLine(false, "", parent)
{
}

TodoLine::TodoLine(bool state, QString text, QWidget *parent)
    : QWidget(parent)
{
    check = new QCheckBox(this);
    edit = new TodoLineEdit(this);
    setObjectName("TodoLine");
    check->setObjectName("TodoCheck");
    edit->setObjectName("TodoEdit");

    auto layout = new QHBoxLayout(this);
    layout->addWidget(check);
    layout->addWidget(edit);
    layout->setMargin(2);

    check->setCursor(Qt::PointingHandCursor);
    edit->setStyleSheet("QLineEdit { background: transparent; border: none; }");
    // setFocusProxy(edit); // 想要自定义更多事件，暂且不用它了

    setChecked(state);
    setText(text);

    connect(check, SIGNAL(stateChanged(int)), this, SIGNAL(modified()));
    connect(edit, SIGNAL(customContextMenuRequested(const QPoint&)), this, SIGNAL(signalMenu()));
    connect(edit, SIGNAL(editingFinished()), this, SIGNAL(modified()));
    connect(edit, SIGNAL(focusIn()), this, SIGNAL(focused()));
    connect(edit, SIGNAL(escKey()), this, SIGNAL(signalEsc()));
    connect(edit, SIGNAL(enterKey(bool)), this, SIGNAL(signalInsertNext(bool)));
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

bool TodoLine::isChecked() const
{
    return check->isChecked();
}

void TodoLine::setText(const QString &text)
{
    edit->setText(text);
}

QString TodoLine::getText() const
{
    return edit->text();
}

bool TodoLine::hasFocus() const
{
    return QWidget::hasFocus() || edit->hasFocus();
}

void TodoLine::setEdit()
{
    edit->setFocus();
}

void TodoLine::keyPressEvent(QKeyEvent *e)
{
    auto key = e->key();
    auto modifier = e->modifiers();
    // qDebug() << "line.key" << key;
    if (key == Qt::Key_Delete)
    {
        emit signalDeleteAction();
        return ;
    }
    else if (key == Qt::Key_Up && !modifier)
    {
        emit signalMoveNext(true);
        return ;
    }
    else if (key == Qt::Key_Down && !modifier)
    {
        emit signalMoveNext(false);
        return ;
    }
    QWidget::keyPressEvent(e);
}

void TodoLine::paintEvent(QPaintEvent *e)
{
    QStyleOption option;
    option.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &option, &p, this);
}
