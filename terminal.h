#pragma once

#include <QPlainTextEdit>
#include <QProcess>

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

    void sendCommand();
};