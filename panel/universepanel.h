#ifndef UNIVERSEPANEL_H
#define UNIVERSEPANEL_H

#include <QObject>
#include <QWidget>
#include "facilemenu.h"
#include "myjson.h"
#include "icontextitem.h"
#include "longtextitem.h"
#include "imageitem.h"
#include "carditem.h"
#include "todoitem.h"
#include "groupboxitem.h"

#define eachitem(x) for (int i = items.size() - 1; i >= 0; i--)\
{\
    auto item = items.at(i);\
    x\
}

class UniversePanel : public QWidget
{
    Q_OBJECT
    friend class FloatPanel;
public:
    explicit UniversePanel(QWidget *parent = nullptr);
    ~UniversePanel() override;

private:
    void initPanel();
    void initAction();
    void initGlobal();
    void readItems();
    QList<GroupBoxItem*> getGroupItems() const;
    IconTextItem *createLinkItem(QPoint pos, bool center, const QIcon& icon, const QString& text, const QString& link, PanelItemType type);
    IconTextItem *createLinkItem(QPoint pos, bool center, const QString& iconName, const QString& text, const QString& link, PanelItemType type);
    LongTextItem *createTextItem(QPoint pos, const QString& text, bool enableHtml);
    ImageItem *createImageItem(QPoint pos, const QPixmap& pixmap);
    ImageItem *createImageItem(QPoint pos, const QString& image);
    CardItem *createCardItem(QPoint pos);
    TodoItem *createTodoItem(QPoint pos);
    GroupBoxItem *createGroupBoxItem(QPoint pos, const QString& title);
    void createItemEvent(PanelItemBase* item, bool first);
    void deleteItemEvent(PanelItemBase* item);
    bool isMouseInPanel() const;
    bool hasItemUsing() const;

signals:
    void openSettings();
    void signalExpandPanel();
    void signalFoldPanel();
    void signalSetKeepFix(bool keep);
    void signalKeepPanelState(FuncType func);

public slots:
    void saveLater();
    void save();
    void selectAll(bool containIgnored = true);
    void unselectAll();
    void selectItem(PanelItemBase* item, const QPoint& pos = UNDEFINED_POS);
    void unselectItem(PanelItemBase* item);
    void unselectItems(const QList<PanelItemBase*>& items);
    void unselectGroupItems();
    void triggerItem(PanelItemBase* item);
    void raiseItem(PanelItemBase* item);
    void lowerItem(PanelItemBase* item);

private slots:
    void startDragSelectedItems(PanelItemBase* eventItem);
    void pasteFromClipboard(QPoint pos);
    void insertMimeData(const QMimeData *mime, QPoint pos);

public:
    bool getWebPageNameAndIcon(QString url, QString& pageName, QPixmap &pageIcon);

protected:
    void focusOutEvent(QFocusEvent *event) override;

    void paintEvent(QPaintEvent *) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    void contextMenuEvent(QContextMenuEvent *event) override;
    void showAddMenu(FacileMenu* addMenu);
    void addPastAction(FacileMenu* menu, QPoint pos, bool split = false);
    void keyPressEvent(QKeyEvent *event) override;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    bool pressing = false; // 左键按下
    QPoint pressPos;
    bool moving = false; // 正在移动items
    bool scening = false; // 正在移动画面
    QPoint draggingPos;
    FacileMenu* currentMenu = nullptr;
    bool _block_menu = false;
    bool _release_outter = false; // 鼠标松开的时候，是不是在外面
    QTimer* saveTimer;
    QTimer* keepTopTimer;

    QList<PanelItemBase*> items;
    QSet<PanelItemBase*> selectedItems;
};

#endif // UNIVERSEPANEL_H
