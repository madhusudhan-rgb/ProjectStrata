#include "codeeditor.h"

#include <QPainter>
#include <QTextBlock>


CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
{
    QFont monoFont("Consolas", 11);

    monoFont.setFixedPitch(true);

    setFont(monoFont);

    setTabStopDistance(
        4 * QFontMetricsF(monoFont).horizontalAdvance(' ')
        );

    setWordWrapMode(QTextOption::NoWrap);

    lineNumberArea = new LineNumberArea(this);

    connect(
        this,
        &QPlainTextEdit::blockCountChanged,
        this,
        &CodeEditor::updateLineNumberAreaWidth
        );

    connect(
        this,
        &QPlainTextEdit::updateRequest,
        this,
        &CodeEditor::updateLineNumberArea
        );

    connect(
        this,
        &QPlainTextEdit::cursorPositionChanged,
        this,
        &CodeEditor::highlightCurrentLine
        );

    updateLineNumberAreaWidth();
    highlightCurrentLine();
}


int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;

    int max = qMax(1, blockCount());

    while (max >= 10)
    {
        max /= 10;
        ++digits;
    }

    int space =
        12 + fontMetrics().horizontalAdvance('9') * digits;

    return space;
}


void CodeEditor::updateLineNumberAreaWidth()
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}


void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
    {
        lineNumberArea->scroll(0, dy);
    }
    else
    {
        lineNumberArea->update(
            0,
            rect.y(),
            lineNumberArea->width(),
            rect.height()
            );
    }

    if (rect.contains(viewport()->rect()))
    {
        updateLineNumberAreaWidth();
    }
}


void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();

    lineNumberArea->setGeometry(
        cr.left(),
        cr.top(),
        lineNumberAreaWidth(),
        cr.height()
        );
}


void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly())
    {
        QTextEdit::ExtraSelection selection;

        selection.format.setBackground(QColor(0x2a, 0x2d, 0x2e));
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);

        selection.cursor = textCursor();
        selection.cursor.clearSelection();

        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}


void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);

    painter.fillRect(event->rect(), QColor(0x1a, 0x1a, 0x1a));

    QTextBlock block = firstVisibleBlock();

    int blockNumber = block.blockNumber();

    int top =
        qRound(
            blockBoundingGeometry(block)
                .translated(contentOffset())
                .top()
            );

    int bottom =
        top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            QString number =
                QString::number(blockNumber + 1);

            bool isCurrentLine =
                blockNumber == textCursor().blockNumber();

            painter.setPen(
                isCurrentLine
                    ? QColor(0xd4, 0xd4, 0xd4)
                    : QColor(0x6a, 0x6a, 0x6a)
                );

            painter.drawText(
                0,
                top,
                lineNumberArea->width() - 8,
                fontMetrics().height(),
                Qt::AlignRight,
                number
                );
        }

        block = block.next();

        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());

        ++blockNumber;
    }
}