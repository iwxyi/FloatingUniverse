#ifndef UNIVERSEPANEL_H
#define UNIVERSEPANEL_H

#include <QObject>
#include <QWidget>

class UniversePanel : public QWidget
{
    Q_OBJECT
public:
    explicit UniversePanel(QWidget *parent = nullptr);

signals:

public slots:
    void initPanel();
    void expandPanel();
    void foldPanel();

public:
    QRect screenGeometry() const;

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *) override;

private:
    bool expanding = false;
    bool animating = false;
};

#endif // UNIVERSEPANEL_H
