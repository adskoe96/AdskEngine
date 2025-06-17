#include "Toolbar.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include "Scene.h"
#include "Light.h"
#include <QFileDialog>
#include <MeshRenderer.h>

Toolbar::Toolbar(Scene* scene, QWidget* parent)
    : QWidget(parent), scene(scene)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);

    // Save the scene pointer in a local variable for explicit capture
    Scene* scenePtr = this->scene;

    // Object creation button with drop-down menu
    QToolButton* createButton = new QToolButton();
    createButton->setText("Create");
    createButton->setPopupMode(QToolButton::InstantPopup);

    QMenu* createMenu = new QMenu(createButton);

    QAction* createLightAction = new QAction("Light", this);
    connect(createLightAction, &QAction::triggered, [scenePtr]() {
        auto obj = std::make_unique<SceneObject>("Light");
        auto* light = obj->addComponent<Light>();
        scenePtr->addObject(std::move(obj));
    });
    createMenu->addAction(createLightAction);

    QAction* createMeshAction = new QAction("Mesh", this);
    connect(createMeshAction, &QAction::triggered, [scenePtr]() {
        auto obj = std::make_unique<SceneObject>("Mesh");
        obj->addComponent<MeshRenderer>();
        scenePtr->addObject(std::move(obj));
    });
    createMenu->addAction(createMeshAction);

    QAction* createEmptyAction = new QAction("Empty", this);
    connect(createEmptyAction, &QAction::triggered, [scenePtr]() {
        auto obj = std::make_unique<SceneObject>("Empty");
        scenePtr->addObject(std::move(obj));
    });
    createMenu->addAction(createEmptyAction);

    createButton->setMenu(createMenu);
    layout->addWidget(createButton);

    // Environment settings button
    QPushButton* envSettingsButton = new QPushButton("Environment Settings");
    layout->addWidget(envSettingsButton);
    connect(envSettingsButton, &QPushButton::clicked, [this]() {
        emit environmentSettingsRequested();
    });

    QPushButton* saveButton = new QPushButton("Save Scene");
    layout->addWidget(saveButton);
    connect(saveButton, &QPushButton::clicked, [this]() {
        QString filePath = QFileDialog::getSaveFileName(nullptr, "Save Scene", "", "Scene Files (*.scene)");
        if (!filePath.isEmpty()) this->scene->saveToFile(filePath);
    });

    QPushButton* loadButton = new QPushButton("Load Scene");
    layout->addWidget(loadButton);
    connect(loadButton, &QPushButton::clicked, [this]() {
        QString filePath = QFileDialog::getOpenFileName(nullptr, "Load Scene", "", "Scene Files (*.scene)");
        if (!filePath.isEmpty()) this->scene->loadFromFile(filePath);
    });

    // Play button
    QPushButton* playButton = new QPushButton("Play");
    layout->addWidget(playButton);

    // Build button
    QPushButton* buildButton = new QPushButton("Build");
    layout->addWidget(buildButton);

    layout->addStretch();
}