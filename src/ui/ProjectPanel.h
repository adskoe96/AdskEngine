#pragma once
#include <QWidget>
#include <QString>

class QTreeView;
class QFileSystemModel;
class QSortFilterProxyModel;

class ProjectPanel : public QWidget {
    Q_OBJECT

public:
    explicit ProjectPanel(const QString& projectRoot, QWidget* parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    QString m_projectPath;
    QFileSystemModel* model;
    QSortFilterProxyModel* proxyModel;
    QTreeView* tree;
};
