#ifndef CUSTOMEDIT_H
#define CUSTOMEDIT_H

#include <QTextEdit>

class CustomEdit : public QTextEdit
{
    Q_OBJECT
public:
    CustomEdit(QWidget* parent);

signals:
    void focusIn();
    void focusOut();

protected:
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
};

#endif // CUSTOMEDIT_H
