#pragma once

#include <QObject>
#include <vector>
#include <memory>
#include <mutex>
#include <d3d9.h>
#include <QJsonObject>
#include "SceneObject.h"
#include "Skybox.h"

class Scene : public QObject {
    Q_OBJECT
public:
    explicit Scene(QObject* parent = nullptr);
    ~Scene();

    bool initializeSkybox(LPDIRECT3DDEVICE9 device);
    Skybox* getSkybox() const { return skybox.get(); }

    void addObject(std::unique_ptr<SceneObject> object);
    void removeObject(SceneObject* object);

    void render(LPDIRECT3DDEVICE9 device);
    const std::vector<std::unique_ptr<SceneObject>>& getObjects() const;

    void invalidateDeviceObjects();
    void restoreDeviceObjects(LPDIRECT3DDEVICE9 device);

    // Serialization
    void saveToFile(const QString& filePath);
    void loadFromFile(const QString& filePath);

    // Environment settings
    void setSkyboxPath(const std::string& path) { skyboxPath = path; }
    const std::string& getSkyboxPath() const { return skyboxPath; }

    void setAmbientColor(const D3DCOLORVALUE& color) { ambientColor = color; }
    const D3DCOLORVALUE& getAmbientColor() const { return ambientColor; }

    void setLightIntensity(float intensity) { lightIntensity = intensity; }
    float getLightIntensity() const { return lightIntensity; }

    void setShadowsEnabled(bool enabled) { shadowsEnabled = enabled; }
    bool getShadowsEnabled() const { return shadowsEnabled; }

    void setLightingEnabled(bool enabled) { lightingEnabled = enabled; }
    bool getLightingEnabled() const { return lightingEnabled; }

signals:
    void objectAdded(SceneObject* object);
    void objectRemoved(SceneObject* object);
    void objectPropertiesChanged();

private:
    std::vector<std::unique_ptr<SceneObject>> objects;
    std::unique_ptr<Skybox> skybox;
    std::string skyboxPath;
    std::mutex sceneMutex;

    D3DCOLORVALUE ambientColor{};
    float lightIntensity = 1.0f;
    bool shadowsEnabled = true;
    bool lightingEnabled = false;
    bool skyboxInitialized = false;
};