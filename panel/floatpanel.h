#ifndef FLOATPANEL_H
#define FLOATPANEL_H

#include <QWidget>
#include "facilemenu.h"

class UniversePanel;

class FloatPanel : public QWidget
{
    Q_OBJECT
public:
    explicit FloatPanel(QWidget *parent = nullptr);

    void initPanel();

signals:
    void openSettings();

public slots:
    void expandPanel();
    void foldPanel();
    void keepPanelState(FuncType func);
    void setPanelFixed(bool f);
    QRect screenGeometry() const;

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *) override;
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;

private:
    UniversePanel* panel = nullptr;

    bool expanding = false;
    bool animating = false;
    bool fixing = false;
    int boundaryWidth = 8;
    QPixmap panelBlurPixmap;

    bool _prev_fixing = false;
};

#endif // FLOATPANEL_H
