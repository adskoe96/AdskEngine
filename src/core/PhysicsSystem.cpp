#include "PhysicsSystem.h"
#include "Scene.h"
#include "SceneObject.h"
#include "Transform.h"
#include "RigidBodyComponent.h"
#include "ColliderComponent.h"

PhysicsSystem& PhysicsSystem::getInstance() {
    static PhysicsSystem instance;
    return instance;
}

void PhysicsSystem::initialize(Scene* scene) {
    this->scene = scene;
}

void PhysicsSystem::simulate(float deltaTime) {
    if (!simulationEnabled || !scene) return;

    std::lock_guard<std::mutex> lock(objectsMutex);

    for (auto& objPtr : scene->getObjects()) {
        auto* rb = objPtr->getComponent<RigidBodyComponent>();
        if (rb) {
            rb->update(deltaTime);
        }
    }

    for (size_t i = 0; i < scene->getObjects().size(); ++i) {
        auto& obj1 = scene->getObjects()[i];
        auto* transform1 = obj1->getComponent<Transform>();
        auto* collider1 = obj1->getComponent<ColliderComponent>();
        auto* rb1 = obj1->getComponent<RigidBodyComponent>();

        if (!transform1 || !collider1) continue;

        for (size_t j = i + 1; j < scene->getObjects().size(); ++j) {
            auto& obj2 = scene->getObjects()[j];
            auto* transform2 = obj2->getComponent<Transform>();
            auto* collider2 = obj2->getComponent<ColliderComponent>();
            auto* rb2 = obj2->getComponent<RigidBodyComponent>();

            if (!transform2 || !collider2) continue;

            if (collider1->checkCollision(transform1->getPosition(), collider2, transform2->getPosition())) {
                if (rb1) {
                    D3DXVECTOR3 velocity = rb1->getVelocity();
                    velocity.y = std::abs(velocity.y) * 0.8f;
                    rb1->setVelocity(velocity);
                }

                if (rb2) {
                    D3DXVECTOR3 velocity = rb2->getVelocity();
                    velocity.y = std::abs(velocity.y) * 0.8f;
                    rb2->setVelocity(velocity);
                }
            }
        }
    }
}

void PhysicsSystem::saveState() {
    savedStates.clear();
    if (!scene) return;

    for (auto& objPtr : scene->getObjects()) {
        auto* transform = objPtr->getComponent<Transform>();
        auto* rb = objPtr->getComponent<RigidBodyComponent>();

        if (transform) {
            ObjectState state;
            state.position = transform->getPosition();
            state.rotation = transform->getRotation();

            if (rb) {
                state.velocity = rb->getVelocity();
            }

            savedStates[objPtr.get()] = state;
        }
    }
}

void PhysicsSystem::restoreState() {
    for (auto& pair : savedStates) {
        SceneObject* obj = pair.first;
        const ObjectState& state = pair.second;

        auto* transform = obj->getComponent<Transform>();
        auto* rb = obj->getComponent<RigidBodyComponent>();

        if (transform) {
            transform->setPosition(state.position);
            transform->setRotation(state.rotation);

            if (rb) {
                rb->setVelocity(state.velocity);
            }
        }
    }
}