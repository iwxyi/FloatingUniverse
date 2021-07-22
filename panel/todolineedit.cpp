#include <QKeyEvent>
#include "todolineedit.h"
#include "facilemenu.h"

TodoLineEdit::TodoLineEdit(QWidget *parent) : QLineEdit(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
}

void TodoLineEdit::focusInEvent(QFocusEvent *e)
{
    QLineEdit::focusInEvent(e);

    emit focusIn();
}

void TodoLineEdit::focusOutEvent(QFocusEvent *e)
{
    QLineEdit::focusOutEvent(e);

    emit focusOut();
}

void TodoLineEdit::keyPressEvent(QKeyEvent *e)
{
    auto key = e->key();
    if (key == Qt::Key_Escape || key == Qt::Key_Enter || key == Qt::Key_Return)
    {
        emit finished();
        return ;
    }

    QLineEdit::keyPressEvent(e);
}

