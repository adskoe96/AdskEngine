#pragma once

#include "Component.h"
#include "Transform.h"
#include "SceneObject.h"
#include <d3dx9.h>

struct Vertex { float x, y, z; float nx, ny, nz; D3DCOLOR color; };
#define FVF_VERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE)

class MeshRenderer : public Component {
public:
    MeshRenderer() = default;
    ~MeshRenderer() override { invalidate(); }

    void render(LPDIRECT3DDEVICE9 device) override {
        if (!restore(device)) return;
        auto* tr = getOwner()->getComponent<Transform>();
        D3DXMATRIX world = tr ? tr->getWorldMatrix() : D3DXMATRIX();
        D3DXMATRIX old;
        device->GetTransform(D3DTS_WORLD, &old);
        device->SetTransform(D3DTS_WORLD, &world);

        device->SetStreamSource(0, vb, 0, sizeof(Vertex));
        device->SetFVF(FVF_VERTEX);
        device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2 * 6);

        device->SetTransform(D3DTS_WORLD, &old);
    }

    void invalidate() {
        if (vb) { vb->Release(); vb = nullptr; needsRestore = true; }
    }

private:
    LPDIRECT3DVERTEXBUFFER9 vb = nullptr;
    bool needsRestore = true;

    bool restore(LPDIRECT3DDEVICE9 device) {
        if (!needsRestore) return true;
        Vertex verts[24]; // temporary option
        // ...
        if (FAILED(device->CreateVertexBuffer(sizeof(verts), 0, FVF_VERTEX, D3DPOOL_MANAGED, &vb, nullptr)))
            return false;
        void* ptr; vb->Lock(0, sizeof(verts), &ptr, 0);
        memcpy(ptr, verts, sizeof(verts)); vb->Unlock();
        needsRestore = false;
        return true;
    }
};