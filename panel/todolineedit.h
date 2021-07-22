#ifndef TODOLINEEDIT_H
#define TODOLINEEDIT_H

#include <QLineEdit>

class TodoLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    TodoLineEdit(QWidget* parent);

signals:
    void focusIn();
    void focusOut();
    void finished();

protected:
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
};

#endif // TODOLINEEDIT_H
