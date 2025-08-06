#include "BoxColliderComponent.h"

bool BoxColliderComponent::checkCollision(const D3DXVECTOR3& position, const ColliderComponent* other, const D3DXVECTOR3& otherPosition) const {
    if (!other) return false;
    
    D3DXVECTOR3 myMin = position + offset - size * 0.5f;
    D3DXVECTOR3 myMax = position + offset + size * 0.5f;
    
    if (other->getType() == BOX) {
        D3DXVECTOR3 otherMin = otherPosition + other->getOffset() - other->getSize() * 0.5f;
        D3DXVECTOR3 otherMax = otherPosition + other->getOffset() + other->getSize() * 0.5f;
        
        return (myMin.x < otherMax.x && myMax.x > otherMin.x &&
                myMin.y < otherMax.y && myMax.y > otherMin.y &&
                myMin.z < otherMax.z && myMax.z > otherMin.z);
    }
    else if (other->getType() == SPHERE) {
        D3DXVECTOR3 sphereCenter = otherPosition + other->getOffset();
        float sphereRadius = other->getSize().x;
        
        D3DXVECTOR3 closestPoint;
        closestPoint.x = std::max(myMin.x, std::min(sphereCenter.x, myMax.x));
        closestPoint.y = std::max(myMin.y, std::min(sphereCenter.y, myMax.y));
        closestPoint.z = std::max(myMin.z, std::min(sphereCenter.z, myMax.z));
        
        float distance = D3DXVec3Length(&(sphereCenter - closestPoint));
        return distance < sphereRadius;
    }
    
    return false;
}