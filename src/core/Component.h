#pragma once
#include "d3d9.h"
#include <QObject>
#include <QJsonObject>
#include <QFormLayout>

class SceneObject;

class Component : public QObject {
    Q_OBJECT
public:
    explicit Component(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~Component() = default;

    virtual QJsonObject serialize() const { return QJsonObject(); }
    virtual void deserialize(const QJsonObject& data) { Q_UNUSED(data); }

    virtual void onAttach() {}
    virtual void onDetach() {}
    virtual void update(float dt) {}
    virtual void render(LPDIRECT3DDEVICE9 device) {}
    virtual void createInspector(QWidget* parent, QFormLayout* layout) {}

    virtual void onPropertiesChanged() {}
    virtual std::string getTypeName() const { return "Component"; }

    virtual void invalidate() {}
    virtual void invalidateDeviceObjects() {}
    virtual bool restoreDeviceObjects(LPDIRECT3DDEVICE9 device) { return true; }

    void setOwner(SceneObject* owner) { this->owner = owner; }
    SceneObject* getOwner() const { return owner; }

private:
    SceneObject* owner = nullptr;
};