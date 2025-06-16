#pragma once
#include "Component.h"
#include <vector>
#include <memory>
#include <typeindex>
#include <cassert>
#include <Transform.h>

class Entity {
public:
    Entity(const std::string& name) : name(name) {
        addComponent<Transform>();
    }

    const std::string& getName() const { return name; }

    template<typename T, typename... Args>
    T* addComponent(Args&&... args) {
        auto idx = std::type_index(typeid(T));
        assert(components.find(idx) == components.end() && "Component already exists!");
        auto comp = std::make_unique<T>(std::forward<Args>(args)...);
        comp->setOwner(this);
        T* ptr = comp.get();
        components[idx] = std::move(comp);
        orderedComponents.push_back(ptr);
        return ptr;
    }

    template<typename T>
    T* getComponent() {
        auto it = components.find(std::type_index(typeid(T)));
        return it != components.end() ? static_cast<T*>(it->second.get()) : nullptr;
    }

    template<typename T>
    void removeComponent() {
        auto idx = std::type_index(typeid(T));
        components.erase(idx);
        orderedComponents.erase(std::remove_if(orderedComponents.begin(), orderedComponents.end(),
            [&](Component* c) { return dynamic_cast<T*>(c) != nullptr; }),
            orderedComponents.end());
    }

    void update(float dt) {
        for (auto* c : orderedComponents) c->update(dt);
    }

    void render(LPDIRECT3DDEVICE9 device) {
        for (auto* c : orderedComponents) c->render(device);
    }

private:
    std::string name;
    std::unordered_map<std::type_index, std::unique_ptr<Component>> components;
    std::vector<Component*> orderedComponents;
};
