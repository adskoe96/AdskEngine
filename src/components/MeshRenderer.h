#pragma once

#include "Component.h"
#include "Transform.h"
#include "SceneObject.h"

#include <d3dx9.h>
#include <QString>
#include <vector>

struct Vertex {
    float x, y, z;
    float nx, ny, nz;
    D3DCOLOR color;
};

#define FVF_VERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE)

class MeshRenderer : public Component {
    Q_OBJECT
public:
    MeshRenderer() = default;
    ~MeshRenderer() override { invalidate(); }

    void render(LPDIRECT3DDEVICE9 device) override;
    void createInspector(QWidget* parent, QFormLayout* layout) override;

    void invalidate() override;
    bool restoreDeviceObjects(LPDIRECT3DDEVICE9 device) override;

    void setMeshPath(const QString& path);
    const QString& getMeshPath() const { return meshPath; }

private:
    LPDIRECT3DVERTEXBUFFER9 vb = nullptr;
    LPDIRECT3DINDEXBUFFER9 ib = nullptr;
    bool needsRestore = true;

    std::vector<Vertex> vertices;
    std::vector<WORD> indices;

    QString meshPath;

    bool loadMeshFromFile(const QString& path);
};
