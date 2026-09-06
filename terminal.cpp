#include "terminal.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QTextBlock>
#include <QTextCursor>

Terminal::Terminal(QWidget *parent)
    : QPlainTextEdit(parent)
{
    setStyleSheet(
        "QPlainTextEdit {"
        "    background-color: #0d0d0d;"
        "    color: #d4d4d4;"
        "    border: none;"
        "    padding: 8px;"
        "    font-family: Consolas;"
        "    font-size: 14px;"
        "}"
        );

    setUndoRedoEnabled(false);

    process = new QProcess(this);

    connect(
        process,
        &QProcess::readyReadStandardOutput,
        this,
        [this]() {
            QString output =
                QString::fromLocal8Bit(
                    process->readAllStandardOutput()
                    );

            appendPlainText(output);

            promptPosition =
                document()->characterCount() - 1;

            moveCursor(QTextCursor::End);
        }
        );

    connect(
        process,
        &QProcess::readyReadStandardError,
        this,
        [this]() {
            QString output =
                QString::fromLocal8Bit(
                    process->readAllStandardError()
                    );

            appendPlainText(output);

            promptPosition =
                document()->characterCount() - 1;

            moveCursor(QTextCursor::End);
        }
        );

    process->start(
        "powershell.exe",
        {
            "-NoLogo",
            "-NoProfile"
        }
        );
}


void Terminal::keyPressEvent(QKeyEvent *event)
{
    QTextCursor cursor = textCursor();

    int position = cursor.position();

    // Don't allow editing before the prompt
    if (position < promptPosition)
    {
        cursor.setPosition(promptPosition);
        setTextCursor(cursor);
    }

    // Enter
    if (event->key() == Qt::Key_Return ||
        event->key() == Qt::Key_Enter)
    {
        sendCommand();
        return;
    }

    // Backspace
    if (event->key() == Qt::Key_Backspace)
    {
        if (cursor.position() <= promptPosition)
        {
            return;
        }
    }

    // Left arrow
    if (event->key() == Qt::Key_Left)
    {
        if (cursor.position() <= promptPosition)
        {
            return;
        }
    }

    // Home
    if (event->key() == Qt::Key_Home)
    {
        cursor.setPosition(promptPosition);
        setTextCursor(cursor);
        return;
    }

    QPlainTextEdit::keyPressEvent(event);
}


void Terminal::mousePressEvent(QMouseEvent *event)
{
    QPlainTextEdit::mousePressEvent(event);

    QTextCursor cursor = textCursor();

    if (cursor.position() < promptPosition)
    {
        cursor.setPosition(promptPosition);
        setTextCursor(cursor);
    }
}


void Terminal::sendCommand()
{
    QTextCursor cursor = textCursor();

    cursor.setPosition(promptPosition);

    cursor.movePosition(
        QTextCursor::End,
        QTextCursor::KeepAnchor
        );

    QString command = cursor.selectedText();

    if (command.isEmpty())
    {
        process->write("\n");
        return;
    }

    // Move to the end of the command
    moveCursor(QTextCursor::End);

    // Send command to PowerShell
    process->write(
        command.toLocal8Bit()
        );

    process->write("\n");

    // Move to a new line
    insertPlainText("\n");

    promptPosition =
        document()->characterCount() - 1;
}