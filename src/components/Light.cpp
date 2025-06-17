#include "Light.h"
#include "SceneObject.h"
#include "Transform.h"

#include <QLabel>

void Light::render(LPDIRECT3DDEVICE9 device)
{
    D3DLIGHT9 L{};
    L.Type = (type == LightType::Directional ? D3DLIGHT_DIRECTIONAL :
        type == LightType::Point ? D3DLIGHT_POINT : D3DLIGHT_SPOT);
    L.Diffuse = color;
    L.Diffuse.r *= intensity;
    L.Diffuse.g *= intensity;
    L.Diffuse.b *= intensity;

    auto* tr = getOwner()->getComponent<Transform>();
    if (tr) {
        L.Position = tr->position;

        if (type != LightType::Point) {
            D3DXVECTOR3 dir(0, 0, 1);

            D3DXMATRIX rotationMatrix;
            D3DXMatrixRotationYawPitchRoll(&rotationMatrix,
                D3DXToRadian(tr->rotation.y),
                D3DXToRadian(tr->rotation.x),
                D3DXToRadian(tr->rotation.z));

            D3DXVec3TransformNormal(&dir, &dir, &rotationMatrix);
            D3DXVec3Normalize(&dir, &dir);
            L.Direction = dir;
        }
    }

    if (type == LightType::Point || type == LightType::Spot) {
        L.Range = 100.0f;
        L.Attenuation0 = 1.0f;
        L.Attenuation1 = 0.1f;
        L.Attenuation2 = 0.01f;

        if (type == LightType::Spot) {
            L.Theta = D3DXToRadian(30.0f);
            L.Phi = D3DXToRadian(45.0f);
            L.Falloff = 1.0f;
        }
    }

    device->SetLight(index, &L);
    device->LightEnable(index, TRUE);
}

void Light::createInspector(QWidget* parent, QFormLayout* layout)
{
    // Label of component
    auto* componentLabel = new QLabel("Light", parent);
    layout->addRow(componentLabel);

    // Type
    auto* combo = new QComboBox(parent);
    combo->addItems({ "Directional","Point","Spot" });
    combo->setCurrentIndex(int(type));
    connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int i) { type = LightType(i); emit getOwner()->propertiesChanged(); });
    layout->addRow("Light Type", combo);

    // Intensity
    auto* intens = new QDoubleSpinBox(parent);
    intens->setRange(0, 10); intens->setValue(intensity);
    connect(intens, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this](double v) { intensity = float(v); emit getOwner()->propertiesChanged(); });
    layout->addRow("Intensity", intens);

    // Color
    auto* btn = new QPushButton(parent);
    QColor qc;
    qc.setRgbF(color.r, color.g, color.b);
    btn->setStyleSheet("background-color:" + qc.name());
    connect(btn, &QPushButton::clicked, [this, btn]() {
        QColor newC = QColorDialog::getColor();
        if (newC.isValid()) {
            color = {
                static_cast<float>(newC.redF()),
                static_cast<float>(newC.greenF()),
                static_cast<float>(newC.blueF()),
                1.0f
            };
            btn->setStyleSheet("background-color:" + newC.name());
            emit getOwner()->propertiesChanged();
        }
        });
    layout->addRow("Color", btn);
}