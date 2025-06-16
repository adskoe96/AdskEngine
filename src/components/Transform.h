#pragma once
#include "Component.h"
#include <QDoubleSpinBox>
#include <QSlider>
#include <QFormLayout>
#include <QWidget>
#include <d3dx9math.h>

class SceneObject;

class Transform : public Component {
    Q_OBJECT
public:
    explicit Transform(QObject* parent = nullptr) : Component(parent) {}

    D3DXVECTOR3 position{ 0,0,0 };
    D3DXVECTOR3 rotation{ 0,0,0 };
    D3DXVECTOR3 scale{ 1,1,1 };

    D3DXMATRIX getWorldMatrix() const;

    void createInspector(QWidget* parent, QFormLayout* layout) override;
};