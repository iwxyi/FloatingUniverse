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

    bool hasFocus() const;
    void setEdit();

signals:
    void modified();
    void focused();
    void signalMenu();
    void signalEsc();
    void signalInsertNext(bool rev);
    void signalDeleteAction();
    void signalMoveNext(bool rev);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void paintEvent(QPaintEvent* e) override;

public slots:

private:
    QCheckBox* check;
    TodoLineEdit* edit;
};

#endif // TODOLINE_H
