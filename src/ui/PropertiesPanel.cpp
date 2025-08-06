#include "PropertiesPanel.h"

PropertiesPanel::PropertiesPanel(Scene* scene, QWidget* parent) : QWidget(parent), scene(scene)
{
    vbox = new QVBoxLayout(this);
    formLayout = new QFormLayout();
    vbox->addLayout(formLayout);
}

void PropertiesPanel::clearPanel()
{
    while ((layItem = formLayout->takeAt(0))) {
        delete layItem->widget();
        delete layItem;
    }
}

void PropertiesPanel::onObjectSelected(QPointer<SceneObject> obj)
{
    clearPanel();
    currentObject = obj;
    if (!currentObject) return;

    for (auto* comp : currentObject->getAllComponents()) {
        comp->createInspector(this, formLayout);
    }

    addBtn = new QPushButton("Add Component", this);
    connect(addBtn, &QPushButton::clicked, this, &PropertiesPanel::onAddComponent);
    formLayout->addRow(addBtn);

    delBtn = new QPushButton("Delete Object", this);
    delBtn->setStyleSheet("background-color: #c0392b; color: white;");
    connect(delBtn, &QPushButton::clicked, [this]() {
        if (currentObject) {
            auto reply = QMessageBox::question(
                this,
                "Delete Object",
                "Are you sure you want to delete '" + QString::fromStdString(currentObject->getName()) + "'?",
                QMessageBox::Yes | QMessageBox::No
            );

            if (reply == QMessageBox::Yes) {
                scene->removeObject(currentObject);
                clearPanel();
            }
        }
        });
    formLayout->addRow(delBtn);
}

void PropertiesPanel::updateUI()
{
    // later bruh
}

void PropertiesPanel::onAddComponent()
{
    if (!currentObject) return;
    QStringList availableComponents;

    if (!currentObject->getComponent<MeshRenderer>())
        availableComponents << "MeshRenderer";
    if (!currentObject->getComponent<Light>())
        availableComponents << "Light";
    if (!currentObject->getComponent<RigidBodyComponent>())
        availableComponents << "RigidBody";
    if (!currentObject->getComponent<BoxColliderComponent>())
        availableComponents << "BoxCollider";
    if (!currentObject->getComponent<SphereColliderComponent>())
        availableComponents << "SphereCollider";

    if (availableComponents.isEmpty()) return;

    bool ok;
    selected = QInputDialog::getItem(this, "Add Component",
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
    else if (selected == "RigidBody") {
        currentObject->addComponent<RigidBodyComponent>();
    }
    else if (selected == "BoxCollider") {
        currentObject->addComponent<BoxColliderComponent>();
    }
    else if (selected == "SphereCollider") {
        currentObject->addComponent<SphereColliderComponent>();
    }

    onObjectSelected(currentObject);
}