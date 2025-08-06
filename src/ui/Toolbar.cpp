#include <Toolbar.h>

Toolbar::Toolbar(Scene* scene, QWidget* parent)
    : QWidget(parent), scene(scene)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);

    Scene* scenePtr = this->scene;

    createButton = new QToolButton();
    createButton->setText("Create");
    createButton->setPopupMode(QToolButton::InstantPopup);

    createMenu = new QMenu(createButton);

    createLightAction = new QAction("Light", this);
    connect(createLightAction, &QAction::triggered, [scenePtr]() {
        auto obj = std::make_unique<SceneObject>("Light");
        auto* light = obj->addComponent<Light>();
        scenePtr->addObject(std::move(obj));
        ConsolePanel::sLog(LogType::Info, "Light has been added to scene");
    });
    createMenu->addAction(createLightAction);

    createMeshAction = new QAction("Mesh", this);
    connect(createMeshAction, &QAction::triggered, [scenePtr]() {
        auto obj = std::make_unique<SceneObject>("Mesh");
        obj->addComponent<MeshRenderer>();
        scenePtr->addObject(std::move(obj));
    });
    createMenu->addAction(createMeshAction);

    createEmptyAction = new QAction("Empty", this);
    connect(createEmptyAction, &QAction::triggered, [scenePtr]() {
        auto obj = std::make_unique<SceneObject>("Empty");
        scenePtr->addObject(std::move(obj));
    });
    createMenu->addAction(createEmptyAction);

    createButton->setMenu(createMenu);
    layout->addWidget(createButton);

    envSettingsButton = new QPushButton("Environment Settings");
    layout->addWidget(envSettingsButton);
    connect(envSettingsButton, &QPushButton::clicked, [this]() {
        emit environmentSettingsRequested();
    });

    saveButton = new QPushButton("Save Scene");
    layout->addWidget(saveButton);
    connect(saveButton, &QPushButton::clicked, [this]() {
        QString filePath = QFileDialog::getSaveFileName(nullptr, "Save Scene", "", "Scene Files (*.scene)");
        if (!filePath.isEmpty()) this->scene->saveToFile(filePath);
    });

    loadButton = new QPushButton("Load Scene");
    layout->addWidget(loadButton);
    connect(loadButton, &QPushButton::clicked, [this]() {
        QString filePath = QFileDialog::getOpenFileName(nullptr, "Load Scene", "", "Scene Files (*.scene)");
        if (!filePath.isEmpty()) this->scene->loadFromFile(filePath);
    });

    buildButton = new QPushButton("Build");
    layout->addWidget(buildButton);

    playButton = new QPushButton("Play");
    layout->addWidget(playButton);

    connect(playButton, &QPushButton::clicked, [this]() {
        static bool isPlaying = false;
        isPlaying = !isPlaying;

        if (isPlaying) {
            playButton->setText("Stop");
            this->scene->setPhysicsEnabled(true);
        }
        else {
            playButton->setText("Play");
            this->scene->setPhysicsEnabled(false);
        }
        });

    layout->addStretch();
}
