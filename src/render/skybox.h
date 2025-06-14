#pragma once
#include <d3d9.h>
#include <d3dx9.h>

class Skybox {
public:
    Skybox();
    ~Skybox();

    bool initialize(LPDIRECT3DDEVICE9 device, const char* texturePath);
    void render(LPDIRECT3DDEVICE9 device, const D3DXVECTOR3& cameraDir, const D3DXVECTOR3& cameraUp, float aspectRatio);
    void cleanup();

private:
    LPDIRECT3DVERTEXBUFFER9 vertexBuffer = nullptr;
    LPDIRECT3DTEXTURE9 texture = nullptr;
};