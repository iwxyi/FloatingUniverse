#include <QVBoxLayout>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QScrollBar>
#include "longtextitem.h"
#include "facilemenu.h"
#include "usettings.h"

LongTextItem::LongTextItem(QWidget *parent) : ResizeableItemBase(parent)
{
    setType(LongText);

    edit = new CustomEdit(this);
    edit->setObjectName("TextEdit");
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(edit);

    layout->setMargin(this->boundaryWidth);
    edit->setMinimumSize(64, 16);
    edit->setStyleSheet("QTextEdit { background: transparent; border: none; }");
    edit->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(edit, &QTextEdit::textChanged, this, [=]{
        emit modified();
    });
    connect(edit, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showEditMenu()));
    connect(edit, &CustomEdit::focusIn, this, [=]{
        emit selectMe();
    });
    connect(edit, &CustomEdit::focusOut, this, [=]{
        /// 如果是右键菜单，同样会触发 focusOut
        /// 然后会触发panel的leaveEvent
        emit useFinished();
    });
    connect(edit, &CustomEdit::finished, this, [=]{
        emit cancelEditMe();
    });
    connect(edit, &CustomEdit::rejectFocus, this, [=]{
        this->parentWidget()->setFocus();
    });
}

MyJson LongTextItem::toJson() const
{
    MyJson json = ResizeableItemBase::toJson();

    json.insert("enableHtml", enableHtml);
    json.insert("text", getText());
    json.insert("scrollPos", edit->verticalScrollBar()->sliderPosition());

    return json;
}

void LongTextItem::fromJson(const MyJson &json)
{
    ResizeableItemBase::fromJson(json);

    enableHtml = json.b("enableHtml", enableHtml);
    setText(json.s("text"), enableHtml);
    if (json.contains("scrollPos"))
    {
        QTimer::singleShot(0, [=]{
            edit->verticalScrollBar()->setSliderPosition(json.i("scrollPos"));
        });
    }
}

bool LongTextItem::isUsing() const
{
    if (edit->hasFocus())
        return true;
    return ResizeableItemBase::isUsing();
}

void LongTextItem::setText(const QString &text)
{
    edit->setPlainText(text);
    enableHtml = false;
}

void LongTextItem::setHtml(const QString &html)
{
    edit->setHtml(html);
    enableHtml = true;
}

void LongTextItem::setText(const QString &text, bool enHtml)
{
    if (enHtml)
        setHtml(text);
    else
        setText(text);
}

QString LongTextItem::getText() const
{
    return enableHtml ? edit->toHtml() : edit->toPlainText();
}

bool LongTextItem::isHtml() const
{
    return enableHtml;
}

void LongTextItem::adjustSizeByText(QSize maxxSize)
{
    auto maxSize = edit->maximumSize();

    // 获取合适的大小
    edit->document()->adjustSize();
    auto size = edit->document()->size().toSize();// + QSize(4, 4);

    bool shrink = false;
    if (size.width() > maxxSize.width())
    {
        size.setWidth(maxxSize.width());
        shrink = true;
    }
    if (size.height() > maxxSize.height())
    {
        size.setHeight(maxxSize.height());
        shrink = true;
    }
    // if (shrink) // 有缩起
    {
        // 需要加上滚动条的宽度
        size += QSize(edit->verticalScrollBar()->width(), 0);
    }

    // 调整大小
    edit->setFixedSize(size);
    this->adjustSize();
    edit->setMinimumSize(ITEM_MIN_SIZE);
    edit->setMaximumSize(maxSize);
}

void LongTextItem::editText()
{
    edit->setFocus();
}

void LongTextItem::showEditMenu()
{
    newFacileMenu;

    bool selection = edit->textCursor().hasSelection();

    menu->addAction(QIcon(":/icons/copy"), "复制 (&C)", [=]{
        if (selection)
        {
            edit->copy();
        }
        else
        {
            if (!enableHtml)
            {
                QApplication::clipboard()->setText(edit->toPlainText());
            }
            else
            {
                QMimeData *mime = new QMimeData();
                mime->setHtml(edit->toHtml());
                QApplication::clipboard()->setMimeData(mime);
            }
        }
    });

    menu->addAction(QIcon(":/icons/cut"), "剪切 (&X)", [=]{
        if (selection)
        {
            edit->cut();
        }
        else
        {
            if (!enableHtml)
            {
                QApplication::clipboard()->setText(edit->toPlainText());
            }
            else
            {
                QMimeData *mime = new QMimeData();
                mime->setHtml(edit->toHtml());
                QApplication::clipboard()->setMimeData(mime);
            }
            edit->clear();
        }
    });

    auto clip = QApplication::clipboard();
    auto mime = clip->mimeData();
    bool canPaste = mime->hasText()
            || (enableHtml && mime->hasHtml());
    menu->addAction(QIcon(":/icons/paste"), "粘贴纯文本 (&V)", [=]{
        /*if (enableHtml)
            edit->paste();
        else*/
            edit->insertPlainText(mime->text());
    })->disable(!canPaste)/*->text(!enableHtml, "粘贴纯文本 (&V)")*/;

    menu->split();
    menu->addAction(QIcon(":/icons/html"), "使用HTML (&H)", [=]{
        enableHtml = !enableHtml;
        emit modified();
    })->check(enableHtml);

    FacileMenu* insertMenu = menu->addMenu(QIcon(":/icons/add"), "插入");
    {
        auto insertHtml = [=](const QString& content) {
            if (!enableHtml)
                enableHtml = true;

            QTextCursor cursor = edit->textCursor();
            int pos = cursor.position();
            edit->insertHtml(content);
            int pos2 = cursor.position();
            cursor.setPosition(pos + 1);
            cursor.setPosition(pos2, QTextCursor::KeepAnchor);
            edit->setTextCursor(cursor);
        };
        insertMenu->addAction("插入无序列表", [=]{
            insertHtml("<ul style='margin: 0px; margin-left:-28px;'><li style='margin: 0px;'>UnorderedList</li></ul>");
        });
        insertMenu->addAction("插入有序列表", [=]{
            insertHtml("<ol style='margin: 0px; margin-left:-24px;'><li style='margin: 0px;'>OrderedList</li></ol>");
        });
    }

    menu->split()->addAction(QIcon(":/icons/delete"), "删除 (&D)", [=]{
        emit deleteMe();
    });

    menu->exec();
    emit facileMenuUsed(menu);
}

void LongTextItem::selectEvent(const QPoint &startPos)
{
    ResizeableItemBase::selectEvent(startPos);

    edit->raise();
}
void LongTextItem::saveMyModuleSize()
{
    saveModuleSize(us->moduleSize_Text, "moduleText");
}
