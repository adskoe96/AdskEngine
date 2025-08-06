#pragma once

#include <vector>
#include <mutex>
#include <unordered_map>
#include <d3dx9.h>

class Scene;
class SceneObject;

class PhysicsSystem {
public:
    static PhysicsSystem& getInstance();

    void initialize(Scene* scene);
    void simulate(float deltaTime);
    void setSimulationEnabled(bool enabled) { simulationEnabled = enabled; }
    bool isSimulationEnabled() const { return simulationEnabled; }

    void saveState();
    void restoreState();

private:
    PhysicsSystem() = default;

    struct ObjectState {
        D3DXVECTOR3 position;
        D3DXVECTOR3 rotation;
        D3DXVECTOR3 velocity;
    };

    bool simulationEnabled = false;
    Scene* scene = nullptr;
    std::mutex objectsMutex;
    std::unordered_map<SceneObject*, ObjectState> savedStates;
};