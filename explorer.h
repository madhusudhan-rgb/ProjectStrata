#pragma once

#include <QWidget>

class QTreeView;
class QFileSystemModel;

class Explorer : public QWidget
{
    Q_OBJECT

public:
    explicit Explorer(QWidget *parent = nullptr);

    void setRootPath(const QString &path);
    QString rootPath() const;

signals:
    void fileActivated(const QString &filePath);

private:
    QTreeView *tree;
    QFileSystemModel *model;
};