#pragma once

#include "Component.h"
#include <QDoubleSpinBox>
#include <QSlider>
#include <QFormLayout>
#include <QWidget>
#include <QJsonObject>
#include <d3dx9math.h>


class SceneObject;

class Transform : public Component {
    Q_OBJECT
public:
    explicit Transform(QObject* parent = nullptr) : Component(parent) {}

    QJsonObject serialize() const override;
    void deserialize(const QJsonObject& data) override;

    std::string getTypeName() const override { return "Transform"; }

    void setPositionX(float x) { setPosition({ x, position.y, position.z }); }
    void setPositionY(float y) { setPosition({ position.x, y, position.z }); }
    void setPositionZ(float z) { setPosition({ position.x, position.y, z }); }

    void setRotationX(float x) { setRotation({ x, rotation.y, rotation.z }); }
    void setRotationY(float y) { setRotation({ rotation.x, y, rotation.z }); }
    void setRotationZ(float z) { setRotation({ rotation.x, rotation.y, z }); }

    void setScaleX(float x) { setScale({ x, scale.y, scale.z }); }
    void setScaleY(float y) { setScale({ scale.x, y, scale.z }); }
    void setScaleZ(float z) { setScale({ scale.x, scale.y, z }); }

    void setPosition(const D3DXVECTOR3& pos);
    void setRotation(const D3DXVECTOR3& rot);
    void setScale(const D3DXVECTOR3& scl);

    const D3DXVECTOR3& getPosition() const { return position; }
    const D3DXVECTOR3& getRotation() const { return rotation; }
    const D3DXVECTOR3& getScale() const { return scale; }

    D3DXMATRIX getWorldMatrix() const;
    void createInspector(QWidget* parent, QFormLayout* layout) override;

private:
    D3DXVECTOR3 position{ 0, 0, 0 };
    D3DXVECTOR3 rotation{ 0, 0, 0 };
    D3DXVECTOR3 scale{ 1, 1, 1 };

    mutable D3DXMATRIX cachedWorldMatrix;
    mutable bool isDirty = true;

    D3DXVECTOR3 getRotationRadians() const {
        return {
            D3DXToRadian(rotation.x),
            D3DXToRadian(rotation.y),
            D3DXToRadian(rotation.z)
        };
    }
};