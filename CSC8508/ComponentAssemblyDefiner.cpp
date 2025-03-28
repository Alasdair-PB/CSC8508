
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
    return AddComponentByMap(componentMap[t], object);
}
bool ComponentAssemblyDefiner::AddComponentFromEnum(ComponentMapId mapId, GameObject& object) {
    return AddComponentByMap(mapId, object);
}

bool ComponentAssemblyDefiner::AddComponentByMap(ComponentMapId mapId, GameObject& object) {
    switch (mapId)
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
