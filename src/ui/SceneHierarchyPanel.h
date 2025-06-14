#pragma once
#include "Scene.h"
#include <QWidget>
#include <QTreeWidget>

class Scene;

class SceneHierarchyPanel : public QWidget {
    Q_OBJECT
public:
    explicit SceneHierarchyPanel(Scene* scene, QWidget* parent = nullptr);
    void updateHierarchy();

signals:
    void objectSelected(SceneObject* object);

private slots:
    void onItemSelectionChanged();

private:
    Scene* scene;
    QTreeWidget* treeWidget;
    QMap<QTreeWidgetItem*, SceneObject*> itemObjectMap;
};