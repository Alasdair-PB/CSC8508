
#include "ComponentAssemblyDefiner.h"
#include "BoundsComponent.h"
#include "PhysicsComponent.h"
#include "AnimationComponent.h"
#include "INetworkDeltaComponent.h"
#include "INetworkComponent.h"
#include "DamageableComponent.h"


using namespace NCL;
using namespace CSC8508;
bool ComponentAssemblyDefiner::AddComponent(size_t t, GameObject& object) {
    switch (componentMap[t])
    {
    case ComponentMapId::Bounds: {
        object.AddComponent<BoundsComponent>(nullptr, nullptr);
        break;
    } case ComponentMapId::Physics: {
        object.AddComponent<PhysicsComponent>();
        break;
    }case ComponentMapId::NavMesh: {
        object.AddComponent<PhysicsComponent>();
        break;
    }case ComponentMapId::Damageable: {
        object.AddComponent<DamageableComponent>(1, 1);
        break;
    
    }case ComponentMapId::Animation: {
        std::cout << "Damageable added" << std::endl;
        object.AddComponent<AnimationComponent>();
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
