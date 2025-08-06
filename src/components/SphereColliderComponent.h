#pragma once

#include "ColliderComponent.h"

class SphereColliderComponent : public ColliderComponent {
    Q_OBJECT
public:
    ColliderType getType() const override { return SPHERE; }

    bool checkCollision(const D3DXVECTOR3& position, const ColliderComponent* other,
        const D3DXVECTOR3& otherPosition) const override;

    void setRadius(float radius) { size.x = radius; }
    float getRadius() const { return size.x; }
};