#include "MeshRenderer.h"
#include "Transform.h"
#include "ConsolePanel.h"
#include "ResourceManager.h"

#include <d3d9types.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <QPushButton>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>


void MeshRenderer::render(LPDIRECT3DDEVICE9 device) {
    if (!mesh || mesh->vertices.empty()) return;

    if (needsRestore) {
        if (!restoreDeviceObjects(device)) {
            return;
        }
    }

    if (!vb || !ib) {
        return;
    }

    updateWorldMatrix();

    D3DXMATRIX old;
    device->GetTransform(D3DTS_WORLD, &old);
    device->SetTransform(D3DTS_WORLD, &cachedWorldMatrix);

    // Material settings
    D3DMATERIAL9 material;
    ZeroMemory(&material, sizeof(material));
    material.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
    material.Ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
    device->SetMaterial(&material);

    // Saving lighting state
    DWORD lightingState;
    device->GetRenderState(D3DRS_LIGHTING, &lightingState);
    device->SetRenderState(D3DRS_LIGHTING, TRUE);

    device->SetStreamSource(0, vb, 0, sizeof(Vertex));
    device->SetFVF(FVF_VERTEX);
    device->SetIndices(ib);
    device->DrawIndexedPrimitive(
        D3DPT_TRIANGLELIST,
        0,
        0,
        mesh->vertices.size(),
        0,
        mesh->indices.size() / 3
    );

    // Restoration of condition
    device->SetTransform(D3DTS_WORLD, &old);
    device->SetRenderState(D3DRS_LIGHTING, lightingState);
}

void MeshRenderer::createInspector(QWidget* parent, QFormLayout* layout) {
    QLabel* label = new QLabel("Mesh Renderer", parent);
    layout->addRow(label);

    QLineEdit* pathField = new QLineEdit(meshPath, parent);
    pathField->setReadOnly(true);
    layout->addRow("Model Path", pathField);

    QPushButton* browse = new QPushButton("Load Model", parent);
    layout->addRow("", browse);

    connect(browse, &QPushButton::clicked, [this, pathField]() {
        QString file = QFileDialog::getOpenFileName(nullptr, "Choose Model", "",
            "Model Files (*.fbx *.obj *.dae *.gltf)");

        if (!file.isEmpty()) {
            setMeshPath(file);
            pathField->setText(file);
            if (getOwner()) {
                emit getOwner()->propertiesChanged();
            }
        }
    });
}

void MeshRenderer::invalidateDeviceObjects()
{
    releaseResources();
    needsRestore = true;
}

bool MeshRenderer::restoreDeviceObjects(LPDIRECT3DDEVICE9 device) {
    if (!needsRestore || !mesh) return true;
    if (mesh->vertices.empty() || mesh->indices.empty()) return false;

    if (!device) {
        ConsolePanel::sError("Cannot restore mesh buffers: invalid device");
        return false;
    }

    releaseResources();

    if (FAILED(device->CreateVertexBuffer(
        mesh->vertices.size() * sizeof(Vertex),
        D3DUSAGE_WRITEONLY,
        FVF_VERTEX,
        D3DPOOL_MANAGED,
        &vb,
        nullptr
    ))) {
        ConsolePanel::sError("Failed to create vertex buffer");
        return false;
    }

    void* ptr = nullptr;
    if (FAILED(vb->Lock(0, 0, &ptr, 0))) {
        ConsolePanel::sError("Failed to lock vertex buffer");
        return false;
    }
    memcpy(ptr, mesh->vertices.data(), mesh->vertices.size() * sizeof(Vertex));
    vb->Unlock();

    if (FAILED(device->CreateIndexBuffer(
        mesh->indices.size() * sizeof(WORD),
        D3DUSAGE_WRITEONLY,
        D3DFMT_INDEX16,
        D3DPOOL_MANAGED,
        &ib,
        nullptr
    ))) {
        ConsolePanel::sError("Failed to create index buffer");
        return false;
    }

    if (FAILED(ib->Lock(0, 0, &ptr, 0))) {
        ConsolePanel::sError("Failed to lock index buffer");
        ib->Release();
        ib = nullptr;
        return false;
    }
    memcpy(ptr, mesh->indices.data(), mesh->indices.size() * sizeof(WORD));
    ib->Unlock();

    needsRestore = false;
    return true;
}

void MeshRenderer::setMeshPath(const QString& path)
{
    if (meshPath == path) return;

    meshPath = path;
    if (loadMeshFromFile(path)) {
        invalidateDeviceObjects();
        if (getOwner()) {
            emit getOwner()->propertiesChanged();
        }
    }
}

bool MeshRenderer::isVisible(const D3DXMATRIX& viewProj) const
{
    if (!mesh || mesh->vertices.empty()) return false;

    D3DXVECTOR3 min = mesh->minBounds;
    D3DXVECTOR3 max = mesh->maxBounds;

    D3DXVECTOR3 corners[8] = {
        D3DXVECTOR3(min.x, min.y, min.z),
        D3DXVECTOR3(max.x, min.y, min.z),
        D3DXVECTOR3(min.x, max.y, min.z),
        D3DXVECTOR3(max.x, max.y, min.z),
        D3DXVECTOR3(min.x, min.y, max.z),
        D3DXVECTOR3(max.x, min.y, max.z),
        D3DXVECTOR3(min.x, max.y, max.z),
        D3DXVECTOR3(max.x, max.y, max.z)
    };

    D3DXMATRIX worldViewProj = cachedWorldMatrix * viewProj;

    for (auto& corner : corners) {
        D3DXVECTOR3 projected;
        D3DXVec3TransformCoord(&projected, &corner, &worldViewProj);

        if (projected.x >= -1.0f && projected.x <= 1.0f &&
            projected.y >= -1.0f && projected.y <= 1.0f &&
            projected.z >= 0.0f && projected.z <= 1.0f) {
            return true;
        }
    }

    return false;
}

bool MeshRenderer::loadMeshFromFile(const QString& path) {
    try {
        mesh = ResourceManager::loadMesh(path);
        return mesh != nullptr;
    }
    catch (const std::exception& e) {
        ConsolePanel::sError(QString("Mesh load error: %1").arg(e.what()));
        QMessageBox::critical(nullptr, "Load Error",
            QString("Failed to load mesh:\n%1").arg(e.what()));
        return false;
    }
}

void MeshRenderer::releaseResources()
{
    if (vb) { vb->Release(); vb = nullptr; }
    if (ib) { ib->Release(); ib = nullptr; }
}

void MeshRenderer::updateWorldMatrix()
{
    if (worldMatrixValid) return;

    auto* tr = getOwner()->getComponent<Transform>();
    if (tr) {
        cachedWorldMatrix = tr->getWorldMatrix();
    }
    else {
        D3DXMatrixIdentity(&cachedWorldMatrix);
    }

    worldMatrixValid = true;
}
