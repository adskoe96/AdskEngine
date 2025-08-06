#include "RigidBodyComponent.h"
#include "Transform.h"
#include "SceneObject.h"

RigidBodyComponent::RigidBodyComponent(QObject* parent)
    : Component(parent)
{
}

void RigidBodyComponent::update(float deltaTime) {
    if (!getOwner()) return;

    auto* transform = getOwner()->getComponent<Transform>();
    if (!transform) return;

    if (useGravity) {
        forces.y -= 9.81f * mass;
    }

    D3DXVECTOR3 acceleration = forces / mass;

    velocity += acceleration * deltaTime;

    D3DXVECTOR3 position = transform->getPosition();
    position += velocity * deltaTime;
    transform->setPosition(position);

    clearForces();
}