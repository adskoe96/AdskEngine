#pragma once
#include <d3d9.h>
#include <d3dx9.h>

class Skybox {
public:
    Skybox();
    ~Skybox();

    bool initialize(LPDIRECT3DDEVICE9 device, const char* texturePath);
    void cleanup();
    void draw(LPDIRECT3DDEVICE9 device);

private:
    LPDIRECT3DVERTEXBUFFER9 vertexBuffer = nullptr;
    LPDIRECT3DTEXTURE9 texture = nullptr;
};