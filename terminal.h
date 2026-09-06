#pragma once

#include <QPlainTextEdit>
#include <QProcess>
#include <QStringList>

class Terminal : public QPlainTextEdit
{
public:
    explicit Terminal(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QProcess *process;
    int promptPosition = 0;

    QStringList history;
    int historyIndex = 0;

    void appendOutput(const QString &output);
    void showHistory(int direction);
    void sendCommand();
};