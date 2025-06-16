#include "Viewport.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <algorithm>
#include <QDebug>
#include <ConsolePanel.h>

Viewport::Viewport(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoFillBackground(false);

    setFocusPolicy(Qt::StrongFocus);
    setFocus();

    renderTimer = new QTimer(this);
    connect(renderTimer, &QTimer::timeout, this, &Viewport::render);

    // Initial camera parameters
    cameraPos = { 0,0,-5 };
    cameraDir = { 0,0,1 };
    cameraUp = { 0,1,0 };
}

Viewport::~Viewport() {
    cleanup();
}

void Viewport::onObjectSelected(SceneObject* obj) {
    selectedObject = obj;
}

bool Viewport::initD3D() {
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d) return false;

    D3DPRESENT_PARAMETERS d3dpp = {};
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    d3dpp.BackBufferWidth = width();
    d3dpp.BackBufferHeight = height();
    d3dpp.hDeviceWindow = reinterpret_cast<HWND>(winId());

    if (gizmoLine) gizmoLine->Release();
    D3DXCreateLine(device, &gizmoLine);

    if (FAILED(d3d->CreateDevice(
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        reinterpret_cast<HWND>(winId()),
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &d3dpp,
        &device
    ))) {
        qDebug() << "Failed to create D3D device";
        return false;
    }

    if (cameraInitialized) {
        D3DXMATRIX view;
        D3DXVECTOR3 lookAt = cameraPos + cameraDir;
        D3DXMatrixLookAtLH(&view, &cameraPos, &lookAt, &cameraUp);
        device->SetTransform(D3DTS_VIEW, &view);
    }
    else {
        // Initial camera parameters
        cameraPos = { 0,0,-5 };
        cameraDir = { 0,0,1 };
        cameraUp = { 0,1,0 };

        D3DXMATRIX view;
        D3DXVECTOR3 lookAt = cameraPos + cameraDir;
        D3DXMatrixLookAtLH(&view, &cameraPos, &lookAt, &cameraUp);
        device->SetTransform(D3DTS_VIEW, &view);

        cameraInitialized = true;
    }

    // Setup render states
    device->SetRenderState(D3DRS_ZENABLE, TRUE);
    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    device->SetRenderState(D3DRS_LIGHTING, scene->getLightingEnabled() ? TRUE : FALSE);

    // Projection matrix
    float aspect = width() / static_cast<float>(height());
    D3DXMATRIX proj;
    D3DXMatrixPerspectiveFovLH(&proj, D3DXToRadian(90.0f), aspect, 0.1f, 100.0f);
    device->SetTransform(D3DTS_PROJECTION, &proj);

    // Initial view matrix
    D3DXMATRIX view;
    D3DXVECTOR3 lookAt = cameraPos + cameraDir;
    D3DXMatrixLookAtLH(&view, &cameraPos, &lookAt, &cameraUp);
    device->SetTransform(D3DTS_VIEW, &view);

    // Initialize and restore scene objects and skybox
    if (scene) {
        scene->restoreDeviceObjects(device);
        scene->updateSkybox(device);

        // Ambient и lighting
        D3DCOLORVALUE ambient = scene->getAmbientColor();
        device->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_COLORVALUE(
            ambient.r, ambient.g, ambient.b, ambient.a));
        device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

        scene->clearLightingDirty();
        scene->clearSkyboxDirty();
    }

    if (FAILED(D3DXCreateLine(device, &gizmoLine))) {
        ConsolePanel::sLog(LogType::Error, "Failed to create D3DX line");
    }

    D3DXCreateLine(device, &gizmoLine);

    return true;
}

void Viewport::cleanup() {
    if (device) { device->Release(); device = nullptr; }
    if (d3d) { d3d->Release(); d3d = nullptr; }
    if (gizmoLine) { gizmoLine->Release(); gizmoLine = nullptr; }
}

void Viewport::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    setFocus();
    if (initD3D()) renderTimer->start(16);
}

void Viewport::paintEvent(QPaintEvent*) {
    // No Qt painting
}

