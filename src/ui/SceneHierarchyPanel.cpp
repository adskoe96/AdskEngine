#include "SceneHierarchyPanel.h"
#include "Scene.h"
#include "SceneObject.h"
#include <QVBoxLayout>
#include <QLabel>

SceneHierarchyPanel::SceneHierarchyPanel(Scene* scene, QWidget* parent)
    : QWidget(parent), scene(scene)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);

    auto* label = new QLabel("Scene Hierarchy");
    label->setStyleSheet("font-weight: bold; color: white;");
    layout->addWidget(label);

    treeWidget = new QTreeWidget();
    treeWidget->setHeaderHidden(true);
    layout->addWidget(treeWidget);

    setStyleSheet("background-color: #2d2d30; color: white;");
    updateHierarchy();

    // selection change
    connect(treeWidget, &QTreeWidget::itemSelectionChanged,
        this, &SceneHierarchyPanel::onItemSelectionChanged);
    // update on add/remove
    connect(scene, &Scene::objectAdded, this, &SceneHierarchyPanel::updateHierarchy);
    connect(scene, &Scene::objectRemoved, this, &SceneHierarchyPanel::updateHierarchy);
}

void SceneHierarchyPanel::updateHierarchy() {
    // remember old selection
    SceneObject* oldObj = nullptr;
    if (auto* oldItem = treeWidget->currentItem()) {
        oldObj = itemObjectMap.value(oldItem, nullptr);
    }

    // rebuild tree
    treeWidget->clear();
    itemObjectMap.clear();
    for (const auto& objPtr : scene->getObjects()) {
        auto* it = new QTreeWidgetItem(treeWidget);
        it->setText(0, QString::fromStdString(objPtr->getName()));
        itemObjectMap[it] = objPtr.get();
    }

    // restore selection if still present
    if (oldObj) {
        for (auto it = itemObjectMap.begin(); it != itemObjectMap.end(); ++it) {
            if (it.value() == oldObj) {
                treeWidget->setCurrentItem(it.key());
                break;
            }
        }
    }
}

void SceneHierarchyPanel::onItemSelectionChanged() {
    auto selectedItems = treeWidget->selectedItems();
    if (!selectedItems.isEmpty()) {
        auto* selectedItem = selectedItems.first();
        auto obj = itemObjectMap.value(selectedItem, nullptr);
        emit objectSelected(obj);
    }
    else {
        emit objectSelected(nullptr);
    }
}