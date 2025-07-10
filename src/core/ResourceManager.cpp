#include "ResourceManager.h"
#include "ConsolePanel.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stdexcept>
#include <algorithm>
#include <cfloat>
#include <d3dx9.h>

std::unordered_map<QString, std::weak_ptr<Mesh>> ResourceManager::meshCache;

std::shared_ptr<Mesh> ResourceManager::loadMesh(const QString& path) {
    auto it = meshCache.find(path);
    if (it != meshCache.end()) {
        if (auto cached = it->second.lock()) {
            return cached;
        }
    }

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path.toStdString(),
        aiProcess_Triangulate |
        aiProcess_GenNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_ConvertToLeftHanded |
        aiProcess_CalcTangentSpace |
        aiProcess_GenUVCoords);

    if (!scene) {
        throw std::runtime_error(importer.GetErrorString());
    }

    if (!scene->HasMeshes()) {
        throw std::runtime_error("No meshes found in the file");
    }

    auto newMesh = std::make_shared<Mesh>();
    aiVector3D sceneMin(FLT_MAX, FLT_MAX, FLT_MAX);
    aiVector3D sceneMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for (unsigned m = 0; m < scene->mNumMeshes; ++m) {
        aiMesh* aiMesh = scene->mMeshes[m];
        for (unsigned i = 0; i < aiMesh->mNumVertices; ++i) {
            const aiVector3D& pos = aiMesh->mVertices[i];

            sceneMin.x = std::min<float>(sceneMin.x, pos.x);
            sceneMin.y = std::min<float>(sceneMin.y, pos.y);
            sceneMin.z = std::min<float>(sceneMin.z, pos.z);
            sceneMax.x = std::max<float>(sceneMax.x, pos.x);
            sceneMax.y = std::max<float>(sceneMax.y, pos.y);
            sceneMax.z = std::max<float>(sceneMax.z, pos.z);
        }
    }

    const float extentX = sceneMax.x - sceneMin.x;
    const float extentY = sceneMax.y - sceneMin.y;
    const float extentZ = sceneMax.z - sceneMin.z;

    const float maxExtent = (std::max)((std::max)(extentX, extentY), extentZ);
    const float targetScale = (maxExtent < 0.0001f) ? 1.0f : 1.0f / maxExtent;

    const float centerX = sceneMin.x + extentX * 0.5f;
    const float centerY = sceneMin.y + extentY * 0.5f;
    const float centerZ = sceneMin.z + extentZ * 0.5f;

    for (unsigned m = 0; m < scene->mNumMeshes; ++m) {
        aiMesh* aiMesh = scene->mMeshes[m];
        unsigned indexOffset = newMesh->vertices.size();

        for (unsigned i = 0; i < aiMesh->mNumVertices; ++i) {
            Vertex v{};
            const aiVector3D& pos = aiMesh->mVertices[i];

            v.x = (pos.x - centerX) * targetScale;
            v.y = -(pos.y - centerY) * targetScale;
            v.z = (pos.z - centerZ) * targetScale;

            if (aiMesh->HasNormals()) {
                v.nx = aiMesh->mNormals[i].x * targetScale;
                v.ny = -aiMesh->mNormals[i].y * targetScale;
                v.nz = aiMesh->mNormals[i].z * targetScale;
            }
            else {
                v.nx = 0;
                v.ny = 0;
                v.nz = 0;
            }

            if (aiMesh->HasTextureCoords(0)) {
                v.u = aiMesh->mTextureCoords[0][i].x;
                v.v = aiMesh->mTextureCoords[0][i].y;
            }
            else {
                v.u = 0;
                v.v = 0;
            }

            v.color = D3DCOLOR_XRGB(255, 255, 255);
            newMesh->vertices.push_back(v);
        }

        for (unsigned i = 0; i < aiMesh->mNumFaces; ++i) {
            const aiFace& face = aiMesh->mFaces[i];
            if (face.mNumIndices == 3) {
                newMesh->indices.push_back(indexOffset + face.mIndices[0]);
                newMesh->indices.push_back(indexOffset + face.mIndices[1]);
                newMesh->indices.push_back(indexOffset + face.mIndices[2]);
            }
        }
    }

    newMesh->minBounds = D3DXVECTOR3(
        (sceneMin.x - centerX) * targetScale,
        -(sceneMax.y - centerY) * targetScale,
        (sceneMin.z - centerZ) * targetScale
    );

    newMesh->maxBounds = D3DXVECTOR3(
        (sceneMax.x - centerX) * targetScale,
        -(sceneMin.y - centerY) * targetScale,
        (sceneMax.z - centerZ) * targetScale
    );

    meshCache[path] = newMesh;
    return newMesh;
}

void ResourceManager::clearUnusedResources() {
    for (auto it = meshCache.begin(); it != meshCache.end(); ) {
        if (it->second.expired()) {
            it = meshCache.erase(it);
        }
        else {
            ++it;
        }
    }
}