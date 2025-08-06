#pragma once

#include "Component.h"
#include "Transform.h"
#include "SceneObject.h"

#include <d3dx9.h>
#include <QString>
#include <QJsonObject>
#include <vector>

struct Vertex {
    float x, y, z;
    float nx, ny, nz;
    D3DCOLOR color;
    float u, v;
};

#define FVF_VERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE)

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<WORD> indices;
    D3DXVECTOR3 minBounds;
    D3DXVECTOR3 maxBounds;
};

class MeshRenderer : public Component {
    Q_OBJECT
public:
    MeshRenderer() = default;
    ~MeshRenderer() override { invalidate(); }

    QJsonObject serialize() const override;
    void deserialize(const QJsonObject& data) override;

    std::string getTypeName() const override { return "MeshRenderer"; }

    void render(LPDIRECT3DDEVICE9 device) override;
    void createInspector(QWidget* parent, QFormLayout* layout) override;

    void invalidateDeviceObjects() override;
    bool restoreDeviceObjects(LPDIRECT3DDEVICE9 device) override;
    void onPropertiesChanged() override;

    void setMeshPath(const QString& path);
    const QString& getMeshPath() const { return meshPath; }

    bool isVisible(const D3DXMATRIX& viewProj) const;

private:
    LPDIRECT3DVERTEXBUFFER9 vb = nullptr;
    LPDIRECT3DINDEXBUFFER9 ib = nullptr;
    bool needsRestore = true;

    std::shared_ptr<Mesh> mesh;
    D3DXMATRIX cachedWorldMatrix;
    bool worldMatrixValid = false;

    QString meshPath;
    QLabel* mrLabel;

    bool loadMeshFromFile(const QString& path);
    void releaseResources();
    void updateWorldMatrix();
};
