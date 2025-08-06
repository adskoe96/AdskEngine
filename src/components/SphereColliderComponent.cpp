#include "SphereColliderComponent.h"

bool SphereColliderComponent::checkCollision(const D3DXVECTOR3& position, const ColliderComponent* other,
    const D3DXVECTOR3& otherPosition) const {
    if (!other) return false;

    D3DXVECTOR3 center = position + offset;
    float radius = size.x;

    if (other->getType() == SPHERE) {
        D3DXVECTOR3 otherCenter = otherPosition + other->getOffset();
        float otherRadius = other->getSize().x;

        float distance = D3DXVec3Length(&(center - otherCenter));
        return distance < (radius + otherRadius);
    }
    else if (other->getType() == BOX) {
        return other->checkCollision(otherPosition, this, center);
    }

    return false;
}