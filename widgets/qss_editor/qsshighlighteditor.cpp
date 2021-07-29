#include "qsshighlighteditor.h"

QSSHighlightEditor::QSSHighlightEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    new QSSHighlighter(document());
}

void QSSHighlightEditor::keyPressEvent(QKeyEvent *e)
{
    QPlainTextEdit::keyPressEvent(e);

    if (e->modifiers() == Qt::NoModifier && (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter))
    {
        // 回车跟随上一行的缩进
        QString left = toPlainText().left(textCursor().position()-1);
        textCursor().insertText(QRegularExpression("\\s*").match(left.mid(left.lastIndexOf("\n")+1)).captured());
    }
}


QSSHighlighter::QSSHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
{

}

void QSSHighlighter::highlightBlock(const QString &text)
{
    static auto getTCF = [](QColor c) {
        QTextCharFormat f;
        f.setForeground(c);
        return f;
    };
    static QList<QSSRule> qss_rules = {
        // ID；
        QSSRule{QRegularExpression("^\\s*[#]\\w+"), getTCF(QColor(222, 49, 99))},
        // 键
        QSSRule{QRegularExpression("[-\\w]+(?=\\s*:)"), getTCF(QColor(204, 85, 0))},
        // 值
        QSSRule{QRegularExpression("(?<=:)\\s*[-#\\w\\d%]+"), getTCF(QColor(255, 127, 80))},
        // 注释
        QSSRule{QRegularExpression("/\\*.+?\\*/"), getTCF(QColor(80, 200, 120))},
    };

    foreach (auto rule, qss_rules)
    {
        auto iterator = rule.pattern.globalMatch(text);
        while (iterator.hasNext()) {
            auto match = iterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
