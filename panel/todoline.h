#ifndef TODOLINE_H
#define TODOLINE_H

#include <QCheckBox>
#include <QLineEdit>
#include "todolineedit.h"
#include "myjson.h"

class TodoLine : public QWidget
{
    Q_OBJECT
public:
    TodoLine(QWidget *parent);
    TodoLine(bool state, QString text, QWidget* parent);

    MyJson toJson() const;
    void fromJson(const MyJson& json);

    void setChecked(bool ch);
    bool isChecked() const;

    void setText(const QString& text);
    QString getText() const;

    void setEdit();

signals:
    void modified();
    void focused();
    void customMenu();

public slots:

private:
    QCheckBox* check;
    TodoLineEdit* edit;
};

#endif // TODOLINE_H
