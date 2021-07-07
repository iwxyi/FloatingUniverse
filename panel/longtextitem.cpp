#include <QVBoxLayout>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include "longtextitem.h"
#include "facilemenu.h"

LongTextItem::LongTextItem(QWidget *parent) : ResizeableItemBase(parent)
{
    setType(LongText);

    edit = new CustomEdit(this);
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
        cancelEdit();
        emit unselectMe();
    });
}

MyJson LongTextItem::toJson() const
{
    MyJson json = ResizeableItemBase::toJson();

    json.insert("enableHtml", enableHtml);
    json.insert("text", getText());

    return json;
}

void LongTextItem::fromJson(const MyJson &json)
{
    ResizeableItemBase::fromJson(json);

    enableHtml = json.b("enableHtml", enableHtml);
    setText(json.s("text"), enableHtml);
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

void LongTextItem::adjustSizeByText()
{
    edit->adjustSize();
    edit->resize(edit->sizeHint());
    this->adjustSize();
}

void LongTextItem::editText()
{
    edit->setFocus();
}

void LongTextItem::cancelEdit()
{
    this->parentWidget()->setFocus();
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

    menu->addAction(QIcon(":/icons/paste"), "粘贴 (&V)", [=]{
        edit->paste();
    });

    menu->split();
    menu->addAction(QIcon(":/icons/html"), "使用HTML (&H)", [=]{
        enableHtml = !enableHtml;
        emit modified();
    })->check(enableHtml);

    menu->exec();
    emit facileMenuUsed(menu);
}

void LongTextItem::selectEvent()
{
    ResizeableItemBase::selectEvent();

    edit->raise();
}

void LongTextItem::unselectEvent()
{
    ResizeableItemBase::unselectEvent();

    if (edit->hasFocus())
        cancelEdit();
}
