#include "Toolbar.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include "Scene.h"
#include "Cube.h"
#include "Light.h"
#include <QFileDialog>

Toolbar::Toolbar(Scene* scene, QWidget* parent)
    : QWidget(parent), scene(scene)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);

    // Сохраняем указатель на сцену в локальной переменной для явного захвата
    Scene* scenePtr = this->scene;

    // Кнопка создания объектов с выпадающим меню
    QToolButton* createButton = new QToolButton();
    createButton->setText("Create");
    createButton->setPopupMode(QToolButton::InstantPopup);

    QMenu* createMenu = new QMenu(createButton);

    QAction* createLightAction = new QAction("Light", this);
    connect(createLightAction, &QAction::triggered, [scenePtr]() {
        scenePtr->addObject(std::make_unique<Light>());
        });
    createMenu->addAction(createLightAction);

    QAction* createCubeAction = new QAction("Cube", this);
    connect(createCubeAction, &QAction::triggered, [scenePtr]() {
        scenePtr->addObject(std::make_unique<Cube>());
        });
    createMenu->addAction(createCubeAction);

    createButton->setMenu(createMenu);
    layout->addWidget(createButton);

    // Кнопка Play
    QPushButton* playButton = new QPushButton("Play");
    layout->addWidget(playButton);

    // Кнопка настроек окружения
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

    layout->addStretch();
}