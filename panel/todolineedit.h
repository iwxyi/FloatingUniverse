#ifndef TODOLINEEDIT_H
#define TODOLINEEDIT_H

#include <QLineEdit>

#define TODO_EDIT_TYPE QLineEdit

class TodoLineEdit : public TODO_EDIT_TYPE
{
    Q_OBJECT
public:
    TodoLineEdit(QWidget* parent);

signals:
    void focusIn();
    void focusOut();
    void escKey();
    void enterKey(bool ctrl);
    void deleteMe(bool next);

protected:
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
};

#endif // TODOLINEEDIT_H
