#include "PropertiesPanel.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QFormLayout>
#include <QInputDialog>
#include <Light.h>

PropertiesPanel::PropertiesPanel(Scene* scene, QWidget* parent)
    : QWidget(parent), scene(scene) {
    auto* vbox = new QVBoxLayout(this);
    formLayout = new QFormLayout();
    vbox->addLayout(formLayout);
}

void PropertiesPanel::clearPanel() {
    QLayoutItem* item;
    while ((item = formLayout->takeAt(0))) {
        delete item->widget();
        delete item;
    }
}

void PropertiesPanel::onObjectSelected(QPointer<SceneObject> obj) {
    clearPanel();
    currentObject = obj;
    if (!currentObject) return;

    // UI for each component
    for (auto* comp : currentObject->getAllComponents()) {
        comp->createInspector(this, formLayout);
    }

    // Add Component Button
    auto* addBtn = new QPushButton("Add Component", this);
    connect(addBtn, &QPushButton::clicked, this, &PropertiesPanel::onAddComponent);
    formLayout->addRow(addBtn);

    // Delete Object Button
    auto* delBtn = new QPushButton("Delete Object", this);
    connect(delBtn, &QPushButton::clicked, [this]() { scene->removeObject(currentObject); });
    formLayout->addRow(delBtn);
}

void PropertiesPanel::updateUI() {
    // update from bound properties
    // finalize if necessary
}

void PropertiesPanel::onAddComponent() {
    if (!currentObject) return;
    QStringList availableComponents;

    if (!currentObject->getComponent<MeshRenderer>())
        availableComponents << "MeshRenderer";
    if (!currentObject->getComponent<Light>())
        availableComponents << "Light";

    if (availableComponents.isEmpty()) return;

    bool ok;
    QString selected = QInputDialog::getItem(this, "Add Component",
        "Select component to add:",
        availableComponents,
        0, false, &ok);

    if (!ok || selected.isEmpty()) return;

    if (selected == "MeshRenderer") {
        currentObject->addComponent<MeshRenderer>();
    }
    else if (selected == "Light") {
        currentObject->addComponent<Light>();
    }

    onObjectSelected(currentObject);
}