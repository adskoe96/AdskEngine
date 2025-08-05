#include "Viewport.h"
#include "ConsolePanel.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <algorithm>
#include <QElapsedTimer>
#include <QtMath>
#include <cmath>

Viewport::Viewport(QWidget* parent)
    : QWidget(parent)
{
    lastMouseMovePos = QPoint(width() / 2, height() / 2);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoFillBackground(false);

    setFocus();

    renderTimer = new QTimer(this);
    connect(renderTimer, &QTimer::timeout, this, &Viewport::render);

    // Initial camera parameters
    cameraPos = { 0,0,-5 };
    cameraDir = { 0,0,1 };
    cameraUp = { 0,1,0 };

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    elapsedTimer.start();
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
        ConsolePanel::sError("Failed to create D3D device");
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
    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
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

void Viewport::enterEvent(QEvent* event)
{
    setFocus();
}

void Viewport::leaveEvent(QEvent* event)
{
    clearFocus();
}

void Viewport::focusOutEvent(QFocusEvent* event)
{
    pressedKeys.clear();
    if (rightMouseHeld) {
        rightMouseHeld = false;
        cursorLocked = false;
        releaseMouse();
        releaseKeyboard();
        setCursor(Qt::ArrowCursor);
    }
    QWidget::focusOutEvent(event);
}

void Viewport::paintEvent(QPaintEvent*) {
    // No Qt painting
}

void Viewport::resizeEvent(QResizeEvent* event) {
    mouseCenterPos = QPoint(width() / 2, height() / 2);
    QWidget::resizeEvent(event);
    if (device) {
        applyCommonRenderStates();
    }
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
        auto* tr = selectedObject->getComponent<Transform>();
        if (!tr) return;

        const auto& position = tr->getPosition();

        struct Axe { D3DXVECTOR3 dir; DragAxis axis; };
        Axe axes[3] = {
            {{1,0,0}, DragAxis::X},
            {{0,1,0}, DragAxis::Y},
            {{0,0,1}, DragAxis::Z}
        };
        QPoint mouse = ev->pos();
        float bestDist = GIZMO_PICK_THRESHOLD;
        DragAxis bestAxis = DragAxis::None;
        D3DXVECTOR3 bestDir;

        for (auto& ax : axes) {
            D3DXVECTOR3 p0 = position;
            D3DXVECTOR3 p1 = position + ax.dir * GIZMO_LENGTH;
            QPoint s0 = projectToScreen(p0, device);
            QPoint s1 = projectToScreen(p1, device);

            float t;
            QPointF diff = s1 - s0;
            float len2 = diff.x() * diff.x() + diff.y() * diff.y();
            if (len2 < 1e-6f) continue;
            t = ((mouse.x() - s0.x()) * diff.x() + (mouse.y() - s0.y()) * diff.y()) / len2;
            t = std::clamp(t, 0.0f, 1.0f);
            QPointF proj = s0 + diff * t;
            float dist = std::hypot(mouse.x() - proj.x(), mouse.y() - proj.y());
            if (dist < bestDist) {
                bestDist = dist;
                bestAxis = ax.axis;
                bestDir = ax.dir;
            }
        }

        if (bestAxis != DragAxis::None) {
            dragging = bestAxis;
            dragAxisDir = bestDir;
            objStartPos = position;
            D3DXVECTOR3 rayO, rayD;
            BuildPickingRay(ev->pos(), rayO, rayD);
            dragStartWorld = ProjectPointOnLine(rayO, rayD, objStartPos, dragAxisDir);
            setCursor(Qt::SizeAllCursor);
            return;
        }
    }

    if (ev->button() == Qt::RightButton) {
        rightMouseHeld = true;
        cursorLocked = true;
        mouseCenterPos = QPoint(width() / 2, height() / 2);
        setCursor(Qt::CrossCursor);
        setFocus();
        syncYawPitchWithCameraDir();
        lastGlobalMousePos = ev->pos();
        lastMouseMovePos = ev->pos();
        ignoreFirstMouseMove = true;
    }
}

void Viewport::mouseReleaseEvent(QMouseEvent* ev) {
    if (dragging != DragAxis::None) {
        dragging = DragAxis::None;
        setCursor(Qt::ArrowCursor);
        return;
    }
    if (ev->button() == Qt::RightButton) {
        rightMouseHeld = false;
        cursorLocked = false;
        releaseMouse();
        releaseKeyboard();
        setCursor(Qt::ArrowCursor);
        return;
    }

    releaseMouse();
}

