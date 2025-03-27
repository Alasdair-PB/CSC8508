//
// Contributors: Alfie
//

#include "DungeonComponent.h"

#include "CollisionDetection.h"
#include "RoomManager.h"

#include "INetworkDeltaComponent.h" // Needed to use GameObject::AddComponent<>()

#include "../../CSC8508CoreClasses/Util.cpp"

void DungeonComponent::Generate(int const roomCount) const {
    int failures = 0;

    // Generate the first room
    auto* entryRoom = new GameObject();
    RoomPrefab* prefab = RoomManager::GetRandom();
    entryRoom->AddComponent<RoomComponent>(prefab);
    GetGameObject().AddChild(entryRoom);

    // Generate subsequent rooms
    for (int i = 0; i < roomCount - 1; i++) {
        if (!GenerateRoom()) {
            i--;
            failures++;
        }
        if (failures >= 5) {
            std::cout << "DungeonComponent::Generate roomCount exceeded max attempts!\n";
            return;
        }
    }
}

bool DungeonComponent::GenerateRoom() const {

    // 1: Pick a random prefab and create the game object for it
    auto* roomB = new GameObject();
    RoomPrefab* prefab = RoomManager::GetRandom();
    auto* component = roomB->AddComponent<RoomComponent>(prefab);

    // 2: Randomly order the rooms and attempt to generate a new room in each until one succeeds
    for (auto const rooms = Util::RandomiseVector(GetRooms()); RoomComponent* r : rooms) {
        if (r->TryGenerateNewRoom(*component)) return true;
    }

    delete roomB;
    return false;
}

std::vector<RoomComponent*> DungeonComponent::GetRooms() const {
    std::vector<RoomComponent*> out;
    for (GameObject const* c : GetGameObject().GetChildren()) {
        if (auto* component = c->TryGetComponent<RoomComponent>()) out.push_back(component);
    }
    return out;
}

