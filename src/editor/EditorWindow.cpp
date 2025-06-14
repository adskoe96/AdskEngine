#include "EditorWindow.h"
#include "Viewport.h"
#include "Toolbar.h"
#include "ConsolePanel.h"
#include "ProjectPanel.h"
#include "PropertiesPanel.h"
#include "SceneHierarchyPanel.h"
#include "Scene.h"
#include "EnvironmentSettingsWindow.h"

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QSplitter>
#include <QVBoxLayout>

EditorWindow::EditorWindow(const QString& projectPath, QWidget* parent)
    : QMainWindow(parent), m_projectPath(projectPath)
{
    QString projectName = "Untitled";

    QDir dir(projectPath);
    QStringList jsonFiles = dir.entryList(QStringList() << "*.json", QDir::Files);
    if (!jsonFiles.isEmpty()) {
        QFile jsonFile(dir.absoluteFilePath(jsonFiles.first()));
        if (jsonFile.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(jsonFile.readAll());
            QJsonObject obj = doc.object();
            projectName = obj["name"].toString();
        }
    }

    setWindowTitle(QString("Adsk Engine - Editor - %1").arg(projectName));
    resize(1600, 900);

    scene = new Scene();
    auto* viewport = new Viewport();
    viewport->setScene(scene);

    // Создаем центральный виджет и основной лэйаут
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Создаем тулбар и добавляем его в лэйаут
    auto* toolbar = new Toolbar(scene);
    toolbar->setFixedHeight(40);
    mainLayout->addWidget(toolbar);

    // Подключаем сигнал для открытия настроек окружения
    connect(toolbar, &Toolbar::environmentSettingsRequested,
        this, &EditorWindow::openEnvironmentSettings);

    // Создаем сплиттеры и панели
    auto* verticalSplitter = new QSplitter(Qt::Vertical);
    mainLayout->addWidget(verticalSplitter);

    auto* topSplitter = new QSplitter(Qt::Horizontal);
    verticalSplitter->addWidget(topSplitter);
    verticalSplitter->setStretchFactor(0, 8);

    auto* sceneHierarchyPanel = new SceneHierarchyPanel(scene);
    sceneHierarchyPanel->setMinimumWidth(200);
    topSplitter->addWidget(sceneHierarchyPanel);

    //auto* viewport = new Viewport();
    topSplitter->addWidget(viewport);

    auto* propertiesPanel = new PropertiesPanel(scene);
    propertiesPanel->setMinimumWidth(250);
    topSplitter->addWidget(propertiesPanel);

    connect(sceneHierarchyPanel, &SceneHierarchyPanel::objectSelected,
        propertiesPanel, &PropertiesPanel::onObjectSelected);

    auto* bottomSplitter = new QSplitter(Qt::Horizontal);
    verticalSplitter->addWidget(bottomSplitter);
    verticalSplitter->setStretchFactor(1, 2);

    auto* projectPanel = new ProjectPanel(m_projectPath);
    bottomSplitter->addWidget(projectPanel);

    auto* consolePanel = new ConsolePanel();
    bottomSplitter->addWidget(consolePanel);

    consolePanel->NewLog("Console initialized!\nProject " + projectName + " loaded successful");

    connect(scene, &Scene::objectAdded, sceneHierarchyPanel, &SceneHierarchyPanel::updateHierarchy);
}

void EditorWindow::openEnvironmentSettings() {
    EnvironmentSettingsWindow* settingsWindow = new EnvironmentSettingsWindow(scene, this);
    settingsWindow->exec();
    delete settingsWindow;
}