void Viewport::mouseMoveEvent(QMouseEvent* ev) {
    if (dragging != DragAxis::None && selectedObject) {
        D3DXVECTOR3 rayO, rayD;
        BuildPickingRay(ev->pos(), rayO, rayD);
        D3DXVECTOR3 currentProj = ProjectPointOnLine(rayO, rayD, objStartPos, dragAxisDir);
        D3DXVECTOR3 delta = currentProj - dragStartWorld;
        float offset = D3DXVec3Dot(&delta, &dragAxisDir);
        D3DXVECTOR3 newPos = objStartPos + dragAxisDir * offset;
        auto* tr = selectedObject->getComponent<Transform>();
        if (tr) {
            tr->setPosition(newPos);
        }
    }

    if (!rightMouseHeld)
    {
        return QWidget::mouseMoveEvent(ev);
    }
        

    QPoint globalCenter = mapToGlobal(mouseCenterPos);

    if (ignoreNextMouseMove) {
        ignoreNextMouseMove = false;
        return;
    }

    QPoint delta = ev->globalPos() - globalCenter;

    const float sensitivity = 0.002f;
    yaw += delta.x() * sensitivity;
    pitch -= delta.y() * sensitivity;
    pitch = std::clamp(pitch, -D3DX_PI / 2 + 0.01f, D3DX_PI / 2 - 0.01f);

    D3DXVECTOR3 forward;
    forward.x = std::sin(yaw) * std::cos(pitch);
    forward.y = std::sin(pitch);
    forward.z = std::cos(yaw) * std::cos(pitch);
    D3DXVec3Normalize(&cameraDir, &forward);

    D3DXMATRIX view;
    D3DXVECTOR3 lookAt = cameraPos + cameraDir;
    D3DXMatrixLookAtLH(&view, &cameraPos, &lookAt, &cameraUp);
    device->SetTransform(D3DTS_VIEW, &view);

    QCursor::setPos(globalCenter);
    ignoreNextMouseMove = true;
}


void Viewport::updateCamera(float deltaTime) {
    D3DXVECTOR3 right;
    D3DXVec3Cross(&right, &cameraUp, &cameraDir);
    D3DXVec3Normalize(&right, &right);

    const float moveSpeed = 10.0f * deltaTime;
    if (pressedKeys.contains(Qt::Key_W)) {
        cameraPos += cameraDir * moveSpeed;
    }
    if (pressedKeys.contains(Qt::Key_S)) {
        cameraPos -= cameraDir * moveSpeed;
    }
    if (pressedKeys.contains(Qt::Key_A)) {
        cameraPos -= right * moveSpeed;
    }
    if (pressedKeys.contains(Qt::Key_D)) {
        cameraPos += right * moveSpeed;
    }
    if (pressedKeys.contains(Qt::Key_Q)) {
        cameraPos -= cameraUp * moveSpeed;
    }
    if (pressedKeys.contains(Qt::Key_E)) {
        cameraPos += cameraUp * moveSpeed;
    }

    // Update view matrix
    D3DXMATRIX view;
    D3DXVECTOR3 lookAt = cameraPos + cameraDir;
    D3DXMatrixLookAtLH(&view, &cameraPos, &lookAt, &cameraUp);
    device->SetTransform(D3DTS_VIEW, &view);
}

