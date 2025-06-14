#include "Light.h"

int Light::lightIndexCounter = 0;

Light::Light(QObject* parent)
    : SceneObject(parent)
    , lightIndex(lightIndexCounter++)
{
    // default initialization
}

void Light::render(LPDIRECT3DDEVICE9 device) {
    // Enable lighting for this draw
    //device->SetRenderState(D3DRS_LIGHTING, TRUE);

    D3DLIGHT9 L{};
    L.Type = (type == LightType::Directional) ? D3DLIGHT_DIRECTIONAL :
        (type == LightType::Point) ? D3DLIGHT_POINT : D3DLIGHT_SPOT;
    L.Diffuse = diffuse;
    L.Diffuse.r *= intensity;
    L.Diffuse.g *= intensity;
    L.Diffuse.b *= intensity;
    if (type != LightType::Directional) {
        L.Position = position;
        L.Range = 100.0f;
        L.Attenuation0 = 1.0f;
    }
    if (type == LightType::Spot) {
        // use rotation as aim direction
        D3DXVECTOR3 dir{ 0,0,1 };
        // apply yaw/pitch to dir
        D3DXMATRIX rx, ry;
        D3DXMatrixRotationX(&rx, rotation.x);
        D3DXMatrixRotationY(&ry, rotation.y);
        D3DXVec3TransformNormal(&dir, &dir, &(rx * ry));
        L.Direction = dir;
        L.Falloff = 1.0f;
        L.Theta = D3DXToRadian(20);
        L.Phi = D3DXToRadian(40);
    }
    device->SetLight(lightIndex, &L);
    device->LightEnable(lightIndex, TRUE);

    // disable lighting again for other objects
    //device->SetRenderState(D3DRS_LIGHTING, FALSE);
}