#pragma once

#include <QWidget>
#include <QTimer>
#include <QPoint>
#include <QSet>
#include <d3d9.h>
#include <d3dx9.h>
#include "Skybox.h"
#include "Scene.h"
#include <dragAxis.h>

class Scene;

class Viewport : public QWidget {
    Q_OBJECT

public:
    void setScene(Scene* scene) { this->scene = scene; }
    void setSelectedObject(SceneObject* obj) { selectedObject = obj; }

    explicit Viewport(QWidget* parent = nullptr);
    ~Viewport();

public slots:
    void onObjectSelected(SceneObject* obj);

protected:
    QPaintEngine* paintEngine() const override { return nullptr; }
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    bool initD3D();
    void cleanup();
    void render();
    void updateCamera(float deltaTime);
    void applyCommonRenderStates();
    void BuildPickingRay(const QPoint& mousePos, D3DXVECTOR3& outOrigin, D3DXVECTOR3& outDir);
    float DistanceRayToLine(const D3DXVECTOR3& rayO, const D3DXVECTOR3& rayD, const D3DXVECTOR3& lineP, const D3DXVECTOR3& lineDir);
    D3DXVECTOR3 ProjectPointOnLine(const D3DXVECTOR3& rayO, const D3DXVECTOR3& rayD, const D3DXVECTOR3& lineP, const D3DXVECTOR3& lineDir);

    Scene* scene = nullptr;

    SceneObject* selectedObject = nullptr;
    ID3DXLine* gizmoLine = nullptr;

    LPDIRECT3D9 d3d = nullptr;
    LPDIRECT3DDEVICE9 device = nullptr;
    QTimer* renderTimer = nullptr;

    // Camera
    D3DXVECTOR3 cameraPos;
    D3DXVECTOR3 cameraDir;
    D3DXVECTOR3 cameraUp;
    float yaw = 0.0f, pitch = 0.0f;

    // Input
    bool rightMouseHeld = false;
    QPoint lastMousePos;
    QSet<int> pressedKeys;

    // Gizmo settings
    DragAxis dragging = DragAxis::None;
    D3DXVECTOR3 dragStartWorld;
    D3DXVECTOR3 dragAxisDir;
    D3DXVECTOR3 objStartPos;
};