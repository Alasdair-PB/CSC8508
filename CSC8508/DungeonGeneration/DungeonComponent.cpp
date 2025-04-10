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
            RoomManager::ClearPrefabs();
            return false;
        }
    }
    RoomManager::ClearPrefabs();
    return true;
}

bool DungeonComponent::GenerateRoom() const {
    GameObject* roomB = RoomManager::GetRandom();
    RoomPrefab* roomPrefabInfo = roomB->TryGetComponent<RoomPrefab>();

    // 2: Randomly order the rooms and attempt to generate a new room in each until one succeeds
    for (auto const rooms = Util::RandomiseVector(GetRooms()); RoomPrefab* r : rooms) {
        if (TryGenerateNewRoom(*r, *roomPrefabInfo)) return true;
    }
    return false;
}

bool DungeonComponent::EndDungeonPaths() {
    GameObject* roomB = RoomManager::GetRandom(Exit);
    RoomPrefab* roomPrefabInfo = roomB->TryGetComponent<RoomPrefab>();

    for (auto const rooms = GetRooms(); RoomPrefab* r : rooms)
        ForceGenerateRooms(*r, *roomPrefabInfo);
    return true;
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

void DungeonComponent::SetTransform(const Transform& transformA, Transform& transformB, const Quaternion orientationDifference,
    const DoorLocation aDoorLoc, const DoorLocation bDoorLoc) const {
    transformB.SetOrientation(orientationDifference * transformA.GetOrientation());
    transformB.SetPosition(
        transformA.GetPosition()
        + transformA.GetOrientation() * (transformA.GetScale() * aDoorLoc.pos)
        - transformB.GetOrientation() * (transformB.GetScale() * bDoorLoc.pos)
    );
}

void DungeonComponent::SetNeighbours(RoomPrefab* roomPrefab) const {
    for (RoomPrefab* room : GetRooms()) {
        for (const DoorLocation& door : room->GetDoorLocations()) {
            for (const DoorLocation& doorB : roomPrefab->GetDoorLocations()) {
                Vector3 doorPosA = room->GetGameObject().GetTransform().GetPosition() + room->GetGameObject().GetTransform().GetOrientation() * door.pos;
                Vector3 doorPosB = roomPrefab->GetGameObject().GetTransform().GetPosition() + roomPrefab->GetGameObject().GetTransform().GetOrientation() * doorB.pos;
                float length = Vector::Length(doorPosA - doorPosB);
                if (length < threshold) {
                    roomPrefab->GetNextDoorRooms().push_back(room);
                    room->GetNextDoorRooms().push_back(roomPrefab);
                }
            }
        }
    }
}

bool DungeonComponent::ForceGenerateRooms(RoomPrefab& roomA, RoomPrefab& roomB) const {
    if (roomA.GetNextDoorRooms().size() > roomA.GetDoorLocations().size()) {
        RoomManager::ReturnPrefab(&roomB.GetGameObject());
        return false;
    }
    auto const aDoorLocations = roomA.GetDoorLocations();
    auto const bDoorLocations = roomB.GetDoorLocations();

    Transform const& transformA = roomA.GetGameObject().GetTransform();
    Transform& transformB = roomB.GetGameObject().GetTransform();
    bool forced = false;
    for (const DoorLocation& aDoorLoc : aDoorLocations) {
        for (const DoorLocation& bDoorLoc : bDoorLocations) {
            Quaternion orientationDifference = Quaternion::VectorsToQuaternion(bDoorLoc.dir, -aDoorLoc.dir);
            // Enforce flipping around the Y axis (no tilting the rooms)
            if (fabs(orientationDifference.x) >= FLT_EPSILON || fabs(orientationDifference.z) >= FLT_EPSILON) continue;
            SetTransform(transformA, transformB, orientationDifference, aDoorLoc, bDoorLoc);
            auto info = CollisionDetection::CollisionInfo();

            if (!CollisionDetection::ObjectIntersection(&roomB.GetGameObject(), &GetGameObject(), info)) {
                GameObject* copiedPrefab = roomB.GetGameObject().CopyGameObject();
                RoomPrefab* roomPrefab = copiedPrefab->TryGetComponent<RoomPrefab>();
                GetGameObject().AddChild(copiedPrefab);
                SetNeighbours(roomPrefab);
                forced = true;
            }
        }
    }
    if (!forced) std::cout << "Room could not be generated at this door" << std::endl;
    RoomManager::ReturnPrefab(&roomB.GetGameObject());
    return true;
}

bool DungeonComponent::TryGenerateNewRoom(RoomPrefab& roomA, RoomPrefab& roomB) const {
    if (roomA.GetNextDoorRooms().size() > roomA.GetDoorLocations().size()) {
        RoomManager::ReturnPrefab(&roomB.GetGameObject());
        return false;
    }

    // Randomly order door locations
    auto const aDoorLocations = Util::RandomiseVector(roomA.GetDoorLocations());
    auto const bDoorLocations = Util::RandomiseVector(roomB.GetDoorLocations());

    // Keep checking each combination until it finds a valid room
    Transform const& transformA = roomA.GetGameObject().GetTransform();
    Transform& transformB = roomB.GetGameObject().GetTransform();

    for (const DoorLocation& aDoorLoc : aDoorLocations) {
        for (const DoorLocation& bDoorLoc : bDoorLocations) {
            Quaternion orientationDifference = Quaternion::VectorsToQuaternion(bDoorLoc.dir, -aDoorLoc.dir);
            // Enforce flipping around the Y axis (no tilting the rooms)
            if (fabs(orientationDifference.x) >= FLT_EPSILON || fabs(orientationDifference.z) >= FLT_EPSILON) continue;
            SetTransform(transformA, transformB, orientationDifference, aDoorLoc, bDoorLoc);
            auto info = CollisionDetection::CollisionInfo();

            if (!CollisionDetection::ObjectIntersection(&roomB.GetGameObject(), &GetGameObject(), info)) {
                GameObject* copiedPrefab = roomB.GetGameObject().CopyGameObject();
                RoomPrefab* roomPrefab = copiedPrefab->TryGetComponent<RoomPrefab>();

                SetNeighbours(roomPrefab);
                GetGameObject().AddChild(copiedPrefab);
                RoomManager::ReturnPrefab(&roomB.GetGameObject());
                return true;
            }
        }
    }
    // If no combination is valid, reset transform and return false
    RoomManager::ReturnPrefab(&roomB.GetGameObject());
    return false;
}