void Viewport::syncYawPitchWithCameraDir()
{
    D3DXVec3Normalize(&cameraDir, &cameraDir);
    pitch = asinf(cameraDir.y);
    yaw = atan2f(cameraDir.x, cameraDir.z);
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

void Viewport::drawGizmoArrow(const D3DXVECTOR3& start, const D3DXVECTOR3& direction, D3DCOLOR color, float length)
{
    if (!gizmoLine) return;

    D3DXMATRIX view, proj, world;
    D3DVIEWPORT9 vp;
    device->GetTransform(D3DTS_VIEW, &view);
    device->GetTransform(D3DTS_PROJECTION, &proj);
    D3DXMatrixIdentity(&world);
    device->GetViewport(&vp);

    D3DXVECTOR3 end = start + direction * length;
    
    D3DXVECTOR3 screenStart, screenEnd;
    D3DXVec3Project(&screenStart, &start, &vp, &proj, &view, &world);
    D3DXVec3Project(&screenEnd, &end, &vp, &proj, &view, &world);

    D3DXVECTOR2 linePoints[] = {
        D3DXVECTOR2(screenStart.x, screenStart.y),
        D3DXVECTOR2(screenEnd.x, screenEnd.y)
    };
    gizmoLine->Draw(linePoints, 2, color);

    const float coneLength = length * 0.2f;
    const float coneRadius = coneLength * 0.4f;

    D3DXVECTOR3 coneBase = end - direction * coneLength;
    D3DXVECTOR3 perp1, perp2;
    D3DXVec3Cross(&perp1, &direction, &cameraUp);
    D3DXVec3Normalize(&perp1, &perp1);
    D3DXVec3Cross(&perp2, &direction, &perp1);
    D3DXVec3Normalize(&perp2, &perp2);

    D3DXVECTOR3 conePoints3D[5] = {
        end,
        coneBase + perp1 * coneRadius,
        coneBase + perp2 * coneRadius,
        coneBase - perp1 * coneRadius,
        coneBase - perp2 * coneRadius
    };

    D3DXVECTOR2 conePoints2D[5];
    for (int i = 0; i < 5; i++) {
        D3DXVECTOR3 screenPoint;
        D3DXVec3Project(&screenPoint, &conePoints3D[i], &vp, &proj, &view, &world);
        conePoints2D[i] = D3DXVECTOR2(screenPoint.x, screenPoint.y);
    }

    gizmoLine->Draw(conePoints2D, 5, color);
}

void Viewport::drawGizmo()
{
    if (!selectedObject || !gizmoLine) return;

    auto* tr = selectedObject->getComponent<Transform>();
    if (!tr) return;

    const auto& position = tr->getPosition();

    QPoint center = projectToScreen(position, device);

    D3DXVECTOR3 origin = position;
    struct AxeDraw { D3DXVECTOR3 dir; D3DCOLOR color; };
    AxeDraw axes[3] = {
        {{1,0,0}, D3DCOLOR_XRGB(255, 0, 0)},
        {{0,1,0}, D3DCOLOR_XRGB(0, 255, 0)},
        {{0,0,1}, D3DCOLOR_XRGB(0, 0, 255)}
    };

    struct DepthAxis { float depth; AxeDraw ad; };
    std::vector<DepthAxis> list;
    for (auto& a : axes) {
        D3DXVECTOR3 mid = origin + a.dir * (GIZMO_LENGTH * 0.5f);
        D3DXVECTOR3 viewPt;
        D3DXMATRIX view; device->GetTransform(D3DTS_VIEW, &view);
        D3DXVec3TransformCoord(&viewPt, &mid, &view);
        list.push_back({ viewPt.z, a });
    }
    std::sort(list.begin(), list.end(),
        [](auto& a, auto& b) { return a.depth < b.depth; });

    for (int i = 0; i < 3; ++i) {
        float width = (i == 0 ? 3.0f : 1.5f);
        gizmoLine->SetWidth(width);
        D3DXVECTOR3 p0 = origin;
        D3DXVECTOR3 p1 = origin + list[i].ad.dir * GIZMO_LENGTH;
        QPoint s0 = projectToScreen(p0, device);
        QPoint s1 = projectToScreen(p1, device);
        D3DXVECTOR2 pts[2] = {
            D3DXVECTOR2(s0.x(), s0.y()),
            D3DXVECTOR2(s1.x(), s1.y())
        };
        gizmoLine->Draw(pts, 2, list[i].ad.color);
    }
}

QPoint Viewport::projectToScreen(const D3DXVECTOR3& p, IDirect3DDevice9* dev)
{
    D3DVIEWPORT9 vp; dev->GetViewport(&vp);
    D3DXMATRIX view, proj, world;
    dev->GetTransform(D3DTS_VIEW, &view);
    dev->GetTransform(D3DTS_PROJECTION, &proj);
    D3DXMatrixIdentity(&world);

    D3DXVECTOR3 sp;
    D3DXVec3Project(&sp, &p, &vp, &proj, &view, &world);
    return QPoint(int(sp.x), int(sp.y));
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

    if (scene->isSkyboxDirty()) {
        scene->updateSkybox(device);
        scene->clearSkyboxDirty();
    }

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

    static qint64 lastTime = 0;
    qint64 currentTime = elapsedTimer.elapsed();
    float deltaTime = lastTime > 0 ? (currentTime - lastTime) / 1000.0f : 0.016f;
    lastTime = currentTime;

    if (deltaTime > 0.1f) deltaTime = 0.016f;

    updateCamera(deltaTime);
    device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(30, 30, 30), 1.0f, 0);

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

            device->SetRenderState(D3DRS_ZENABLE, TRUE);
            device->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

            drawGizmo();
        }

        device->EndScene();
    }

    device->Present(nullptr, nullptr, nullptr, nullptr);
}