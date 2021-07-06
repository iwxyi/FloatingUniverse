#include <QVBoxLayout>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include "longtextitem.h"
#include "facilemenu.h"

LongTextItem::LongTextItem(QWidget *parent) : ResizeableItemBase(parent)
{
    setType(LongText);

    edit = new QTextEdit(this);
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

    int w = json.i("width", this->width());
    int h = json.i("height", this->height());
    resize(w, h);

    enableHtml = json.b("enableHtml", enableHtml);
    setText(json.s("text"), enableHtml);
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
    this->adjustSize();
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

void LongTextItem::showEdgeEvent()
{
    ResizeableItemBase::showEdgeEvent();

    edit->raise();
}
