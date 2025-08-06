#include "Scene.h"
#include "Light.h"
#include "MeshRenderer.h"
#include "ConsolePanel.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QFile>
#include <QApplication>

Scene::Scene(QObject* parent) : QObject(parent)
{
    PhysicsSystem::getInstance().initialize(this);
    ambientColor.r = 0.2f;
    ambientColor.g = 0.2f;
    ambientColor.b = 0.2f;
    ambientColor.a = 1.0f;
}

Scene::~Scene() {
    invalidateDeviceObjects();
}

void Scene::addObject(std::unique_ptr<SceneObject> object) {
    std::lock_guard<std::mutex> lock(sceneMutex);
    SceneObject* raw = object.get();
    connect(raw, &SceneObject::propertiesChanged,
        this, &Scene::objectPropertiesChanged);
    objects.push_back(std::move(object));
    emit objectAdded(raw);
    emit objectPropertiesChanged();
}

void Scene::removeObject(SceneObject* object) {
    std::lock_guard<std::mutex> lock(sceneMutex);
    auto it = std::find_if(objects.begin(), objects.end(),
        [&](const std::unique_ptr<SceneObject>& ptr) {
            return ptr.get() == object;
        });
    if (it != objects.end()) {
        objects.erase(it);
        emit objectRemoved(object);
        emit objectPropertiesChanged();
    }
}

void Scene::render(LPDIRECT3DDEVICE9 device) {
    std::lock_guard<std::mutex> lock(sceneMutex);
    for (const auto& obj : objects) {
        obj->render(device);
    }
}

const std::vector<std::unique_ptr<SceneObject>>& Scene::getObjects() const {
    return objects;
}

void Scene::physicsUpdate(float deltaTime)
{
    PhysicsSystem::getInstance().simulate(deltaTime);
}

void Scene::setPhysicsEnabled(bool enabled)
{
    if (enabled) {
        PhysicsSystem::getInstance().saveState();
    }
    else {
        PhysicsSystem::getInstance().restoreState();
    }
    PhysicsSystem::getInstance().setSimulationEnabled(enabled);
}

void Scene::invalidateDeviceObjects() {
    if (skybox) {
        skybox->cleanup();
        skybox.reset();
    }

    for (const auto& obj : objects) {
        obj->invalidateDeviceObjects();
    }

    skyboxInitialized = false;
}

void Scene::restoreDeviceObjects(LPDIRECT3DDEVICE9 device) {
    // Skybox will be reloaded in updateSkybox()
    skyboxDirty = true;

    for (const auto& obj : objects) {
        obj->restoreDeviceObjects(device);
    }
}

void Scene::saveToFile(const QString& filePath) {
    QJsonObject root;
    QJsonObject ambient;
    ambient["r"] = ambientColor.r;
    ambient["g"] = ambientColor.g;
    ambient["b"] = ambientColor.b;
    ambient["a"] = ambientColor.a;
    root["ambientColor"] = ambient;
    root["lightIntensity"] = lightIntensity;
    root["shadowsEnabled"] = shadowsEnabled;
    root["lightingEnabled"] = lightingEnabled;
    root["skyboxPath"] = QString::fromStdString(skyboxPath);

    QJsonArray objArray;
    for (const auto& objPtr : objects) {
        objArray.append(objPtr->serialize());
    }
    root["objects"] = objArray;

    QFile f(filePath);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(root).toJson());
    }

    ConsolePanel::sInfo("Scene saved to: " + filePath);
}

void Scene::loadFromFile(const QString& filePath) {
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) {
        ConsolePanel::sError("Failed to open scene file: " + filePath);
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if (doc.isNull()) {
        ConsolePanel::sError("Invalid JSON in scene file: " + filePath);
        return;
    }

    QJsonObject root = doc.object();

    auto ambient = root["ambientColor"].toObject();
    ambientColor.r = ambient["r"].toDouble();
    ambientColor.g = ambient["g"].toDouble();
    ambientColor.b = ambient["b"].toDouble();
    ambientColor.a = ambient["a"].toDouble();

    lightIntensity = root["lightIntensity"].toDouble();
    shadowsEnabled = root["shadowsEnabled"].toBool();
    lightingEnabled = root["lightingEnabled"].toBool();
    skyboxPath = root["skyboxPath"].toString().toStdString();

    objects.clear();

    QJsonArray objArray = root["objects"].toArray();
    for (const auto& objValue : objArray) {
        QJsonObject jsObj = objValue.toObject();

        std::string name = jsObj["name"].toString().toStdString();
        auto newObj = std::make_unique<SceneObject>(name);

        QJsonArray components = jsObj["components"].toArray();
        for (const auto& compValue : components) {
            QJsonObject compObj = compValue.toObject();
            QString type = compObj["type"].toString();
            QJsonObject data = compObj["data"].toObject();

            if (type == "Transform") {
                if (auto* tr = newObj->getComponent<Transform>()) {
                    tr->deserialize(data);
                }
            }
            else if (type == "MeshRenderer") {
                if (auto* mr = newObj->addComponent<MeshRenderer>()) {
                    mr->deserialize(data);
                }
            }
            else if (type == "Light") {
                if (auto* light = newObj->addComponent<Light>()) {
                    light->deserialize(data);
                }
            }
        }

        addObject(std::move(newObj));
    }

    skyboxDirty = true;
    lightingDirty = true;

    ConsolePanel::sInfo("Scene loaded from: " + filePath);
}

void Scene::updateSkybox(LPDIRECT3DDEVICE9 device) {
    if (skyboxDirty) {
        // Release old skybox
        if (skybox) {
            skybox->cleanup();
            skybox.reset();
        }

        // Load new skybox if path is valid
        if (!skyboxPath.empty()) {
            skybox = std::make_unique<Skybox>();
            if (skybox->initialize(device, skyboxPath.c_str())) {
                skyboxInitialized = true;
            }
            else {
                ConsolePanel::sError("Failed to initialize skybox");
                skybox.reset();
            }
        }

        skyboxDirty = false;
    }
}