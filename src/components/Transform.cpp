#include "Transform.h"
#include "SceneObject.h"
#include <QLabel>

D3DXMATRIX Transform::getWorldMatrix() const {
    if (isDirty) {
        D3DXMATRIX S, R, T;
        const D3DXVECTOR3 rad = {
            D3DXToRadian(rotation.x),
            D3DXToRadian(rotation.y),
            D3DXToRadian(rotation.z)
        };

        D3DXMatrixScaling(&S, scale.x, scale.y, scale.z);
        D3DXMatrixRotationYawPitchRoll(&R, rad.y, rad.x, rad.z);
        D3DXMatrixTranslation(&T, position.x, position.y, position.z);

        cachedWorldMatrix = S * R * T;
        isDirty = false;
    }
    return cachedWorldMatrix;
}

QJsonObject Transform::serialize() const
{
    QJsonObject jsTr;
    jsTr["posX"] = position.x;
    jsTr["posY"] = position.y;
    jsTr["posZ"] = position.z;
    jsTr["rotX"] = rotation.x;
    jsTr["rotY"] = rotation.y;
    jsTr["rotZ"] = rotation.z;
    jsTr["scaleX"] = scale.x;
    jsTr["scaleY"] = scale.y;
    jsTr["scaleZ"] = scale.z;
    return jsTr;
}

void Transform::deserialize(const QJsonObject& data)
{
    setPosition({
        static_cast<float>(data["posX"].toDouble()),
        static_cast<float>(data["posY"].toDouble()),
        static_cast<float>(data["posZ"].toDouble())
        });

    setRotation({
        static_cast<float>(data["rotX"].toDouble()),
        static_cast<float>(data["rotY"].toDouble()),
        static_cast<float>(data["rotZ"].toDouble())
        });

    setScale({
        static_cast<float>(data["scaleX"].toDouble()),
        static_cast<float>(data["scaleY"].toDouble()),
        static_cast<float>(data["scaleZ"].toDouble())
        });
}

void Transform::createInspector(QWidget* parent, QFormLayout* layout)
{
    // Label of component
    auto* transformLabel = new QLabel("Transform", parent);
    layout->addRow(transformLabel);

    // Position
    auto createInput = [parent, layout](const QString& label, float value, auto setter) {
        auto* spinner = new QDoubleSpinBox(parent);
        spinner->setRange(-100000, 100000);
        spinner->setValue(value);

        // Исправленный connect с явным указанием перегрузки
        QObject::connect(spinner, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [setter](double v) { (setter)(static_cast<float>(v)); });

        layout->addRow(label, spinner);
        return spinner;
    };

    // Position
    createInput("Position X", position.x, [this](float v) { setPositionX(v); });
    createInput("Position Y", position.y, [this](float v) { setPositionY(v); });
    createInput("Position Z", position.z, [this](float v) { setPositionZ(v); });

    // Rotation
    createInput("Rotation X", rotation.x, [this](float v) { setRotationX(v); });
    createInput("Rotation Y", rotation.y, [this](float v) { setRotationY(v); });
    createInput("Rotation Z", rotation.z, [this](float v) { setRotationZ(v); });

    // Scale
    createInput("Scale X", scale.x, [this](float v) { setScaleX(v); });
    createInput("Scale Y", scale.y, [this](float v) { setScaleY(v); });
    createInput("Scale Z", scale.z, [this](float v) { setScaleZ(v); });
}

void Transform::setPosition(const D3DXVECTOR3& pos)
{
    if (position.x != pos.x || position.y != pos.y || position.z != pos.z) {
        position = pos;
        isDirty = true;
        if (getOwner()) emit getOwner()->propertiesChanged();
    }
}

void Transform::setRotation(const D3DXVECTOR3& rot)
{
    if (rotation.x != rot.x || rotation.y != rot.y || rotation.z != rot.z) {
        rotation = rot;
        isDirty = true;
        if (getOwner()) emit getOwner()->propertiesChanged();
    }
}

void Transform::setScale(const D3DXVECTOR3& scl)
{
    if (scale.x != scl.x || scale.y != scl.y || scale.z != scl.z) {
        scale = scl;
        isDirty = true;
        if (getOwner()) emit getOwner()->propertiesChanged();
    }
}