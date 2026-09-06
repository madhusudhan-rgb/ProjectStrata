#include "explorer.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QTreeView>
#include <QFileSystemModel>
#include <QFileInfo>
#include <QDir>

Explorer::Explorer(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QLabel *header = new QLabel("EXPLORER", this);

    header->setObjectName("panelHeader");

    layout->addWidget(header);

    tree = new QTreeView(this);

    model = new QFileSystemModel(this);

    model->setFilter(
        QDir::AllEntries |
        QDir::Hidden |
        QDir::NoDotAndDotDot
        );

    QString startPath = QDir::currentPath();

    QFileInfo startInfo(startPath);

    if (!startInfo.isDir() ||
        startPath.startsWith("C:/Windows", Qt::CaseInsensitive))
    {
        startPath = QDir::homePath();
    }

    tree->setModel(model);

    // setRootPath() returns the index; model->index() would be
    // invalid until the model loads the directory asynchronously
    tree->setRootIndex(model->setRootPath(startPath));

    connect(
        model,
        &QFileSystemModel::directoryLoaded,
        this,
        [this](const QString &path) {
            if (model->rootPath() == path)
            {
                tree->setRootIndex(model->index(path));
            }
        }
        );

    connect(
        tree,
        &QTreeView::doubleClicked,
        this,
        [this](const QModelIndex &index) {
            QString path = model->filePath(index);

            if (QFileInfo(path).isFile())
            {
                emit fileActivated(path);
            }
        }
        );

    layout->addWidget(tree);

    setLayout(layout);
}

void Explorer::setRootPath(const QString &path)
{
    tree->setRootIndex(model->setRootPath(path));
}

QString Explorer::rootPath() const
{
    return model->rootPath();
}