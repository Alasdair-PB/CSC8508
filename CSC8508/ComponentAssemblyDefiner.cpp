
#include "ComponentAssemblyDefiner.h"
#include "BoundsComponent.h"
#include "PhysicsComponent.h"
#include "INetworkDeltaComponent.h"
#include "INetworkComponent.h"

using namespace NCL;
using namespace CSC8508;
bool ComponentAssemblyDefiner::AddComponent(size_t t, GameObject& object) {
    switch (componentMap[t])
    {
    case ComponentMapId::Bounds: {
        std::cout << "Bounds added" << std::endl;
        object.AddComponent<BoundsComponent>(nullptr, nullptr);
        break;
    } case ComponentMapId::Physics: {
        std::cout << "Physics added" << std::endl;
        object.AddComponent<PhysicsComponent>();
        break;
    }case ComponentMapId::NavMesh: {
        std::cout << "Navmesh added" << std::endl;
        object.AddComponent<PhysicsComponent>();
        break;
    }case ComponentMapId::Error:
            std::cout << "Error adding component" << std::endl;
            break;
    default: {
        std::cout << "adding undefined" << std::endl;
        break;
    }
    }
    return true;
}

void ComponentAssemblyDefiner::InitializeMap() {
    EventManager::RegisterListener<AddComponentEvent>(this, EARLY);
    SetHash<BoundsComponent>(Bounds);
    SetHash<PhysicsComponent>(Physics);
}
