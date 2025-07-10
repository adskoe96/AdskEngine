#pragma once

#include <QString>
#include <memory>
#include <unordered_map>
#include "MeshRenderer.h"

class ResourceManager {
public:
    static std::shared_ptr<Mesh> loadMesh(const QString& path);
    static void clearUnusedResources();

private:
    static std::unordered_map<QString, std::weak_ptr<Mesh>> meshCache;
};