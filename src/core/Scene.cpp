#include "Scene.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QFile>
#include "Light.h"
#include "MeshRenderer.h"
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

    QJsonArray objArray;
    for (const auto& objPtr : objects) {
        SceneObject* obj = objPtr.get();
        QJsonObject o;

        // Always Transform
        if (auto* tr = obj->getComponent<Transform>()) {
            QJsonObject jsTr;
            const auto& pos = tr->getPosition();
            const auto& rot = tr->getRotation();
            const auto& scl = tr->getScale();

            jsTr["posX"] = pos.x;
            jsTr["posY"] = pos.y;
            jsTr["posZ"] = pos.z;
            jsTr["rotX"] = rot.x;
            jsTr["rotY"] = rot.y;
            jsTr["rotZ"] = rot.z;
            jsTr["scaleX"] = scl.x;
            jsTr["scaleY"] = scl.y;
            jsTr["scaleZ"] = scl.z;
            o["Transform"] = jsTr;
        }

        // MeshRenderer?
        if (obj->getComponent<MeshRenderer>()) {
            o["MeshRenderer"] = true;
        }

        // Light?
        if (auto* light = obj->getComponent<Light>()) {
            QJsonObject jsL;
            jsL["type"] = int(light->type);
            jsL["intensity"] = light->intensity;
            jsL["colorR"] = light->color.r;
            jsL["colorG"] = light->color.g;
            jsL["colorB"] = light->color.b;
            o["Light"] = jsL;
        }

        objArray.append(o);
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
    QJsonArray objArray = root["objects"].toArray();
    for (auto v : objArray) {
        auto jsObj = v.toObject();
        auto newObj = std::make_unique<SceneObject>("Entity");

        // Transform
        if (jsObj.contains("Transform")) {
            auto jsTr = jsObj["Transform"].toObject();
            auto* tr = newObj->getComponent<Transform>();

            tr->setPosition({
                static_cast<float>(jsTr["posX"].toDouble()),
                static_cast<float>(jsTr["posY"].toDouble()),
                static_cast<float>(jsTr["posZ"].toDouble())
                });

            tr->setRotation({
                static_cast<float>(jsTr["rotX"].toDouble()),
                static_cast<float>(jsTr["rotY"].toDouble()),
                static_cast<float>(jsTr["rotZ"].toDouble())
                });

            tr->setScale({
                static_cast<float>(jsTr["scaleX"].toDouble()),
                static_cast<float>(jsTr["scaleY"].toDouble()),
                static_cast<float>(jsTr["scaleZ"].toDouble())
                });
        }

        // MeshRenderer
        if (jsObj.contains("MeshRenderer") && jsObj["MeshRenderer"].toBool()) {
            newObj->addComponent<MeshRenderer>();
        }

        // Light
        if (jsObj.contains("Light")) {
            auto jsL = jsObj["Light"].toObject();
            auto* light = newObj->addComponent<Light>();
            light->type = LightType(jsL["type"].toInt());
            light->intensity = jsL["intensity"].toDouble();
            light->color.r = jsL["colorR"].toDouble();
            light->color.g = jsL["colorG"].toDouble();
            light->color.b = jsL["colorB"].toDouble();
            light->color.a = 1.0f;
        }

        addObject(std::move(newObj));
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