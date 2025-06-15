#pragma once
#include "Scene.h"
#include <QWidget>
#include <QTreeWidget>
#include <QEvent>
#include <QMouseEvent>
#include <QPointer>

class Scene;

class SceneHierarchyPanel : public QWidget {
    Q_OBJECT
public:
    explicit SceneHierarchyPanel(Scene* scene, QWidget* parent = nullptr);
    void updateHierarchy();

protected:
    bool eventFilter(QObject* watched, QEvent* ev) override;

signals:
    void objectSelected(SceneObject* object);

private slots:
    void onItemSelectionChanged();

private:
    Scene* scene;
    QTreeWidget* treeWidget;
    QMap<QTreeWidgetItem*, QPointer<SceneObject>> itemObjectMap;
};