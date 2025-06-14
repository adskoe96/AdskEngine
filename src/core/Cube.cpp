#include "Cube.h"
#include <d3dx9.h>

Cube::Cube(QObject* parent) : SceneObject(parent) {}
Cube::~Cube() { invalidateDeviceObjects(); }

void Cube::render(LPDIRECT3DDEVICE9 device) {
    if (!restoreDeviceObjects(device)) return;

    // Build world matrix: rotation then translation
    D3DXMATRIX rotX, rotY, rotZ, world;
    D3DXMatrixRotationX(&rotX, rotation.x);
    D3DXMatrixRotationY(&rotY, rotation.y);
    D3DXMatrixRotationZ(&rotZ, rotation.z);
    world = rotZ * rotY * rotX;

    D3DXMATRIX trans;
    D3DXMatrixTranslation(&trans, position.x, position.y, position.z);
    world *= trans;

    // Apply world
    D3DXMATRIX oldWorld;
    device->GetTransform(D3DTS_WORLD, &oldWorld);
    device->SetTransform(D3DTS_WORLD, &world);

    // Установка материала
    D3DMATERIAL9 mat;
    ZeroMemory(&mat, sizeof(mat));
    mat.Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f }; // Белый цвет для полного отражения света
    mat.Ambient = { 0.1f, 0.1f, 0.1f, 1.0f }; // Небольшой фоновый свет
    device->SetMaterial(&mat);

    device->SetStreamSource(0, vertexBuffer, 0, sizeof(CubeVertex));
    device->SetFVF(D3DFVF_CUBE);
    for (int i = 0; i < 6; ++i) device->DrawPrimitive(D3DPT_TRIANGLEFAN, i * 4, 2);

    // Restore old world
    device->SetTransform(D3DTS_WORLD, &oldWorld);
}

void Cube::invalidateDeviceObjects() {
    if (vertexBuffer) { vertexBuffer->Release(); vertexBuffer = nullptr; }
    needsRestore = true;
}

bool Cube::restoreDeviceObjects(LPDIRECT3DDEVICE9 device) {
    if (!needsRestore || vertexBuffer) return true;

    CubeVertex vertices[] = {
        // Передняя грань (z = -1)
        {-1, 1, -1, 0, 0, -1, D3DCOLOR_XRGB(255, 0, 0)},
        {1, 1, -1, 0, 0, -1, D3DCOLOR_XRGB(255, 0, 0)},
        {1, -1, -1, 0, 0, -1, D3DCOLOR_XRGB(255, 0, 0)},
        {-1, -1, -1, 0, 0, -1, D3DCOLOR_XRGB(255, 0, 0)},
        // Задняя грань (z = 1)
        {-1, 1, 1, 0, 0, 1, D3DCOLOR_XRGB(0, 255, 0)},
        {1, 1, 1, 0, 0, 1, D3DCOLOR_XRGB(0, 255, 0)},
        {1, -1, 1, 0, 0, 1, D3DCOLOR_XRGB(0, 255, 0)},
        {-1, -1, 1, 0, 0, 1, D3DCOLOR_XRGB(0, 255, 0)},
        // Левая грань (x = -1)
        {-1, 1, 1, -1, 0, 0, D3DCOLOR_XRGB(0, 0, 255)},
        {-1, 1, -1, -1, 0, 0, D3DCOLOR_XRGB(0, 0, 255)},
        {-1, -1, -1, -1, 0, 0, D3DCOLOR_XRGB(0, 0, 255)},
        {-1, -1, 1, -1, 0, 0, D3DCOLOR_XRGB(0, 0, 255)},
        // Правая грань (x = 1)
        {1, 1, -1, 1, 0, 0, D3DCOLOR_XRGB(255, 255, 0)},
        {1, 1, 1, 1, 0, 0, D3DCOLOR_XRGB(255, 255, 0)},
        {1, -1, 1, 1, 0, 0, D3DCOLOR_XRGB(255, 255, 0)},
        {1, -1, -1, 1, 0, 0, D3DCOLOR_XRGB(255, 255, 0)},
        // Верхняя грань (y = 1)
        {-1, 1, 1, 0, 1, 0, D3DCOLOR_XRGB(255, 0, 255)},
        {1, 1, 1, 0, 1, 0, D3DCOLOR_XRGB(255, 0, 255)},
        {1, 1, -1, 0, 1, 0, D3DCOLOR_XRGB(255, 0, 255)},
        {-1, 1, -1, 0, 1, 0, D3DCOLOR_XRGB(255, 0, 255)},
        // Нижняя грань (y = -1)
        {-1, -1, -1, 0, -1, 0, D3DCOLOR_XRGB(0, 255, 255)},
        {1, -1, -1, 0, -1, 0, D3DCOLOR_XRGB(0, 255, 255)},
        {1, -1, 1, 0, -1, 0, D3DCOLOR_XRGB(0, 255, 255)},
        {-1, -1, 1, 0, -1, 0, D3DCOLOR_XRGB(0, 255, 255)},
    };

    if (FAILED(device->CreateVertexBuffer(sizeof(vertices), 0, D3DFVF_CUBE, D3DPOOL_MANAGED, &vertexBuffer, nullptr)))
        return false;
    void* ptr = nullptr;
    if (FAILED(vertexBuffer->Lock(0, sizeof(vertices), &ptr, 0)))
        return false;
    memcpy(ptr, vertices, sizeof(vertices));
    vertexBuffer->Unlock();
    needsRestore = false;
    return true;
}