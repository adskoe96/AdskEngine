#pragma once
#include "SceneObject.h"
#include <d3dx9.h>
#include <qobjectdefs.h>

struct CubeVertex {
    float x, y, z;
    D3DCOLOR color;
};
#define D3DFVF_CUBE (D3DFVF_XYZ | D3DFVF_DIFFUSE)

class Cube : public SceneObject {
    Q_OBJECT
public:
    Cube(QObject* parent = nullptr);
    ~Cube() override;

    void render(LPDIRECT3DDEVICE9 device) override;
    std::string getName() const override { return "Cube"; }
    unsigned int getId() const override { return 1; }

    void invalidateDeviceObjects() override;
    bool restoreDeviceObjects(LPDIRECT3DDEVICE9 device) override;

private:
    LPDIRECT3DVERTEXBUFFER9 vertexBuffer = nullptr;
    bool needsRestore = true;
};