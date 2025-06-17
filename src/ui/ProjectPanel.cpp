#include "ProjectPanel.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QTreeView>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDebug>

class JsonFilterProxyModel : public QSortFilterProxyModel {
public:
    explicit JsonFilterProxyModel(QObject* parent = nullptr) : QSortFilterProxyModel(parent) {}

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override {
        QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
        QString name = sourceModel()->data(index, Qt::DisplayRole).toString();
        return !name.endsWith(".json", Qt::CaseInsensitive);
    }
};

ProjectPanel::ProjectPanel(const QString& projectRoot, QWidget* parent)
    : QWidget(parent), m_projectPath(projectRoot)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);

    auto* label = new QLabel("Project Files");
    label->setStyleSheet("font-weight: bold; color: white;");
    layout->addWidget(label);

    model = new QFileSystemModel(this);
    model->setRootPath(projectRoot);
    model->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);

    proxyModel = new JsonFilterProxyModel(this);
    proxyModel->setSourceModel(model);

    tree = new QTreeView(this);
    tree->setModel(proxyModel);
    tree->setRootIndex(proxyModel->mapFromSource(model->index(projectRoot)));
    tree->setHeaderHidden(true);
    tree->setEditTriggers(QAbstractItemView::NoEditTriggers);

    layout->addWidget(tree);

    setAcceptDrops(true);
    setStyleSheet("background-color: #252526; color: white;");
}

void ProjectPanel::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void ProjectPanel::dropEvent(QDropEvent* event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    for (const QUrl& url : urls) {
        QFileInfo fileInfo(url.toLocalFile());
        if (fileInfo.exists() && fileInfo.isFile()) {
            QString dest = m_projectPath + "/" + fileInfo.fileName();
            QFile::copy(fileInfo.absoluteFilePath(), dest);
            qDebug() << "Copied:" << fileInfo.absoluteFilePath() << "->" << dest;
        }
    }
}
