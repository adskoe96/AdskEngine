#include "MeshRenderer.h"
#include "Transform.h"
#include "ConsolePanel.h"

#include <d3d9types.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <QPushButton>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>


void MeshRenderer::render(LPDIRECT3DDEVICE9 device) {
    if (needsRestore) {
        if (!restoreDeviceObjects(device)) {
            return;
        }
    }

    if (!vb || !ib) {
        ConsolePanel::sError("Invalid mesh buffers");
        return;
    }

    auto* tr = getOwner()->getComponent<Transform>();
    D3DXMATRIX world = tr ? tr->getWorldMatrix() : D3DXMATRIX();
    D3DXMATRIX old;
    device->GetTransform(D3DTS_WORLD, &old);
    device->SetTransform(D3DTS_WORLD, &world);

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
        vertices.size(),
        0,
        indices.size() / 3
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
        QString file = QFileDialog::getOpenFileName(nullptr, "Choose Model", "", "Model Files (*.fbx *.obj)");
        if (!file.isEmpty()) {
            setMeshPath(file);
            pathField->setText(file);
            if (getOwner()) {
                emit getOwner()->propertiesChanged();
            }
        }
    });
}

void MeshRenderer::invalidate() {
    if (vb) { vb->Release(); vb = nullptr; }
    if (ib) { ib->Release(); ib = nullptr; }
    needsRestore = true;
}

bool MeshRenderer::restoreDeviceObjects(LPDIRECT3DDEVICE9 device) {
    if (!needsRestore) return true;
    if (vertices.empty() || indices.empty()) return false;

    // Добавляем проверку на валидность устройства
    if (!device) {
        ConsolePanel::sError("Cannot restore mesh buffers: invalid device");
        return false;
    }

    // Освобождаем старые буферы, если они есть
    if (vb) { vb->Release(); vb = nullptr; }
    if (ib) { ib->Release(); ib = nullptr; }

    if (FAILED(device->CreateVertexBuffer(vertices.size() * sizeof(Vertex),
        0, FVF_VERTEX, D3DPOOL_MANAGED, &vb, nullptr))) {
        ConsolePanel::sError("Failed to create vertex buffer");
        return false;
    }

    void* ptr = nullptr;
    if (FAILED(vb->Lock(0, 0, &ptr, 0))) {
        ConsolePanel::sError("Failed to lock vertex buffer");
        return false;
    }
    memcpy(ptr, vertices.data(), vertices.size() * sizeof(Vertex));
    vb->Unlock();

    if (FAILED(device->CreateIndexBuffer(indices.size() * sizeof(WORD),
        0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &ib, nullptr))) {
        ConsolePanel::sError("Failed to create index buffer");
        return false;
    }

    if (FAILED(ib->Lock(0, 0, &ptr, 0))) {
        ConsolePanel::sError("Failed to lock index buffer");
        ib->Release();
        ib = nullptr;
        return false;
    }
    memcpy(ptr, indices.data(), indices.size() * sizeof(WORD));
    ib->Unlock();

    needsRestore = false;
    return true;
}

void MeshRenderer::setMeshPath(const QString& path)
{
    meshPath = path;
    if (loadMeshFromFile(path)) {
        invalidate();
        if (getOwner()) {
            emit getOwner()->propertiesChanged();
        }
    }
}

bool MeshRenderer::loadMeshFromFile(const QString& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path.toStdString(),
        aiProcess_Triangulate |
        aiProcess_GenNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_ConvertToLeftHanded);

    if (!scene) {
        ConsolePanel::sError(QString("Mesh load error: %1").arg(importer.GetErrorString()));
        return false;
    }

    if (!scene->HasMeshes()) {
        ConsolePanel::sError("No meshes found in the file");
        return false;
    }

    vertices.clear();
    indices.clear();

    // Automatic scale
    float maxExtent = 0.0f;
    std::vector<aiVector3D> tempVertices;

    for (unsigned m = 0; m < scene->mNumMeshes; ++m) {
        aiMesh* mesh = scene->mMeshes[m];
        for (unsigned i = 0; i < mesh->mNumVertices; ++i) {
            const aiVector3D& v = mesh->mVertices[i];

            // Calculate absolute values of coordinates
            float absX = (v.x < 0) ? -v.x : v.x;
            float absY = (v.y < 0) ? -v.y : v.y;
            float absZ = (v.z < 0) ? -v.z : v.z;

            // Updating the maximum size
            if (absX > maxExtent) maxExtent = absX;
            if (absY > maxExtent) maxExtent = absY;
            if (absZ > maxExtent) maxExtent = absZ;
        }
    }

    // Calculate scale
    if (maxExtent < 0.0001f) maxExtent = 1.0f;
    const float targetScale = 1.0f / maxExtent;
    if (auto* tr = getOwner()->getComponent<Transform>()) {
        tr->scale = { targetScale, targetScale, targetScale };
    }

    // Filling vertices with scaling and Y inversion
    for (unsigned m = 0; m < scene->mNumMeshes; ++m) {
        aiMesh* mesh = scene->mMeshes[m];
        unsigned indexOffset = vertices.size();

        for (unsigned i = 0; i < mesh->mNumVertices; ++i) {
            Vertex v{};
            v.x = mesh->mVertices[i].x * targetScale;
            v.y = -mesh->mVertices[i].y * targetScale; // Y inversion
            v.z = mesh->mVertices[i].z * targetScale;

            if (mesh->HasNormals()) {
                v.nx = mesh->mNormals[i].x;
                v.ny = -mesh->mNormals[i].y; // Inversion of normal
                v.nz = mesh->mNormals[i].z;
            }

            v.color = D3DCOLOR_XRGB(255, 255, 255);
            vertices.push_back(v);
        }

        for (unsigned i = 0; i < mesh->mNumFaces; ++i) {
            const aiFace& face = mesh->mFaces[i];
            if (face.mNumIndices == 3) {
                indices.push_back(indexOffset + face.mIndices[0]);
                indices.push_back(indexOffset + face.mIndices[1]);
                indices.push_back(indexOffset + face.mIndices[2]);
            }
        }
    }

    invalidate();
    return true;
}
