#include "Skybox.h"
#include <qlogging.h>
#include <ConsolePanel.h>

struct SkyboxVertex {
    float x, y, z;
    float u, v;
};

#define D3DFVF_SKYBOX (D3DFVF_XYZ | D3DFVF_TEX1)

Skybox::Skybox() {}
Skybox::~Skybox() {
    cleanup();
}

bool Skybox::initialize(LPDIRECT3DDEVICE9 device, const char* texturePath) {
    SkyboxVertex vertices[] = {
        // edge +Z
        { -1,  1,  1, 0, 0 }, { 1,  1,  1, 1, 0 }, { 1, -1,  1, 1, 1 },
        { -1,  1,  1, 0, 0 }, { 1, -1,  1, 1, 1 }, { -1, -1,  1, 0, 1 },
        // edge -Z
        {  1,  1, -1, 0, 0 }, { -1,  1, -1, 1, 0 }, { -1, -1, -1, 1, 1 },
        {  1,  1, -1, 0, 0 }, { -1, -1, -1, 1, 1 }, {  1, -1, -1, 0, 1 },
        // edge +X
        {  1,  1,  1, 0, 0 }, {  1,  1, -1, 1, 0 }, {  1, -1, -1, 1, 1 },
        {  1,  1,  1, 0, 0 }, {  1, -1, -1, 1, 1 }, {  1, -1,  1, 0, 1 },
        // edge -X
        { -1,  1, -1, 0, 0 }, { -1,  1,  1, 1, 0 }, { -1, -1,  1, 1, 1 },
        { -1,  1, -1, 0, 0 }, { -1, -1,  1, 1, 1 }, { -1, -1, -1, 0, 1 },
        // edge +Y
        { -1,  1, -1, 0, 0 }, {  1,  1, -1, 1, 0 }, {  1,  1,  1, 1, 1 },
        { -1,  1, -1, 0, 0 }, {  1,  1,  1, 1, 1 }, { -1,  1,  1, 0, 1 },
        // edge -Y
        { -1, -1,  1, 0, 0 }, {  1, -1,  1, 1, 0 }, {  1, -1, -1, 1, 1 },
        { -1, -1,  1, 0, 0 }, {  1, -1, -1, 1, 1 }, { -1, -1, -1, 0, 1 },
    };

    if (FAILED(device->CreateVertexBuffer(sizeof(vertices), 0, D3DFVF_SKYBOX,
        D3DPOOL_MANAGED, &vertexBuffer, nullptr))) {
        ConsolePanel::sError("Failed to create vertex buffer");
        return false;
    }

    void* data = nullptr;
    vertexBuffer->Lock(0, sizeof(vertices), &data, 0);
    memcpy(data, vertices, sizeof(vertices));
    vertexBuffer->Unlock();

    if (texturePath && *texturePath) {
        if (FAILED(D3DXCreateTextureFromFileA(device, texturePath, &texture))) {
            ConsolePanel::sError("Failed to load skybox texture from: " + QString(texturePath));
            return false;
        }
    }
    else {
        // Запасной вариант: белая текстура
        if (FAILED(device->CreateTexture(256, 256, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &texture, nullptr))) {
            ConsolePanel::sError("Failed to create fallback texture");
            return false;
        }
        D3DLOCKED_RECT rect;
        texture->LockRect(0, &rect, nullptr, 0);
        uint32_t* pixels = static_cast<uint32_t*>(rect.pBits);
        for (int i = 0; i < 256 * 256; ++i) {
            pixels[i] = 0xFFFFFFFF;
        }
        texture->UnlockRect(0);
    }

    return true;
}

void Skybox::render(LPDIRECT3DDEVICE9 device, const D3DXVECTOR3& cameraDir, const D3DXVECTOR3& cameraUp, float aspectRatio) {
    if (!vertexBuffer || !texture) return;

    device->SetRenderState(D3DRS_ZENABLE, FALSE);
    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    D3DXMATRIX world, view, proj;
    D3DXMatrixIdentity(&world);
    device->SetTransform(D3DTS_WORLD, &world);

    D3DXMatrixPerspectiveFovLH(&proj, D3DXToRadian(90), aspectRatio, 0.1f, 100.0f);
    device->SetTransform(D3DTS_PROJECTION, &proj);

    D3DXVECTOR3 eye(0, 0, 0);
    D3DXVECTOR3 at = eye + cameraDir;
    D3DXMatrixLookAtLH(&view, &eye, &at, &cameraUp);
    device->SetTransform(D3DTS_VIEW, &view);

    device->SetStreamSource(0, vertexBuffer, 0, sizeof(SkyboxVertex));
    device->SetFVF(D3DFVF_SKYBOX);
    device->SetTexture(0, texture);
    device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 12); // 6 edges * 2 triangles

    device->SetRenderState(D3DRS_ZENABLE, TRUE);
    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

void Skybox::cleanup() {
    if (vertexBuffer) {
        vertexBuffer->Release();
        vertexBuffer = nullptr;
    }
    if (texture) {
        texture->Release();
        texture = nullptr;
    }
}