void Viewport::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (!device) return;

    bool wasCameraInitialized = cameraInitialized;
    D3DXVECTOR3 savedCameraPos = cameraPos;
    D3DXVECTOR3 savedCameraDir = cameraDir;
    D3DXVECTOR3 savedCameraUp = cameraUp;
    float savedYaw = yaw;
    float savedPitch = pitch;

    if (gizmoLine) {
        gizmoLine->Release();
        gizmoLine = nullptr;
    }

    D3DPRESENT_PARAMETERS d3dpp = {};
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    d3dpp.BackBufferWidth = event->size().width();
    d3dpp.BackBufferHeight = event->size().height();
    d3dpp.hDeviceWindow = (HWND)winId();

    HRESULT hr = device->Reset(&d3dpp);
    if (FAILED(hr)) {
        ConsolePanel::sLog(LogType::Error, "Device reset failed!");
        renderTimer->stop();
        cleanup();
        return;
    }

    if (wasCameraInitialized) {
        cameraPos = savedCameraPos;
        cameraDir = savedCameraDir;
        cameraUp = savedCameraUp;
        yaw = savedYaw;
        pitch = savedPitch;

        D3DXMATRIX view;
        D3DXVECTOR3 lookAt = cameraPos + cameraDir;
        D3DXMatrixLookAtLH(&view, &cameraPos, &lookAt, &cameraUp);
        device->SetTransform(D3DTS_VIEW, &view);
    }

    applyCommonRenderStates();
    if (scene) {
        scene->restoreDeviceObjects(device);
        scene->updateSkybox(device);
    }

    if (FAILED(D3DXCreateLine(device, &gizmoLine))) {
        ConsolePanel::sLog(LogType::Error, "Failed to recreate D3DX line after resize");
    }
    D3DXCreateLine(device, &gizmoLine);
}

void Viewport::keyPressEvent(QKeyEvent* ev) { pressedKeys.insert(ev->key()); }

void Viewport::keyReleaseEvent(QKeyEvent* ev) { pressedKeys.remove(ev->key()); }

void Viewport::mousePressEvent(QMouseEvent* ev) {
    if (ev->button() == Qt::LeftButton && selectedObject) {
        D3DXVECTOR3 rayO, rayD;
        BuildPickingRay(ev->pos(), rayO, rayD);

        D3DXVECTOR3 origin(selectedObject->getPositionX(),
            selectedObject->getPositionY(),
            selectedObject->getPositionZ());
        struct AxisLine { D3DXVECTOR3 dir; DragAxis axis; };
        AxisLine axes[3] = {
            {{1,0,0}, DragAxis::X},
            {{0,1,0}, DragAxis::Y},
            {{0,0,1}, DragAxis::Z}
        };
        const float pickThresh = 0.1f;
        for (auto& ax : axes) {
            float dist = DistanceRayToLine(rayO, rayD, origin, ax.dir);
            if (dist < pickThresh) {
                dragging = ax.axis;
                dragAxisDir = ax.dir;
                objStartPos = origin;
                dragStartWorld = ProjectPointOnLine(rayO, rayD, origin, ax.dir);
                setCursor(Qt::SizeAllCursor);
                return;
            }
        }
    }
    if (ev->button() == Qt::RightButton) {
        rightMouseHeld = true; lastMousePos = ev->pos(); setCursor(Qt::BlankCursor);
        grabMouse(); setFocus();
    }
}

void Viewport::mouseReleaseEvent(QMouseEvent* ev) {
    if (dragging != DragAxis::None) {
        dragging = DragAxis::None;
        setCursor(Qt::ArrowCursor);
        return;
    }
    if (ev->button() == Qt::RightButton) {
        rightMouseHeld = false; releaseMouse(); setCursor(Qt::ArrowCursor);
        return;
    }
}

void Viewport::mouseMoveEvent(QMouseEvent* ev) {
    if (dragging != DragAxis::None && selectedObject) {
        D3DXVECTOR3 rayO, rayD;
        BuildPickingRay(ev->pos(), rayO, rayD);
        D3DXVECTOR3 currentOnAxis = ProjectPointOnLine(rayO, rayD, objStartPos, dragAxisDir);
        D3DXVECTOR3 delta = currentOnAxis - dragStartWorld;
        D3DXVECTOR3 newPos = objStartPos + delta;
        selectedObject->setPositionX(newPos.x);
        selectedObject->setPositionY(newPos.y);
        selectedObject->setPositionZ(newPos.z);
        return;
    }
    if (rightMouseHeld) {
        // existing camera rotation code
        QPoint delta = ev->pos() - lastMousePos;
        lastMousePos = ev->pos(); float s = 0.002f;
        yaw += delta.x() * s; pitch -= delta.y() * s;
        pitch = std::clamp(pitch, -1.5f, 1.5f);
        D3DXVECTOR3 front{ cosf(pitch) * sinf(yaw), sinf(pitch), cosf(pitch) * cosf(yaw) };
        D3DXVec3Normalize(&cameraDir, &front);
        return;
    }
    if (dragging == DragAxis::None && !rightMouseHeld) {
        setFocus();
    }
    QWidget::mouseMoveEvent(ev);
}

