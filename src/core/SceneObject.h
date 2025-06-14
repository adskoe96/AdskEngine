#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include <QObject>

class SceneObject : public QObject {
    Q_OBJECT
public:
    explicit SceneObject(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~SceneObject() = default;

    virtual void render(LPDIRECT3DDEVICE9 device) = 0;
    virtual std::string getName() const = 0;
    virtual unsigned int getId() const = 0;

    // Position
    virtual float getPositionX() const { return position.x; }
    virtual float getPositionY() const { return position.y; }
    virtual float getPositionZ() const { return position.z; }

    virtual void setPositionX(float p) { position.x = p; emit propertiesChanged(); }
    virtual void setPositionY(float p) { position.y = p; emit propertiesChanged(); }
    virtual void setPositionZ(float p) { position.z = p; emit propertiesChanged(); }

    // Rotation
    virtual float getRotationX() const { return rotation.x; }
    virtual float getRotationY() const { return rotation.y; }
    virtual float getRotationZ() const { return rotation.z; }

    virtual void setRotationX(float r) { rotation.x = r; emit propertiesChanged(); }
    virtual void setRotationY(float r) { rotation.y = r; emit propertiesChanged(); }
    virtual void setRotationZ(float r) { rotation.z = r; emit propertiesChanged(); }

    // Scale
    virtual float getScaleX() const { return scale.x; }
    virtual float getScaleY() const { return scale.y; }
    virtual float getScaleZ() const { return scale.z; }

    virtual void setScaleX(float s) { scale.x = s; emit propertiesChanged(); }
    virtual void setScaleY(float s) { scale.y = s; emit propertiesChanged(); }
    virtual void setScaleZ(float s) { scale.z = s; emit propertiesChanged(); }

    virtual void invalidateDeviceObjects() {}
    virtual bool restoreDeviceObjects(LPDIRECT3DDEVICE9 device) { return true; }

protected:
    D3DXVECTOR3 position{ 0,0,0 };
    D3DXVECTOR3 rotation{ 0,0,0 };
    D3DXVECTOR3 scale { 1,1,1 };

signals:
    void propertiesChanged();
};