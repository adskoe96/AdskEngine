#include "Light.h"
#include "SceneObject.h"
#include "Transform.h"

void Light::render(LPDIRECT3DDEVICE9 device)
{
    D3DLIGHT9 L{};
        L.Type = (type == LightType::Directional ? D3DLIGHT_DIRECTIONAL :
            type == LightType::Point ? D3DLIGHT_POINT : D3DLIGHT_SPOT);
        L.Diffuse = color;
        L.Diffuse.r *= intensity;
        L.Diffuse.g *= intensity;
        L.Diffuse.b *= intensity;

        if (type != LightType::Directional) {
            auto* tr = getOwner()->getComponent<Transform>();
            if (tr) {
                L.Position = tr->position;
                if (type == LightType::Spot) {
                    D3DXVECTOR3 dir{ 0,0,1 };
                    D3DXMATRIX rx, ry;
                    D3DXMatrixRotationX(&rx, tr->rotation.x);
                    D3DXMatrixRotationY(&ry, tr->rotation.y);
                    D3DXVec3TransformNormal(&dir, &dir, &(rx * ry));
                    L.Direction = dir;
                }
            }
        }

        device->SetLight(index, &L);
        device->LightEnable(index, TRUE);
}

void Light::createInspector(QWidget* parent, QFormLayout* layout)
{
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