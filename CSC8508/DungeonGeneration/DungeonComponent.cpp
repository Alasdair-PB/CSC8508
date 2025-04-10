//
// Contributors: Alfie
//

#include "DungeonComponent.h"
#include "CollisionDetection.h"
#include "RoomManager.h"
#include "INetworkDeltaComponent.h"
#include "../../CSC8508CoreClasses/Util.cpp"

bool DungeonComponent::Generate(int const roomCount) const {
    int failures = 0;
    std::srand(GetSeed());

    GameObject* prefab = RoomManager::GetRandom();
    GameObject* copy = prefab->CopyGameObject();
    RoomManager::ReturnPrefab(prefab);
    RoomPrefab* roomPrefab = copy->TryGetComponent<RoomPrefab>();
    GetGameObject().AddChild(copy);

    // Line up the entry room with the dungeon entrance
    DoorLocation const doorLoc = roomPrefab->GetDoorLocations().at(0);
    Quaternion const orientationDifference = Quaternion::VectorsToQuaternion(doorLoc.dir, -entrancePosition.dir);
    Transform entryTransform = copy->GetTransform();
    entryTransform.SetOrientation(orientationDifference);
    entryTransform.SetPosition(
        GetGameObject().GetTransform().GetPosition()
        + GetGameObject().GetTransform().GetOrientation() * entrancePosition.pos * GetGameObject().GetTransform().GetScale()
        - entryTransform.GetOrientation() * doorLoc.pos * entryTransform.GetScale()
        );

    // Generate subsequent rooms
    for (int i = 0; i < roomCount - 1; i++) {
        if (!GenerateRoom()) {
            i--;
            failures++;
        }
        if (failures >= MAX_FAILURES) {
            std::cout << "failed" << std::endl;
            return false;
        }
    }
    return true;
}

bool DungeonComponent::GenerateRoom() const {
    GameObject* roomB = RoomManager::GetRandom();
    RoomPrefab* roomPrefabInfo = roomB->TryGetComponent<RoomPrefab>();

    // 2: Randomly order the rooms and attempt to generate a new room in each until one succeeds
    for (auto const rooms = Util::RandomiseVector(GetRooms()); RoomPrefab* r : rooms) {
        if (r->TryGenerateNewRoom(*roomPrefabInfo)) return true;
    }
    roomB->SetEnabled(false);
    //delete roomB;
    return false;
}

std::vector<RoomPrefab*> DungeonComponent::GetRooms() const {
    std::vector<RoomPrefab*> out;
    for (GameObject const* c : GetGameObject().GetChildren()) {
        if (auto* component = c->TryGetComponent<RoomPrefab>()) out.push_back(component);
    }
    return out;
}

void DungeonComponent::GetAllItemSpawnLocations(std::vector<Vector3>& locations) const {
    for (GameObject* r : GetGameObject().GetChildren()) {
        Transform const& transform = r->GetTransform();
        RoomPrefab const* roomComponent = r->TryGetComponent<RoomPrefab>();
        if (!roomComponent) continue;

        for (SpawnLocation const loc : roomComponent->GetItemSpawnLocations()) {
            Vector3 const outLoc = transform.GetOrientation() * (transform.GetScale() * loc.location)  + transform.GetPosition();
            locations.push_back(outLoc);
        }
    }
}
