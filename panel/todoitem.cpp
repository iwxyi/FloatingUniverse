#include <QVBoxLayout>
#include <QClipboard>
#include "todoitem.h"
#include "usettings.h"
#include "facilemenu.h"

TodoItem::TodoItem(QWidget *parent) : ResizeableItemBase(parent)
{
    setType(TodoList);

    // 列表
    listWidget = new QListWidget(this);
    listWidget->setObjectName("ListWidget");
    auto layout = new QVBoxLayout(this);
    layout->addWidget(listWidget);

    listWidget->setResizeMode(QListWidget::Adjust);
    listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    listWidget->setCursor(Qt::ArrowCursor);
    listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect(listWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showMenu()));

    // 添加按钮
    addButton = new WaterCircleButton(QIcon(":/icons/add"), this);
    addButton->setObjectName("AddButton");
    addButton->setFixedForePos();
    addButton->setFixedForeSize();
    addButton->setFixedSize(us->widgetSize, us->widgetSize);
    addButton->hide();

    connect(addButton, &InteractiveButtonBase::clicked, this, [=]{
        insertAndFocusItem(0);
    });

    listWidget->setStyleSheet("QListWidget { background: transparent; border: none; }");
    listWidget->setFocusPolicy(Qt::StrongFocus);
    this->setFocusProxy(listWidget);

    // 添加action
    auto createAction = [=](QString key, FuncType fun){
        QAction* action = new QAction(listWidget);
        action->setShortcut(QKeySequence(key));
        connect(action, &QAction::triggered, this, fun);
        listWidget->addAction(action);
    };

    /* createAction("ctrl+a", [=]{ // 快捷键冲突了
        listWidget->selectAll();
    }); */
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

