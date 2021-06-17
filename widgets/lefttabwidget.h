#ifndef LEFTTABWIDGET_H
#define LEFTTABWIDGET_H

#include <QTabBar>
#include <QTabWidget>
#include <QStyle>
#include <QStylePainter>
#include <QStyleOptionTab>

class LeftTabBar : public QTabBar
{
public:
    QSize tabSizeHint(int index) const
    {
        QSize s = QTabBar::tabSizeHint(index);
        s.transpose();
        return s;
    }

protected:
    void paintEvent(QPaintEvent * /*event*/)
    {
        QStylePainter painter(this);
        QStyleOptionTab opt;

        for (int i = 0; i < count(); i++)
        {
            initStyleOption(&opt, i);
            if (this->currentIndex() == i) // 选中项
                painter.drawControl(QStyle::CE_TabBarTabShape, opt); // 画背景
            painter.save();

            QSize s = opt.rect.size();
            s.transpose();
            QRect r(QPoint(), s);
            r.moveCenter(opt.rect.center());
            opt.rect = r;

            QPoint c = tabRect(i).center();
            painter.translate(c);
            painter.rotate(90);
            painter.translate(-c);
            painter.drawControl(QStyle::CE_TabBarTabLabel, opt); // 画文字
            painter.restore();
        }
    }
};

class LeftTabWidget : public QTabWidget
{
public:
    LeftTabWidget(QWidget *parent = nullptr) : QTabWidget(parent)
    {
        setTabBar(new LeftTabBar);
        setTabPosition(QTabWidget::West);
    }
};

#endif // LEFTTABWIDGET_H
