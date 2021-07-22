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
    connect(listWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showMenu()));

    // 添加按钮
    addButton = new WaterCircleButton(QIcon(":/icons/add"), this);
    addButton->setFixedForePos();
    addButton->setFixedForeSize();
    addButton->setFixedSize(us->buttonSize, us->buttonSize);

    connect(addButton, &InteractiveButtonBase::clicked, this, [=]{
        insertNewItem();
        lines.at(0)->setEdit();
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

void TodoItem::insertNewItem()
{
    insertItem(0, false, "");
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
    connect(line, &TodoLine::customMenu, this, &TodoItem::showMenu);
    connect(line, &TodoLine::focused, listWidget, [=]{
        listWidget->setCurrentRow(lines.indexOf(line));
    });
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
    return ResizeableItemBase::isUsing();
}

void TodoItem::showMenu()
{
    newFacileMenu;

    menu->addAction("插入", [=]{
        int row = listWidget->currentRow();
        if (row == -1)
            return ;
        insertItem(row, false, "");
        lines.at(row)->setEdit();
    });

    menu->addAction("删除", [=]{
        auto selects = listWidget->selectedItems();
        foreach (auto item, selects)
        {
            int row = listWidget->row(item);
            if (row == -1)
                continue;
            deleteItem(row);
        }
    });

    menu->exec();
    emit facileMenuUsed(menu);
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
