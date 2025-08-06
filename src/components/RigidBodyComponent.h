#pragma once

#include "Component.h"
#include <d3dx9.h>

class RigidBodyComponent : public Component {
    Q_OBJECT
public:
    explicit RigidBodyComponent(QObject* parent = nullptr);

    void setMass(float mass) { this->mass = mass; }
    float getMass() const { return mass; }

    void setUseGravity(bool use) { useGravity = use; }
    bool getUseGravity() const { return useGravity; }

    void setVelocity(const D3DXVECTOR3& vel) { velocity = vel; }
    const D3DXVECTOR3& getVelocity() const { return velocity; }

    void addForce(const D3DXVECTOR3& force) { forces += force; }
    void clearForces() { forces = D3DXVECTOR3(0, 0, 0); }

    void update(float deltaTime);

private:
    float mass = 1.0f;
    bool useGravity = true;
    D3DXVECTOR3 velocity = { 0, 0, 0 };
    D3DXVECTOR3 forces = { 0, 0, 0 };
};