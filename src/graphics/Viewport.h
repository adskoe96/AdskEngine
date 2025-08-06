#pragma once

#include "Skybox.h"
#include "Scene.h"
#include "dragAxis.h"
#include <QWidget>
#include <QTimer>
#include <QPoint>
#include <QSet>
#include <QElapsedTimer>
#include <d3d9.h>
#include <d3dx9.h>

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

    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

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
    void syncYawPitchWithCameraDir();
    void applyCommonRenderStates();
    void BuildPickingRay(const QPoint& mousePos, D3DXVECTOR3& outOrigin, D3DXVECTOR3& outDir);
    void drawGizmoArrow(const D3DXVECTOR3& start, const D3DXVECTOR3& direction, D3DCOLOR color, float length);
    void drawGizmo();
    static QPoint projectToScreen(const D3DXVECTOR3& p, IDirect3DDevice9* dev);

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
    bool cameraInitialized = false;

    // Input
    QPoint lastMouseMovePos;
    QPoint lastGlobalMousePos;
    QSet<int> pressedKeys;
    QElapsedTimer elapsedTimer;
    QPoint mouseCenterPos;
    QPoint mouseDeltaAccum;
    bool rightMouseHeld = false;
    bool cursorLocked = false;
    bool ignoreNextMouseMove = false;
	bool mouseReadyForCamera = false;
    bool ignoreFirstMouseMove = false;

    // Gizmo settings
    DragAxis dragging = DragAxis::None;
    D3DXVECTOR3 dragStartWorld;
    D3DXVECTOR3 dragAxisDir;
    D3DXVECTOR3 objStartPos;
    float GIZMO_LENGTH = 1.0f;
    float GIZMO_PICK_THRESHOLD = 6.0f;
};