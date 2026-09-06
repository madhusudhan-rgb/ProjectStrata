#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QSplitter>
#include <QLabel>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QStringDecoder>
#include <QStatusBar>
#include <QDir>
#include <utility>

#include "terminal.h"
#include "explorer.h"
#include "codeeditor.h"
#include "highlighter.h"

QString currentFilePath;
bool documentModified = false;
QString currentEncoding = "UTF-8";

void updateWindowTitle(QMainWindow *window)
{
    if (currentFilePath.isEmpty())
    {
        window->setWindowTitle("Strata");
        return;
    }

    QFileInfo info(currentFilePath);

    window->setWindowTitle(
        info.fileName() +
        (documentModified ? " *" : "") +
        " - Strata"
        );
}

void updateStatusBar(QMainWindow *window)
{
    if (currentFilePath.isEmpty())
    {
        window->statusBar()->showMessage("No file open");
        return;
    }

    window->statusBar()->showMessage(
        QDir::toNativeSeparators(currentFilePath)
        );
}

// Decodes UTF-8, UTF-8 BOM, UTF-16 LE/BE or ANSI.
// Sets *encodingName to "" when the file cannot be read.
QString readTextFile(const QString &path, QString *encodingName)
{
    *encodingName = "";

    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
    {
        return QString();
    }

    QByteArray data = file.readAll();

    if (data.startsWith("\xEF\xBB\xBF"))
    {
        *encodingName = "UTF-8 BOM";
        return QString::fromUtf8(data.mid(3));
    }

    if (data.startsWith("\xFF\xFE") || data.startsWith("\xFE\xFF"))
    {
        *encodingName = "UTF-16";

        QByteArray payload = data.mid(2);

        if (data.at(0) == '\xFE')
        {
            for (int i = 0; i + 1 < payload.size(); i += 2)
            {
                std::swap(payload[i], payload[i + 1]);
            }
        }

        return QString::fromUtf16(
            reinterpret_cast<const char16_t *>(payload.constData()),
            payload.size() / 2
            );
    }

    QStringDecoder decoder(QStringConverter::Utf8);

    QString text = decoder.decode(data);

    if (!decoder.hasError())
    {
        *encodingName = "UTF-8";
        return text;
    }

    *encodingName = "ANSI";
    return QString::fromLocal8Bit(data);
}

void about(QWidget *parent)
{
    QMessageBox::information(
        parent,
        "About Strata",
        "Strata is a native C++ development environment."
        );
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setApplicationName("Strata");

    QMainWindow window;

    window.setWindowTitle("Strata");
    window.resize(1200, 800);

    Explorer *explorer = new Explorer();

    CodeEditor *editor = new CodeEditor();

    editor->setPlaceholderText(
        "Select a file in the explorer to start editing"
        );

    QWidget *editorPanel = new QWidget();

    QVBoxLayout *editorLayout = new QVBoxLayout(editorPanel);

    editorLayout->setContentsMargins(0, 0, 0, 0);
    editorLayout->setSpacing(0);

    QLabel *editorHeader = new QLabel("untitled");

    editorHeader->setObjectName("panelHeader");

    editorLayout->addWidget(editorHeader);
    editorLayout->addWidget(editor);

    Terminal *terminal = new Terminal();

    QWidget *terminalPanel = new QWidget();

    QVBoxLayout *terminalLayout = new QVBoxLayout(terminalPanel);

    terminalLayout->setContentsMargins(0, 0, 0, 0);
    terminalLayout->setSpacing(0);

    QLabel *terminalHeader = new QLabel("TERMINAL");

    terminalHeader->setObjectName("panelHeader");

    terminalLayout->addWidget(terminalHeader);
    terminalLayout->addWidget(terminal);

    QSplitter *topSplitter = new QSplitter(Qt::Horizontal);

    topSplitter->addWidget(explorer);
    topSplitter->addWidget(editorPanel);
    topSplitter->setSizes({250, 950});
    topSplitter->setHandleWidth(5);

    QSplitter *mainSplitter = new QSplitter(Qt::Vertical);

    mainSplitter->addWidget(topSplitter);
    mainSplitter->addWidget(terminalPanel);
    mainSplitter->setSizes({600, 200});
    mainSplitter->setHandleWidth(5);

    window.setCentralWidget(mainSplitter);

    Highlighter *highlighter = new Highlighter(editor->document());

    QLabel *positionLabel = new QLabel("Ln 1, Col 1");
    QLabel *languageLabel = new QLabel("Plain Text");
    QLabel *encodingLabel = new QLabel("UTF-8");

    window.statusBar()->addPermanentWidget(positionLabel);
    window.statusBar()->addPermanentWidget(languageLabel);
    window.statusBar()->addPermanentWidget(encodingLabel);