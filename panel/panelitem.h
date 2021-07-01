#ifndef PANELITEM_H
#define PANELITEM_H

#include <QObject>
#include <QLabel>
#include "myjson.h"

class PanelItem : public QWidget
{
    Q_OBJECT
public:
    explicit PanelItem(QWidget *parent = nullptr);

    MyJson toJson() const;
    static PanelItem* fromJson(const MyJson& json, QWidget* parent = nullptr);

    void setIcon(const QString& iconName);
    void setText(const QString& text);
    void setLink(const QString& link);

signals:
    void triggered();
    void pressed();
    void needSave();
    void moveItems(QPoint delta);

public slots:
    void showSelect(bool sh);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

public:
    QLabel* iconLabel;
    QLabel* textLabel;
    QWidget* selectWidget;

    const int selectBorder = 2;
    QPoint pressPos;
    QPoint pressGlobalPos;
    bool dragged = false; // 按压下来有没有拖拽过

    QString iconName;
    QString text;
    QString link; // 文件或者网址
};

#endif // PANELITEM_H
