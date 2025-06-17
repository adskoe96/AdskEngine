#pragma once
#include "d3d9.h"
#include <QObject>
#include <QFormLayout>

class SceneObject;

class Component : public QObject {
    Q_OBJECT
public:
    explicit Component(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~Component() = default;

    virtual void onAttach() {}
    virtual void onDetach() {}
    virtual void update(float dt) {}
    virtual void render(LPDIRECT3DDEVICE9 device) {}
    virtual void createInspector(QWidget* parent, QFormLayout* layout) {}

    virtual void invalidate() {}
    virtual void invalidateDeviceObjects() {}
    virtual bool restoreDeviceObjects(LPDIRECT3DDEVICE9 device) { return true; }

    void setOwner(SceneObject* owner) { this->owner = owner; }
    SceneObject* getOwner() const { return owner; }

private:
    SceneObject* owner = nullptr;
};