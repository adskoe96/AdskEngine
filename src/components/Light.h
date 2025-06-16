#pragma once
#include "Component.h"
#include <d3dx9math.h>

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QColorDialog>
#include <QFormLayout>

class SceneObject;
class Transform;

enum class LightType { Directional, Point, Spot };

class Light : public Component {
    Q_OBJECT
public:
    explicit Light(QObject* parent = nullptr) : Component(parent) {}

    void render(LPDIRECT3DDEVICE9 device) override;
    void createInspector(QWidget* parent, QFormLayout* layout) override;

    LightType type = LightType::Point;
    D3DCOLORVALUE color{ 1,1,1,1 };
    float intensity = 1.0f;
    int index = 0;
};