#include "Light.h"
#include "SceneObject.h"
#include "Transform.h"

#include <algorithm>
#include <QLabel>

QJsonObject Light::serialize() const
{
    QJsonObject jsL;
    jsL["type"] = int(type);
    jsL["intensity"] = intensity;
    jsL["colorR"] = color.r;
    jsL["colorG"] = color.g;
    jsL["colorB"] = color.b;
    return jsL;
}

void Light::deserialize(const QJsonObject& data)
{
    type = static_cast<LightType>(data["type"].toInt());
    intensity = static_cast<float>(data["intensity"].toDouble());
    color.r = static_cast<float>(data["colorR"].toDouble());
    color.g = static_cast<float>(data["colorG"].toDouble());
    color.b = static_cast<float>(data["colorB"].toDouble());
}

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
        const auto& pos = tr->getPosition();
        const auto& rot = tr->getRotation();

        L.Position = pos;

        if (type != LightType::Point) {
            D3DXVECTOR3 dir(0, 0, 1);
            D3DXMATRIX rotationMatrix;

            D3DXMatrixRotationYawPitchRoll(&rotationMatrix,
                D3DXToRadian(rot.y),
                D3DXToRadian(rot.x),
                D3DXToRadian(rot.z));

            D3DXVec3TransformNormal(&dir, &dir, &rotationMatrix);
            D3DXVec3Normalize(&dir, &dir);
            L.Direction = dir;
        }
    }

    if (type == LightType::Point || type == LightType::Spot) {
        L.Range = radius;
        L.Attenuation0 = 1.0f;
        L.Attenuation1 = 0.1f;
        L.Attenuation2 = 0.01f;

        if (type == LightType::Spot) {
            L.Theta = D3DXToRadian(30.0f);
            L.Phi = D3DXToRadian(45.0f);
            L.Falloff = spotFalloff;
        }
    }

    device->SetLight(index, &L);
    device->LightEnable(index, TRUE);
}

void Light::createInspector(QWidget* parent, QFormLayout* layout)
{
    auto* componentLabel = new QLabel("Light", parent);
    layout->addRow(componentLabel);

    auto* combo = new QComboBox(parent);
    combo->addItems({ "Directional","Point","Spot" });
    combo->setCurrentIndex(int(type));
    connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int i) { type = LightType(i); emit getOwner()->propertiesChanged(); });
    layout->addRow("Light Type", combo);

    auto* intens = new QDoubleSpinBox(parent);
    intens->setRange(0, 10); intens->setValue(intensity);
    connect(intens, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this](double v) { intensity = float(v); emit getOwner()->propertiesChanged(); });
    layout->addRow("Intensity", intens);

    auto* rad = new QDoubleSpinBox(parent);
    rad->setRange(0, 10000); rad->setValue(radius);
    connect(rad, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this](double v) { radius = float(v); emit getOwner()->propertiesChanged(); });
    layout->addRow("Radius", rad);

    auto* fal = new QDoubleSpinBox(parent);
    fal->setRange(0, 90); fal->setValue(radius);
    connect(fal, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this](double v) { spotFalloff = float(v); emit getOwner()->propertiesChanged(); });
    layout->addRow("Falloff", rad);

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