void Viewport::updateCamera(float deltaTime) {
    D3DXVECTOR3 right;
    D3DXVec3Cross(&right, &cameraDir, &cameraUp);
    D3DXVec3Normalize(&right, &right);
    float speed = 5.0f * deltaTime;
    if (pressedKeys.contains(Qt::Key_W)) {
        cameraPos += cameraDir * speed;
    }
    if (pressedKeys.contains(Qt::Key_S)) cameraPos -= cameraDir * speed;
    if (pressedKeys.contains(Qt::Key_A)) cameraPos += right * speed;
    if (pressedKeys.contains(Qt::Key_D)) cameraPos -= right * speed;

    D3DXMATRIX view;
    D3DXVECTOR3 look = cameraPos + cameraDir;
    D3DXMatrixLookAtLH(&view, &cameraPos, &look, &cameraUp);
    device->SetTransform(D3DTS_VIEW, &view);
}

void Viewport::applyCommonRenderStates() {
    device->SetRenderState(D3DRS_ZENABLE, TRUE);
    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    device->SetRenderState(D3DRS_LIGHTING, scene->getLightingEnabled() ? TRUE : FALSE);

    // Color of ambient light
    D3DCOLORVALUE ambient = scene->getAmbientColor();
    device->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_COLORVALUE(
        ambient.r, ambient.g, ambient.b, ambient.a));

    // Projection
    float aspect = width() / float(height());
    D3DXMATRIX proj;
    D3DXMatrixPerspectiveFovLH(&proj, D3DXToRadian(90), aspect, 0.1f, 100.0f);
    device->SetTransform(D3DTS_PROJECTION, &proj);
}

void Viewport::BuildPickingRay(const QPoint& mousePos, D3DXVECTOR3& outOrigin, D3DXVECTOR3& outDir)
{
    D3DVIEWPORT9 vp; device->GetViewport(&vp);
    D3DXMATRIX view, proj, world;
    device->GetTransform(D3DTS_VIEW, &view);
    device->GetTransform(D3DTS_PROJECTION, &proj);
    D3DXMatrixIdentity(&world);

    D3DXVECTOR3 pN, pF;
    D3DXVec3Unproject(&pN,
        &D3DXVECTOR3(mousePos.x(), mousePos.y(), 0.0f),
        &vp, &proj, &view, &world);
    D3DXVec3Unproject(&pF,
        &D3DXVECTOR3(mousePos.x(), mousePos.y(), 1.0f),
        &vp, &proj, &view, &world);

    outOrigin = pN;
    D3DXVec3Normalize(&outDir, &(pF - pN));
}

float Viewport::DistanceRayToLine(const D3DXVECTOR3& rayO, const D3DXVECTOR3& rayD, const D3DXVECTOR3& lineP, const D3DXVECTOR3& lineDir)
{
    D3DXVECTOR3 cross;
    D3DXVec3Cross(&cross, &rayD, &lineDir);
    float denom = D3DXVec3Length(&cross);
    if (denom < 1e-6f) return FLT_MAX;
    D3DXVECTOR3 diff = lineP - rayO;
    return fabsf(D3DXVec3Dot(&diff, &cross)) / denom;
}

D3DXVECTOR3 Viewport::ProjectPointOnLine(const D3DXVECTOR3& rayO, const D3DXVECTOR3& rayD, const D3DXVECTOR3& lineP, const D3DXVECTOR3& lineDir)
{
    float t = D3DXVec3Dot(&(lineP - rayO), &rayD);
    D3DXVECTOR3 p = rayO + rayD * t;
    float u = D3DXVec3Dot(&(p - lineP), &lineDir);
    return lineP + lineDir * u;
}

