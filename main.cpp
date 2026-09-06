#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QDebug>
#include <QMessageBox>
#include <QSplitter>
#include <QPlainTextEdit>

#include "terminal.h"


// File → New
void newFile(QWidget *parent)
{
    QMessageBox::information(
        parent,
        "strata",
        "coming soon folk"
        );
}


// File → Open
void openFile()
{
    qDebug() << "Open file";
}


// File → Save
void saveFile()
{
    qDebug() << "Save file";
}


// Help → About
void about(QWidget *parent)
{
    QMessageBox::information(
        parent,
        "About Strata",
        ""
        );
}


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QMainWindow window;

    window.setWindowTitle("Strata");
    window.resize(1000, 800);


    // Main splitter
    QSplitter *splitter =
        new QSplitter(Qt::Vertical);


    // Editor
    QPlainTextEdit *editor =
        new QPlainTextEdit();

    editor->setPlaceholderText(
        "Strata editor"
        );


    // Terminal
    Terminal *terminal =
        new Terminal();


    // Add widgets
    splitter->addWidget(editor);
    splitter->addWidget(terminal);


    // Initial sizes
    splitter->setSizes({
        600,
        200
    });


    // Resize handle
    splitter->setHandleWidth(5);


    // Put splitter in window
    window.setCentralWidget(splitter);


    // Menu bar
    QMenuBar *menuBar =
        window.menuBar();


    // File
    QMenu *fileMenu =
        menuBar->addMenu("File");


    QAction *newAction =
        fileMenu->addAction("New");

    QObject::connect(
        newAction,
        &QAction::triggered,
        [&window]() {
            newFile(&window);
        }
        );


    QAction *openAction =
        fileMenu->addAction("Open");

    QObject::connect(
        openAction,
        &QAction::triggered,
        openFile
        );


    QAction *saveAction =
        fileMenu->addAction("Save");

    QObject::connect(
        saveAction,
        &QAction::triggered,
        saveFile
        );


    fileMenu->addSeparator();


    QAction *exitAction =
        fileMenu->addAction("Exit");

    QObject::connect(
        exitAction,
        &QAction::triggered,
        &app,
        &QApplication::quit
        );


    // Edit
    QMenu *editMenu =
        menuBar->addMenu("Edit");

    editMenu->addAction("Undo");
    editMenu->addAction("Redo");


    // View
    QMenu *viewMenu =
        menuBar->addMenu("View");

    viewMenu->addAction("Explorer");

    QAction *terminalAction =
        viewMenu->addAction("Terminal");

    QObject::connect(
        terminalAction,
        &QAction::triggered,
        [terminal]() {
            terminal->setVisible(
                !terminal->isVisible()
                );
        }
        );


    // Help
    QMenu *helpMenu =
        menuBar->addMenu("Help");

    QAction *aboutAction =
        helpMenu->addAction("About");

    QObject::connect(
        aboutAction,
        &QAction::triggered,
        [&window]() {
            about(&window);
        }
        );


    // QSS for styling
    window.setStyleSheet(
        "QMainWindow {"
        "    background-color: #1e1e1e;"
        "}"

        "QMenuBar {"
        "    background-color: #181818;"
        "    color: #cccccc;"
        "    padding: 4px;"
        "}"

        "QMenuBar::item {"
        "    padding: 5px 10px;"
        "    background: transparent;"
        "}"

        "QMenuBar::item:selected {"
        "    background-color: #2a2d2e;"
        "}"

        "QMenu {"
        "    background-color: #252526;"
        "    color: #cccccc;"
        "    border: 1px solid #3c3c3c;"
        "}"

        "QMenu::item {"
        "    padding: 7px 30px 7px 12px;"
        "}"

        "QMenu::item:selected {"
        "    background-color: #094771;"
        "}"

        "QPlainTextEdit {"
        "    background-color: #0d0d0d;"
        "    color: #d4d4d4;"
        "    border: none;"
        "}"

        "QSplitter::handle {"
        "    background-color: #252525;"
        "}"

        "QSplitter::handle:hover {"
        "    background-color: #3f3f3f;"
        "}"
        );


    window.show();

    return app.exec();
}