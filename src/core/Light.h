#pragma once
#include "SceneObject.h"
#include <d3dx9math.h>

enum class LightType { Directional, Point, Spot };

class Light : public SceneObject {
    Q_OBJECT
public:
    Light(QObject* parent = nullptr);
    void render(LPDIRECT3DDEVICE9 device) override;
    std::string getName() const override { return "Light"; }
    unsigned int getId() const override { return 2; }

    // Light properties
    LightType type = LightType::Point;
    D3DCOLORVALUE diffuse{ 1,1,1,1 };
    float intensity = 1.0f;

    float getPositionX() const override { return position.x; }
    float getPositionY() const override { return position.y; }
    float getPositionZ() const override { return position.z; }
    void setPositionX(float p) override { position.x = p; emit propertiesChanged(); }
    void setPositionY(float p) override { position.y = p; emit propertiesChanged(); }
    void setPositionZ(float p) override { position.z = p; emit propertiesChanged(); }

    bool restoreDeviceObjects(LPDIRECT3DDEVICE9) override { return true; }
private:
    D3DXVECTOR3 position{ 0,5,0 };
    static int lightIndexCounter;
    int lightIndex;
};