void Viewport::render() {
    if (!device || !scene) return;

    // Обновляем Skybox при необходимости
    if (scene->isSkyboxDirty()) {
        scene->updateSkybox(device);
        scene->clearSkyboxDirty();
    }

    // Обновляем настройки освещения
    if (scene->isLightingDirty()) {
        device->SetRenderState(D3DRS_LIGHTING, scene->getLightingEnabled() ? TRUE : FALSE);

        D3DCOLORVALUE ambient = scene->getAmbientColor();
        device->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_COLORVALUE(
            ambient.r, ambient.g, ambient.b, ambient.a));

        scene->clearLightingDirty();
    }

    HRESULT hr = device->TestCooperativeLevel();
    if (hr == D3DERR_DEVICELOST) {
        return;
    }
    else if (hr == D3DERR_DEVICENOTRESET) {
        cleanup();
        if (!initD3D()) {
            renderTimer->stop();
            return;
        }
    }

    static QTime time = QTime::currentTime();
    int msecs = time.elapsed();
    time.restart();
    float deltaTime = msecs / 1000.0f;

    updateCamera(deltaTime);

    device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
        D3DCOLOR_XRGB(30, 30, 30), 1.0f, 0);

    if (SUCCEEDED(device->BeginScene())) {
        if (scene->getSkybox()) {
            D3DXMATRIX savedView, savedProj;
            device->GetTransform(D3DTS_VIEW, &savedView);
            device->GetTransform(D3DTS_PROJECTION, &savedProj);

            DWORD zEnable, cullMode;
            device->GetRenderState(D3DRS_ZENABLE, &zEnable);
            device->GetRenderState(D3DRS_CULLMODE, &cullMode);

            device->SetRenderState(D3DRS_ZENABLE, FALSE);
            device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

            D3DXMATRIX skyboxView;
            D3DXVECTOR3 eye(0, 0, 0);
            D3DXVECTOR3 at = eye + cameraDir;
            D3DXMatrixLookAtLH(&skyboxView, &eye, &at, &cameraUp);
            device->SetTransform(D3DTS_VIEW, &skyboxView);

            float aspect = width() / static_cast<float>(height());
            D3DXMATRIX skyboxProj;
            D3DXMatrixPerspectiveFovLH(&skyboxProj, D3DXToRadian(90), aspect, 0.1f, 100.0f);
            device->SetTransform(D3DTS_PROJECTION, &skyboxProj);

            scene->getSkybox()->draw(device);

            device->SetTransform(D3DTS_VIEW, &savedView);
            device->SetTransform(D3DTS_PROJECTION, &savedProj);
            device->SetRenderState(D3DRS_ZENABLE, zEnable);
            device->SetRenderState(D3DRS_CULLMODE, cullMode);
        }
        for (const auto& obj : scene->getObjects()) {
            obj->render(device);
        }

        if (selectedObject && gizmoLine) {
            D3DXMATRIX identity;
            D3DXMatrixIdentity(&identity);
            device->SetTransform(D3DTS_WORLD, &identity);

            // 2) параметры проекции и view
            D3DVIEWPORT9 vp;
            device->GetViewport(&vp);
            D3DXMATRIX matView, matProj;
            device->GetTransform(D3DTS_VIEW, &matView);
            device->GetTransform(D3DTS_PROJECTION, &matProj);

            // 3) собираем шесть 3D‑точек (начало и конец трёх осей длины 1.0)
            D3DXVECTOR3 origin(
                selectedObject->getPositionX(),
                selectedObject->getPositionY(),
                selectedObject->getPositionZ()
            );
            D3DXVECTOR3 points3D[6] = {
                origin, origin + D3DXVECTOR3(1,0,0),  // X
                origin, origin + D3DXVECTOR3(0,1,0),  // Y
                origin, origin + D3DXVECTOR3(0,0,1)   // Z
            };

            D3DXVECTOR2 points2D[6];
            for (int i = 0; i < 6; ++i) {
                D3DXVECTOR3 proj;
                D3DXVec3Project(
                    &proj, // <- Исправлено: передается реальная переменная
                    &points3D[i],
                    &vp,
                    &matProj,
                    &matView,
                    &identity
                );
                points2D[i] = D3DXVECTOR2(proj.x, proj.y);
            }
            
            gizmoLine->Begin();
            gizmoLine->Draw(&points2D[0], 2, D3DCOLOR_XRGB(255, 0, 0));
            gizmoLine->Draw(&points2D[2], 2, D3DCOLOR_XRGB(0, 255, 0));
            gizmoLine->Draw(&points2D[4], 2, D3DCOLOR_XRGB(0, 0, 255));
            gizmoLine->End();
        }
        device->EndScene();
    }

    device->Present(nullptr, nullptr, nullptr, nullptr);
}