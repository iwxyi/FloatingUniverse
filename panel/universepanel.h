#ifndef UNIVERSEPANEL_H
#define UNIVERSEPANEL_H

#include <QObject>
#include <QWidget>
#include "facilemenu.h"
#include "myjson.h"
#include "icontextitem.h"

class UniversePanel : public QWidget
{
    Q_OBJECT
public:
    explicit UniversePanel(QWidget *parent = nullptr);
    ~UniversePanel() override;

private:
    void initPanel();
    void readItems();
    IconTextItem *createNewItem(QPoint pos, const QIcon &icon, const QString& text);
    IconTextItem *createNewItem(QPoint pos, const QString &iconName, const QString& text);
    void connectItem(PanelItemBase* item);
    QString saveIcon(const QIcon& icon) const;
    QString saveIcon(const QPixmap& pixmap) const;
    void deleteItem(PanelItemBase* item);

signals:

public slots:
    void expandPanel();
    void foldPanel();
    void save();
    void selectAll();
    void unselectAll();
    void selectItem(PanelItemBase* item);
    void unselectItem(PanelItemBase* item);
    void triggerItem(PanelItemBase* item);

private slots:
    void startDragSelectedItems();

public:
    QRect screenGeometry() const;
    bool getWebPageNameAndIcon(QString url, QString& pageName, QPixmap &pageIcon);

protected:
    void closeEvent(QCloseEvent *) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

    void paintEvent(QPaintEvent *) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    void contextMenuEvent(QContextMenuEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;

private:
    bool expanding = false;
    bool animating = false;
    bool fixing = false;

    int boundaryWidth = 8;

    bool pressing = false; // 左键按下
    QPoint pressPos;
    bool moving = false; // 正在移动items
    bool scening = false; // 正在移动画面
    QPoint draggingPos;
    FacileMenu* currentMenu = nullptr;
    bool _block_menu = false;

    QList<PanelItemBase*> items;
    QSet<PanelItemBase*> selectedItems;
};

#endif // UNIVERSEPANEL_H
