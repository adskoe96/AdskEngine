#pragma once

#define NOMINMAX
#include "ColliderComponent.h"
#include <d3dx9.h>

class SceneObject;

class BoxColliderComponent : public ColliderComponent {
    Q_OBJECT
public:
    ColliderType getType() const override { return BOX; }

    bool checkCollision(const D3DXVECTOR3& position, const ColliderComponent* other, const D3DXVECTOR3& otherPosition) const override;
};