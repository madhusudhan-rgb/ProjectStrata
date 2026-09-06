#include "terminal.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QFont>
#include <QTextCursor>

Terminal::Terminal(QWidget *parent)
    : QPlainTextEdit(parent)
{
    QFont monoFont("Consolas", 11);

    monoFont.setFixedPitch(true);
    setFont(monoFont);

    setStyleSheet(
        "QPlainTextEdit {"
        "    background-color: #0d0d0d;"
        "    color: #d4d4d4;"
        "    border: none;"
        "    padding: 8px;"
        "    selection-background-color: #264f78;"
        "    selection-color: #ffffff;"
        "}"
        );

    setUndoRedoEnabled(false);

    // Read-only until the shell prints its first prompt
    setReadOnly(true);

    process = new QProcess(this);

    connect(
        process,
        &QProcess::readyReadStandardOutput,
        this,
        [this]() {
            appendOutput(
                QString::fromLocal8Bit(
                    process->readAllStandardOutput()
                    )
                );

            setReadOnly(false);
        }
        );

    connect(
        process,
        &QProcess::readyReadStandardError,
        this,
        [this]() {
            appendOutput(
                QString::fromLocal8Bit(
                    process->readAllStandardError()
                    )
                );

            setReadOnly(false);
        }
        );

    // Queued so the widget is never touched during shutdown
    connect(
        process,
        &QProcess::finished,
        this,
        [this](int exitCode, QProcess::ExitStatus) {
            appendPlainText(
                QString("\n[Process exited with code %1]")
                    .arg(exitCode)
                );

            setReadOnly(true);
        },
        Qt::QueuedConnection
        );

    process->start(
        "powershell.exe",
        {
            "-NoLogo",
            "-NoProfile"
        }
        );
}

void Terminal::appendOutput(const QString &output)
{
    appendPlainText(output);

    promptPosition = document()->characterCount() - 1;

    moveCursor(QTextCursor::End);
}

void Terminal::keyPressEvent(QKeyEvent *event)
{
    QTextCursor cursor = textCursor();

    // Don't allow editing before the prompt
    if (cursor.position() < promptPosition ||
        cursor.anchor() < promptPosition)
    {
        cursor.movePosition(QTextCursor::End);
        setTextCursor(cursor);

        cursor = textCursor();
    }

    int key = event->key();

    if (key == Qt::Key_Return || key == Qt::Key_Enter)
    {
        sendCommand();
        return;
    }

    if (key == Qt::Key_C &&
        (event->modifiers() & Qt::ControlModifier))
    {
        if (textCursor().hasSelection())
        {
            QPlainTextEdit::keyPressEvent(event);
            return;
        }

        cursor.setPosition(promptPosition);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        setTextCursor(cursor);

        return;
    }

    if (key == Qt::Key_Up)
    {
        showHistory(-1);
        return;
    }

    if (key == Qt::Key_Down)
    {
        showHistory(1);
        return;
    }

    if (key == Qt::Key_Backspace &&
        cursor.position() <= promptPosition)
    {
        return;
    }

    if (key == Qt::Key_Left &&
        cursor.position() <= promptPosition)
    {
        return;
    }

    if (key == Qt::Key_Home)
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

void Terminal::showHistory(int direction)
{
    if (history.isEmpty())
    {
        return;
    }

    int index = historyIndex + direction;

    if (index < 0 || index > history.size())
    {
        return;
    }

    historyIndex = index;

    QString command;

    if (index < history.size())
    {
        command = history.at(index);
    }

    QTextCursor cursor = textCursor();

    cursor.setPosition(promptPosition);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.insertText(command);

    setTextCursor(cursor);
}

void Terminal::sendCommand()
{
    if (process->state() != QProcess::Running)
    {
        return;
    }

    QTextCursor cursor = textCursor();

    cursor.setPosition(promptPosition);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

    QString command = cursor.selectedText();

    moveCursor(QTextCursor::End);

    QString trimmed = command.trimmed();

    if (!trimmed.isEmpty() &&
        (history.isEmpty() || history.last() != trimmed))
    {
        history.append(trimmed);
    }

    historyIndex = history.size();

    process->write(command.toLocal8Bit());
    process->write("\r\n");

    insertPlainText("\n");

    promptPosition = document()->characterCount() - 1;
}