#include "Transform.h"
#include "SceneObject.h"

D3DXMATRIX Transform::getWorldMatrix() const {
    D3DXMATRIX S, RX, RY, RZ, T;
    D3DXMatrixScaling(&S, scale.x, scale.y, scale.z);
    D3DXMatrixRotationX(&RX, rotation.x);
    D3DXMatrixRotationY(&RY, rotation.y);
    D3DXMatrixRotationZ(&RZ, rotation.z);
    D3DXMatrixTranslation(&T, position.x, position.y, position.z);
    return S * RZ * RY * RX * T;
}

void Transform::createInspector(QWidget* parent, QFormLayout* layout)
{
    // Position
    auto* posX = new QDoubleSpinBox(parent);
    posX->setRange(-100000, 100000); posX->setValue(position.x);
    connect(posX, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this](double v) { position.x = v; emit getOwner()->propertiesChanged(); });
    layout->addRow("Position X", posX);

    auto* posY = new QDoubleSpinBox(parent);
    posY->setRange(-100000, 100000); posY->setValue(position.y);
    connect(posY, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this](double v) { position.y = v; emit getOwner()->propertiesChanged(); });
    layout->addRow("Position Y", posY);

    auto* posZ = new QDoubleSpinBox(parent);
    posZ->setRange(-100000, 100000); posZ->setValue(position.z);
    connect(posZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this](double v) { position.z = v; emit getOwner()->propertiesChanged(); });
    layout->addRow("Position Z", posZ);

    // Rotation
    auto* rotX = new QDoubleSpinBox(parent);
    rotX->setRange(-360, 360); rotX->setValue(rotation.x);
    connect(rotX, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this](double v) { rotation.x = v; emit getOwner()->propertiesChanged(); });
    layout->addRow("Rotation X", rotX);

    auto* rotY = new QDoubleSpinBox(parent);
    rotY->setRange(-360, 360); rotY->setValue(rotation.y);
    connect(rotY, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this](double v) { rotation.y = v; emit getOwner()->propertiesChanged(); });
    layout->addRow("Rotation Y", rotY);

    auto* rotZ = new QDoubleSpinBox(parent);
    rotZ->setRange(-360, 360); rotZ->setValue(rotation.z);
    connect(rotZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this](double v) { rotation.z = v; emit getOwner()->propertiesChanged(); });
    layout->addRow("Rotation Z", rotZ);

    // Scale
    auto* scaleX = new QDoubleSpinBox(parent);
    scaleX->setRange(0, 1000); scaleX->setValue(scale.x);
    connect(scaleX, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this](double v) { scale.x = float(v); emit getOwner()->propertiesChanged(); });
    layout->addRow("Scale X", scaleX);

    auto* scaleY = new QDoubleSpinBox(parent);
    scaleY->setRange(0, 1000); scaleY->setValue(scale.y);
    connect(scaleY, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this](double v) { scale.y = float(v); emit getOwner()->propertiesChanged(); });
    layout->addRow("Scale Y", scaleY);

    auto* scaleZ = new QDoubleSpinBox(parent);
    scaleZ->setRange(0, 1000); scaleZ->setValue(scale.z);
    connect(scaleZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this](double v) { scale.z = float(v); emit getOwner()->propertiesChanged(); });
    layout->addRow("Scale Z", scaleZ);
}
