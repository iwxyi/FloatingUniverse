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
    auto modifier = e->modifiers();
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
    else if (modifier & Qt::AltModifier)
    {
        if (key == Qt::Key_Delete) // Alt+Delete 删除行
        {
            emit deleteMe(true);
            return ;
        }
        else if (key == Qt::Key_Backspace) // Alt+Backspace 清空行
        {
            this->clear();
            return ;
        }
    }
    else if (modifier == Qt::NoModifier)
    {
        if (key == Qt::Key_Delete || key == Qt::Key_Backspace) // Delete / Backspace 删除
        {
            if (this->text().isEmpty())
            {
                emit deleteMe(key == Qt::Key_Delete);
                return ;
            }
        }
    }

    TODO_EDIT_TYPE::keyPressEvent(e);
}

