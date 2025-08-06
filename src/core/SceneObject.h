#pragma once

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <vector>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <algorithm>
#include <cassert>
#include <d3d9.h>

#include "Component.h"
#include "Transform.h"
class Transform;

class SceneObject : public QObject {
    Q_OBJECT
public:
    explicit SceneObject(const std::string& name = "Entity", QObject* parent = nullptr) : QObject(parent), name(name)
    {
        this->addComponent<Transform>(this);
    }

    QJsonObject SceneObject::serialize() const {
        QJsonObject o;
        o["name"] = QString::fromStdString(name);

        QJsonArray componentsArray;
        for (const auto& comp : orderedComponents) {
            QJsonObject compObj;
            compObj["type"] = QString::fromStdString(comp->getTypeName());
            compObj["data"] = comp->serialize();
            componentsArray.append(compObj);
        }
        o["components"] = componentsArray;

        return o;
    }

    void setName(std::string value) {
        if (name != value) {
            name = value;
            emit nameChanged(name);
        }
    }
    const std::string& getName() const { return name; }

    const std::vector<Component*>& getAllComponents() const { return orderedComponents; }

    template<typename T, typename... Args>
    T* addComponent(Args&&... args) {
        auto idx = std::type_index(typeid(T));
        assert(components.find(idx) == components.end());
        auto comp = std::make_unique<T>(std::forward<Args>(args)...);
        comp->setOwner(this);
        T* ptr = comp.get();
        components[idx] = std::move(comp);
        orderedComponents.push_back(ptr);
        ptr->onAttach();
        connect(this, &SceneObject::propertiesChanged, ptr, &Component::onPropertiesChanged);
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
        auto it = components.find(idx);
        if (it != components.end()) {
            it->second->onDetach();
            components.erase(it);
            orderedComponents.erase(
                std::remove_if(orderedComponents.begin(), orderedComponents.end(),
                    [](Component* c) { return dynamic_cast<T*>(c) != nullptr; }),
                orderedComponents.end());
        }
    }

    void update(float dt) {
        for (auto* c : orderedComponents) c->update(dt);
    }

    void render(LPDIRECT3DDEVICE9 device) {
        for (auto* c : orderedComponents) c->render(device);
    }

    void invalidateDeviceObjects() {
        for (auto* c : orderedComponents)
            c->invalidateDeviceObjects();
    }

    bool restoreDeviceObjects(LPDIRECT3DDEVICE9 device) {
        bool ok = true;
        for (auto* c : orderedComponents)
            ok &= c->restoreDeviceObjects(device);
        return ok;
    }

signals:
    void propertiesChanged();
    void nameChanged(const std::string& newName);

private:
    std::string name;
    std::unordered_map<std::type_index, std::unique_ptr<Component>> components;
    std::vector<Component*> orderedComponents;
};