#pragma once

#include <QObject>
#include <vector>
#include <memory>
#include <mutex>
#include <d3d9.h>
#include <QJsonObject>
#include "MeshRenderer.h"
#include "SceneObject.h"
#include "Skybox.h"

class Scene : public QObject {
    Q_OBJECT
public:
    explicit Scene(QObject* parent = nullptr);
    ~Scene();

    void updateSkybox(LPDIRECT3DDEVICE9 device);
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
    void setSkyboxPath(const std::string& path) {
        if (skyboxPath != path) {
            skyboxPath = path;
            skyboxDirty = true;
        }
    }
    const std::string& getSkyboxPath() const { return skyboxPath; }

    void setAmbientColor(const D3DCOLORVALUE& color) {
        if (memcmp(&ambientColor, &color, sizeof(D3DCOLORVALUE)) != 0) {
            ambientColor = color;
            lightingDirty = true;
        }
    }
    const D3DCOLORVALUE& getAmbientColor() const { return ambientColor; }

    void setLightIntensity(float intensity) {
        if (lightIntensity != intensity) {
            lightIntensity = intensity;
            lightingDirty = true;
        }
    }
    float getLightIntensity() const { return lightIntensity; }

    void setShadowsEnabled(bool enabled) {
        if (shadowsEnabled != enabled) {
            shadowsEnabled = enabled;
            lightingDirty = true;
        }
    }
    bool getShadowsEnabled() const { return shadowsEnabled; }

    void setLightingEnabled(bool enabled) {
        if (lightingEnabled != enabled) {
            lightingEnabled = enabled;
            lightingDirty = true;
        }
    }
    bool getLightingEnabled() const { return lightingEnabled; }

    bool isLightingDirty() const { return lightingDirty; }
    void clearLightingDirty() { lightingDirty = false; }
    bool isSkyboxDirty() const { return skyboxDirty; }
    void clearSkyboxDirty() { skyboxDirty = false; }

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

    // Flags for tracking changes
    bool lightingDirty = true;
    bool skyboxDirty = true;
};