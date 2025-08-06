#pragma once

#include "Component.h"
#include <d3dx9.h>

class ColliderComponent : public Component {
public:
    enum ColliderType { BOX, SPHERE };

    virtual ColliderType getType() const = 0;
    virtual bool checkCollision(const D3DXVECTOR3& position, const ColliderComponent* other,
        const D3DXVECTOR3& otherPosition) const = 0;

    void setOffset(const D3DXVECTOR3& offset) { this->offset = offset; }
    const D3DXVECTOR3& getOffset() const { return offset; }

    void setSize(const D3DXVECTOR3& size) { this->size = size; }
    const D3DXVECTOR3& getSize() const { return size; }

protected:
    D3DXVECTOR3 offset = { 0, 0, 0 };
    D3DXVECTOR3 size = { 1, 1, 1 };
};