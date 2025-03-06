
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
        case Bounds: {
            object.AddComponent<BoundsComponent>(nullptr, nullptr);
        }
        case Physics: {
            object.AddComponent<PhysicsComponent>();
        }
        default:
            break;
    }
    return true;
}

void ComponentAssemblyDefiner::InitializeMap() {
    EventManager::RegisterListener<AddComponentEvent>(this, EARLY);
    SetHash<BoundsComponent>(Bounds);
    SetHash<PhysicsComponent>(Physics);
}