QListWidgetItem* TodoItem::insertItem(int index, bool checked, const QString &text)
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
        if (!isSelected())
            emit selectMe();
        if (listWidget->selectedItems().contains(listWidget->item(lines.indexOf(line))))
            // 这个item已经选中了，不需要重复选中
            return ;
        int index = lines.indexOf(line);
        auto modifier = QApplication::keyboardModifiers();
        if (modifier == Qt::NoModifier)
        {
            listWidget->clearSelection();
            listWidget->setCurrentRow(index);
        }
        else if (modifier & Qt::ShiftModifier)
        {
            int current = listWidget->currentRow();
            if (current < 0)
                current = 0;
            // 从 current ~ index 之间的选中
            if (current > index)
            {
                int temp = current;
                current = index;
                index = temp;
            }
            for (int i = current; i <= index; i++)
            {
                listWidget->setCurrentRow(i, QItemSelectionModel::Select);
            }
            listWidget->setCurrentRow(lines.indexOf(line), QItemSelectionModel::Current);
        }
        else if (modifier & Qt::ControlModifier)
        {
            // 多选（此项必须选中）
            listWidget->setCurrentRow(index, QItemSelectionModel::Select);
        }
        else if (modifier & Qt::AltModifier)
        {
            // 全部反选
            int current = listWidget->currentRow();
            if (current < 0)
                current = 0;
            // 从 current ~ index 之间的选中
            if (current > index)
            {
                int temp = current;
                current = index;
                index = temp;
            }
            for (int i = current; i <= index; i++)
            {
                listWidget->setCurrentRow(i, QItemSelectionModel::Toggle);
            }
            listWidget->setCurrentRow(lines.indexOf(line), QItemSelectionModel::Current);
        }
    });
    connect(line, &TodoLine::signalEsc, listWidget, [=]{
        listWidget->clearSelection();
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

    return item;
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
    menu->setTipArea("Shift+Enter");

    int row = listWidget->currentRow();

    menu->addAction(QIcon(":/icons/check"), "勾选 (&C)", [=]{
        auto selects = listWidget->selectedItems();
        foreach (auto item, selects)
        {
            int row = listWidget->row(item);
            lines.at(row)->setChecked(true);
        }
    })->disable(row == -1);

    menu->addAction(QIcon(":/icons/uncheck"), "反勾选 (&R)", [=]{
        auto selects = listWidget->selectedItems();
        foreach (auto item, selects)
        {
            int row = listWidget->row(item);
            lines.at(row)->setChecked(!lines.at(row)->isChecked());
        }
    })->disable(row == -1);

    auto selectCount = listWidget->selectedItems().size();
    menu->addAction(QIcon(":/icons/list_copy"), "复制列表 (&G)", [=]{
        auto selects = listWidget->selectedItems();
        QStringList sl;
        foreach (auto item, selects)
        {
            int row = listWidget->row(item);
            sl.append(lines.at(row)->getText());
        }
        QApplication::clipboard()->setText(sl.join("\n"));
    })->text(selectCount > 1, QString("复制列表 %1 (&G)").arg(selectCount))
            ->disable(selectCount == 0);

    QString text = QApplication::clipboard()->text();
    int pasteCount = text.split("\n", QString::SkipEmptyParts).size();
    menu->addAction(QIcon(":/icons/list_paste"), "粘贴列表 (&P)", [=]{
        QStringList sl = text.split("\n", QString::SkipEmptyParts);
        int index = listWidget->currentRow();
        if (index == -1)
            index = 0;
        listWidget->clearSelection();
        foreach (auto s, sl)
        {
            insertItem(index, false, s.trimmed());
            listWidget->setCurrentRow(index, QItemSelectionModel::Select);
            index++;
        }
    })->text(pasteCount > 1, QString("粘贴列表 %1 (&P)").arg(pasteCount))->disable(!pasteCount);

    menu->split()->addAction(QIcon(":/icons/add"), "插入 (&I)", [=]{
        int row = listWidget->currentRow();
        if (row == -1)
            row = 0;
        insertAndFocusItem(row);
    })->tip("Shift+Enter");

    menu->addAction(QIcon(":/icons/select_all"), "全选 (&A)", [=]{
        listWidget->selectAll();
    })->tip("Alt+A")->disable(!listWidget->count());

    menu->addAction(QIcon(":/icons/delete"), "删除行 (&D)", [=]{
        actionDelete();
    })->tip("Alt+Delete")->disable(row == -1);

    menu->exec();
    emit facileMenuUsed(menu);
}

void TodoItem::actionDelete()
{
    int selectRow = listWidget->currentRow();
    auto selects = listWidget->selectedItems();
    foreach (auto item, selects)
    {
        int row = listWidget->row(item);
        if (row < selectRow)
            selectRow--;
        if (row == -1)
            continue;
        deleteItem(row);
    }

    if (selectRow >= listWidget->count())
        selectRow = listWidget->count() - 1;
    if (selectRow >= 0 && selectRow < listWidget->count())
        listWidget->setCurrentRow(selectRow);
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
    // addButton->show();
}

void TodoItem::leaveEvent(QEvent *event)
{
    ResizeableItemBase::leaveEvent(event);

    addButton->hide();
}

void TodoItem::keyPressEvent(QKeyEvent *e)
{
    auto key = e->key();
    auto modifier = e->modifiers();
    // qDebug() << "item.key" << modifier << key;
    if (key == Qt::Key_Delete)
    {
        actionDelete();
        return ;
    }
    else if (!modifier && (key == Qt::Key_Enter || key == Qt::Key_Return))
    {
        int row = listWidget->currentRow();
        if (row != -1)
        {
            lines.at(row)->setEdit();
        }
        return ;
    }
    else if ((modifier & Qt::ControlModifier || modifier & Qt::AltModifier) && key == Qt::Key_A)
    {
        // 因为快捷键冲突了，只能这样子覆盖了
        listWidget->selectAll();
        return ;
    }
    else if (modifier & Qt::AltModifier && key == Qt::Key_Delete)
    {
        actionDelete();
        return ;
    }

    ResizeableItemBase::keyPressEvent(e);
}
