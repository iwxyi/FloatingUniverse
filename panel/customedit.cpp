#include "customedit.h"

CustomEdit::CustomEdit(QWidget *parent) : QTextEdit(parent)
{

}

void CustomEdit::focusInEvent(QFocusEvent *e)
{
    QTextEdit::focusInEvent(e);

    emit focusIn();
}

void CustomEdit::focusOutEvent(QFocusEvent *e)
{
    QTextEdit::focusOutEvent(e);

    emit focusOut();
}
