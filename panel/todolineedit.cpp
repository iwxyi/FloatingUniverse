#include <QKeyEvent>
#include "todolineedit.h"
#include "facilemenu.h"

TodoLineEdit::TodoLineEdit(QWidget *parent) : QLineEdit(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
}

void TodoLineEdit::focusInEvent(QFocusEvent *e)
{
    TODO_EDIT_TYPE::focusInEvent(e);

    emit focusIn();
}

void TodoLineEdit::focusOutEvent(QFocusEvent *e)
{
    TODO_EDIT_TYPE::focusOutEvent(e);

    emit focusOut();
}

void TodoLineEdit::keyPressEvent(QKeyEvent *e)
{
    auto key = e->key();
    // qDebug() << "edit.key" << key;
    if (key == Qt::Key_Escape)
    {
        emit escKey();
        return ;
    }
    else if (key == Qt::Key_Enter || key == Qt::Key_Return)
    {
        auto modify = e->modifiers();
        emit enterKey(modify != Qt::NoModifier);
        return ;
    }

    TODO_EDIT_TYPE::keyPressEvent(e);
}

