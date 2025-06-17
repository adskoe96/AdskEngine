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

    treeWidget->viewport()->installEventFilter(this);

    // selection change
    connect(treeWidget, &QTreeWidget::itemSelectionChanged,
        this, &SceneHierarchyPanel::onItemSelectionChanged);
    // update on add/remove
    connect(scene, &Scene::objectAdded, this, &SceneHierarchyPanel::updateHierarchy);
    connect(scene, &Scene::objectRemoved, this, &SceneHierarchyPanel::updateHierarchy);
    connect(treeWidget, &QTreeWidget::itemChanged, this, &SceneHierarchyPanel::onItemEdited);
}

void SceneHierarchyPanel::updateHierarchy() {
    SceneObject* oldObj = nullptr;
    if (auto* oldItem = treeWidget->currentItem()) {
        auto ptr = itemObjectMap.value(oldItem);
        oldObj = ptr.data();
    }

    for (auto it = itemObjectMap.keyBegin(); it != itemObjectMap.keyEnd(); ++it) {
        if (auto obj = itemObjectMap.value(*it)) {
            disconnect(obj, &SceneObject::nameChanged,
                this, &SceneHierarchyPanel::onObjectNameChanged);
        }
    }

    treeWidget->clear();
    itemObjectMap.clear();

    treeWidget->blockSignals(true);
    for (const auto& objPtr : scene->getObjects()) {
        auto* item = new QTreeWidgetItem(treeWidget);
        item->setText(0, QString::fromStdString(objPtr->getName()));
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        itemObjectMap[item] = QPointer<SceneObject>(objPtr.get());

        connect(objPtr.get(), &SceneObject::nameChanged,
            this, &SceneHierarchyPanel::onObjectNameChanged);
    }
    treeWidget->blockSignals(false);

    if (oldObj) {
        for (auto it = itemObjectMap.begin(); it != itemObjectMap.end(); ++it) {
            if (it.value().data() == oldObj) {
                treeWidget->setCurrentItem(it.key());
                break;
            }
        }
    }
}

bool SceneHierarchyPanel::eventFilter(QObject* watched, QEvent* ev)
{
    if (watched == treeWidget->viewport() && ev->type() == QEvent::MouseButtonPress) {
        auto* me = static_cast<QMouseEvent*>(ev);
        if (!treeWidget->itemAt(me->pos())) {
            treeWidget->clearSelection();
            return true;
        }
    }
    return QWidget::eventFilter(watched, ev);
}

void SceneHierarchyPanel::onItemSelectionChanged() {
    auto selectedItems = treeWidget->selectedItems();
    if (!selectedItems.isEmpty()) {
        auto* selectedItem = selectedItems.first();
        auto objPtr = itemObjectMap.value(selectedItem); // QPointer<SceneObject>
        if (objPtr) {
            emit objectSelected(objPtr.data()); // We're only transferring a live object
        }
        else {
            emit objectSelected(nullptr); // If the object is deleted, pass nullptr
        }
    }
    else {
        emit objectSelected(nullptr);
    }
}

void SceneHierarchyPanel::onItemEdited(QTreeWidgetItem* item, int column) {
    if (column != 0) return;

    if (auto obj = itemObjectMap.value(item)) {
        QString newName = item->text(0);

        treeWidget->blockSignals(true);
        obj->setName(newName.toStdString());
        treeWidget->blockSignals(false);
    }
}

void SceneHierarchyPanel::onObjectNameChanged(const std::string& newName)
{
    SceneObject* obj = qobject_cast<SceneObject*>(sender());
    if (!obj) return;

    for (auto it = itemObjectMap.begin(); it != itemObjectMap.end(); ++it) {
        if (it.value() == obj) {
            treeWidget->blockSignals(true);
            it.key()->setText(0, QString::fromStdString(newName));
            treeWidget->blockSignals(false);
            break;
        }
    }
}
