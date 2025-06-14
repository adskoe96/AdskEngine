#include "Viewport.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <algorithm>
#include <QDebug>

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
        scene->initializeSkybox(device);
        scene->restoreDeviceObjects(device);
        // Ambient and lighting
        D3DCOLORVALUE ambient = scene->getAmbientColor();
        device->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_COLORVALUE(
            ambient.r, ambient.g, ambient.b, ambient.a));
        device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

    }

    return true;
}

void Viewport::cleanup() {
    if (device) { device->Release(); device = nullptr; }
    if (d3d) { d3d->Release(); d3d = nullptr; }
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

    D3DPRESENT_PARAMETERS d3dpp = {};
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    d3dpp.BackBufferWidth = event->size().width();
    d3dpp.BackBufferHeight = event->size().height();
    d3dpp.hDeviceWindow = (HWND)winId();

    // Reset device
    if (SUCCEEDED(device->Reset(&d3dpp))) {
        applyCommonRenderStates();
        if (scene) scene->restoreDeviceObjects(device);
    }
}

void Viewport::keyPressEvent(QKeyEvent* ev) { pressedKeys.insert(ev->key()); }

void Viewport::keyReleaseEvent(QKeyEvent* ev) { pressedKeys.remove(ev->key()); }

void Viewport::mousePressEvent(QMouseEvent* ev) {
    if (ev->button() == Qt::RightButton) {
        rightMouseHeld = true; lastMousePos = ev->pos(); setCursor(Qt::BlankCursor);
        grabMouse(); setFocus();
    }
}

void Viewport::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        rightMouseHeld = false;
        releaseMouse();
        setCursor(Qt::ArrowCursor);
    }
}

void Viewport::mouseMoveEvent(QMouseEvent* ev) {
    if (!rightMouseHeld) return;
    QPoint delta = ev->pos() - lastMousePos;
    lastMousePos = ev->pos();
    float s = 0.002f;
    yaw += delta.x() * s;
    pitch -= delta.y() * s;
    pitch = std::clamp(pitch, -1.5f, 1.5f);
    D3DXVECTOR3 front{ cosf(pitch) * sinf(yaw), sinf(pitch), cosf(pitch) * cosf(yaw) };
    D3DXVec3Normalize(&cameraDir, &front);
}

void Viewport::updateCamera(float deltaTime) {
    D3DXVECTOR3 right;
    D3DXVec3Cross(&right, &cameraDir, &cameraUp);
    D3DXVec3Normalize(&right, &right);
    float speed = 5.0f * deltaTime;
    if (pressedKeys.contains(Qt::Key_W)) {
        cameraPos += cameraDir * speed;
        qDebug() << "W pressed, cameraPos:" << cameraPos.x << cameraPos.y << cameraPos.z;
    }
    if (pressedKeys.contains(Qt::Key_S)) cameraPos -= cameraDir * speed;
    if (pressedKeys.contains(Qt::Key_A)) cameraPos -= right * speed;
    if (pressedKeys.contains(Qt::Key_D)) cameraPos += right * speed;

    D3DXMATRIX view;
    D3DXVECTOR3 look = cameraPos + cameraDir;
    D3DXMatrixLookAtLH(&view, &cameraPos, &look, &cameraUp);
    device->SetTransform(D3DTS_VIEW, &view);
}

void Viewport::applyCommonRenderStates() {
    // отключаем Qt‑рисовальщик, настраиваем Z‑тест и отбрасываем обратные полигоны
    device->SetRenderState(D3DRS_ZENABLE, TRUE);
    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
    device->SetRenderState(D3DRS_LIGHTING, scene->getLightingEnabled() ? TRUE : FALSE);

    // задаём цвет фонового клира
    D3DCOLORVALUE amb = scene ? scene->getAmbientColor() : D3DCOLORVALUE{ 0.1f,0.1f,0.1f,1.0f };
    device->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_COLORVALUE(amb.r, amb.g, amb.b, amb.a));

    // Проекцию катаем всегда здесь
    float aspect = width() / float(height());
    D3DXMATRIX proj;
    D3DXMatrixPerspectiveFovLH(&proj, D3DXToRadian(90), aspect, 0.1f, 100.0f);
    device->SetTransform(D3DTS_PROJECTION, &proj);
}

void Viewport::render() {
    if (!device || !scene) return;

    static QTime time = QTime::currentTime();
    int msecs = time.elapsed();
    time.restart();
    float deltaTime = msecs / 1000.0f;

    updateCamera(deltaTime);

    device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
        D3DCOLOR_XRGB(30, 30, 30), 1.0f, 0);

    if (SUCCEEDED(device->BeginScene())) {
        if (scene->getSkybox()) {
            float aspect = width() / static_cast<float>(height());
            scene->getSkybox()->render(device, cameraDir, cameraUp, aspect);
        }
        for (const auto& obj : scene->getObjects()) {
            obj->render(device);
        }
        device->EndScene();
    }

    device->Present(nullptr, nullptr, nullptr, nullptr);
}