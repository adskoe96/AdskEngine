#include "Scene.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QFile>
#include "Cube.h"
#include "Light.h"
#include <ConsolePanel.h>
#include <QApplication>

Scene::Scene(QObject* parent)
    : QObject(parent)
{
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

    QJsonArray arr;
    for (const auto& obj : objects) {
        QJsonObject o;
        o["id"] = int(obj->getId());
        o["posX"] = obj->getPositionX();
        o["posY"] = obj->getPositionY();
        o["posZ"] = obj->getPositionZ();
        arr.append(o);
    }
    root["objects"] = arr;

    QFile f(filePath);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(root).toJson());
    }

    ConsolePanel::sInfo("Scene saved to: " + filePath);
}

void Scene::loadFromFile(const QString& filePath) {
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly))
        return;

    auto doc = QJsonDocument::fromJson(f.readAll());
    auto root = doc.object();

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
    QJsonArray arr = root["objects"].toArray();
    for (auto v : arr) {
        auto o = v.toObject();
        int id = o["id"].toInt();
        std::unique_ptr<SceneObject> obj;
        if (id == 1) obj = std::make_unique<Cube>(this);
        else if (id == 2) obj = std::make_unique<Light>(this);
        if (obj) {
            obj->setPositionX(o["posX"].toDouble());
            obj->setPositionY(o["posY"].toDouble());
            obj->setPositionZ(o["posZ"].toDouble());
            addObject(std::move(obj));
        }
    }

    // Mark for reload
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