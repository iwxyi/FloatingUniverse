#include <QVBoxLayout>
#include "todoitem.h"
#include "usettings.h"
#include "facilemenu.h"

TodoItem::TodoItem(QWidget *parent) : ResizeableItemBase(parent)
{
    setType(TodoList);

    // 列表
    listWidget = new QListWidget(this);
    auto layout = new QVBoxLayout(this);
    layout->addWidget(listWidget);

    listWidget->setResizeMode(QListWidget::Adjust);
    listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    listWidget->setCursor(Qt::ArrowCursor);
    listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect(listWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showMenu()));

    // 添加按钮
    addButton = new WaterCircleButton(QIcon(":/icons/add"), this);
    addButton->setFixedForePos();
    addButton->setFixedForeSize();
    addButton->setFixedSize(us->widgetSize, us->widgetSize);
    addButton->hide();

    connect(addButton, &InteractiveButtonBase::clicked, this, [=]{
        insertAndFocusItem(0);
    });

    listWidget->setStyleSheet("QListWidget { background: transparent; border: none; }");
}

void TodoItem::fromJson(const MyJson &json)
{
    ResizeableItemBase::fromJson(json);

    // 读取todo
    json.each("todos", [=](MyJson item) {
        JS(item, text);
        JB(item, checked);
        addItem(checked, text);
    });
}

MyJson TodoItem::toJson() const
{
    MyJson json = ResizeableItemBase::toJson();

    // 保存todo
    QJsonArray array;
    for (int i = 0; i < listWidget->count(); i++)
    {
        array.append(lines.at(i)->toJson());
    }
    json.insert("todos", array);

    return json;
}

/// 按顺序添加到底部的
void TodoItem::addItem(bool checked, const QString &text)
{
    insertItem(-1, checked, text);
}

void TodoItem::insertItem(int index, bool checked, const QString &text)
{
    auto line = new TodoLine(checked, text, listWidget);
    auto item = index < 0 ? new QListWidgetItem(listWidget) : new QListWidgetItem;
    if (index >= 0)
        listWidget->insertItem(index, item);
    listWidget->setItemWidget(item, line);
    if (index < 0)
        lines.append(line);
    else
        lines.insert(index, line);
    item->setSizeHint(line->sizeHint());

    connect(line, &TodoLine::modified, this, &TodoItem::modified);
    connect(line, &TodoLine::signalMenu, this, &TodoItem::showMenu);
    connect(line, &TodoLine::focused, listWidget, [=]{
        listWidget->setCurrentRow(lines.indexOf(line));
    });
    connect(line, &TodoLine::signalEsc, listWidget, [=]{
        listWidget->setFocus();
    });
    connect(line, &TodoLine::signalInsertNext, listWidget, [=](bool rev) {
        int row = listWidget->currentRow();
        if (row == -1)
            return ;
        int newRow = row + (rev ? 0 : 1);
        insertAndFocusItem(newRow);
    });
    connect(line, &TodoLine::signalMoveNext, listWidget, [=](bool rev) {
        int row = listWidget->currentRow();
        if (row == -1)
            return ;
        int newRow = row + (rev ? -1 : 1);
        if (newRow >= 0 && newRow < lines.count())
            lines.at(newRow)->setEdit();
    });
}

void TodoItem::insertAndFocusItem(int index)
{
    insertItem(index, false, "");
    listWidget->clearSelection();
    lines.at(index)->setEdit();
}

void TodoItem::deleteItem(int index)
{
    delete listWidget->takeItem(index);
    delete lines.takeAt(index);
}

bool TodoItem::isUsing() const
{
    if (listWidget->hasFocus())
        return true;
    foreach (auto line, lines)
    {
        if (line->hasFocus())
            return true;
    }
    return ResizeableItemBase::isUsing();
}

void TodoItem::showMenu()
{
    newFacileMenu;

    int row = listWidget->currentRow();

    menu->addAction(QIcon(":/icons/add"), "插入 (&I)", [=]{
        int row = listWidget->currentRow();
        if (row == -1)
            return ;
        insertAndFocusItem(row);
    })->disable(row == -1);

    menu->addAction(QIcon(":/icons/delete"), "删除行 (&D)", [=]{
        deleteAction();
    })->disable(row == -1);

    menu->exec();
    emit facileMenuUsed(menu);
}

void TodoItem::deleteAction()
{
    auto selects = listWidget->selectedItems();
    foreach (auto item, selects)
    {
        int row = listWidget->row(item);
        if (row == -1)
            continue;
        deleteItem(row);
    }
}

void TodoItem::selectEvent(const QPoint &startPos)
{
    ResizeableItemBase::selectEvent(startPos);

    listWidget->raise();
    addButton->raise();
}

void TodoItem::resizeEvent(QResizeEvent *event)
{
    ResizeableItemBase::resizeEvent(event);

    addButton->move(int(width() - addButton->width() * 1.2), int(height() - addButton->height() * 1.2));
}

void TodoItem::enterEvent(QEvent *event)
{
    ResizeableItemBase::enterEvent(event);

    addButton->show();
}

void TodoItem::leaveEvent(QEvent *event)
{
    ResizeableItemBase::leaveEvent(event);

    addButton->hide();
}

void TodoItem::keyPressEvent(QKeyEvent *e)
{
    auto key = e->key();
    // qDebug() << "item.key" << key;
    if (key == Qt::Key_Delete)
    {
        deleteAction();
        return ;
    }
    ResizeableItemBase::keyPressEvent(e);
}
