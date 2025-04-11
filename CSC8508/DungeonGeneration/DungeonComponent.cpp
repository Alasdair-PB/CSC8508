//
// Contributors: Alfie & Alasdair
//

#include "DungeonComponent.h"
#include "CollisionDetection.h"
#include "RoomManager.h"
#include "INetworkDeltaComponent.h"
#include "../../CSC8508CoreClasses/Util.cpp"

bool DungeonComponent::Generate(int const roomCount) const {
    int failures = 0;
    std::srand(GetSeed());

    GameObject* prefab = RoomManager::GetRandom(Split);
    GameObject* copy = prefab->CopyGameObject();
    RoomManager::ReturnPrefab(prefab);
    RoomPrefab* roomPrefab = copy->TryGetComponent<RoomPrefab>();

    //SetEntranceTransform(roomPrefab, copy->GetTransform());
    GetGameObject().AddChild(copy);

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

void DungeonComponent::SetEntranceTransform(RoomPrefab* roomPrefab, Transform& entryTransform) const {
    DoorLocation const doorLoc = roomPrefab->GetDoorLocations().at(0);
    Quaternion const orientationDifference = Quaternion::VectorsToQuaternion(doorLoc.dir, -entrancePosition.dir);
    entryTransform.SetOrientation(orientationDifference);
    entryTransform.SetPosition(
        GetGameObject().GetTransform().GetPosition()
        + GetGameObject().GetTransform().GetOrientation() * entrancePosition.pos * GetGameObject().GetTransform().GetScale()
        - entryTransform.GetOrientation() * doorLoc.pos * entryTransform.GetScale()
    );
}

bool DungeonComponent::GenerateRoom() const {
    GameObject* roomB = RoomManager::GetRandom();
    RoomPrefab* roomPrefabInfo = roomB->TryGetComponent<RoomPrefab>();
    // 2: Randomly order the rooms and attempt to generate a new room in each until one succeeds
    for (auto const rooms = Util::RandomiseVector(GetRooms()); RoomPrefab* r : rooms) {
        if (TryGenerateNewRoom(*r, *roomPrefabInfo)) {
            RoomManager::ReturnPrefab(roomB);
            return true; 
        }
    }
    RoomManager::ReturnPrefab(roomB);
    return false;
}

bool DungeonComponent::EndDungeonPaths() {
    GameObject* roomB = RoomManager::GetRandom(Bank);
    RoomPrefab* roomPrefabInfo = roomB->TryGetComponent<RoomPrefab>();

    auto const x = GetRooms();
    for (auto const rooms = GetRooms(); RoomPrefab* r : rooms)
        ForceGenerateRooms(*r, *roomPrefabInfo);
    RoomManager::ReturnPrefab(roomB);
    return true;
}

std::vector<RoomPrefab*> DungeonComponent::GetRooms() const {
    std::vector<RoomPrefab*> out;

    for (GameObject const* c : GetGameObject().GetChildren())
        if (auto* component = c->TryGetComponent<RoomPrefab>()) out.push_back(component);
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

bool DungeonComponent::IsConnectedDoor(RoomPrefab& roomA, const DoorLocation& aDoorLoc) const {
    Vector3 doorPosA = roomA.GetGameObject().GetTransform().GetPosition() + roomA.GetGameObject().GetTransform().GetOrientation() * aDoorLoc.pos;
    for (const RoomPrefab* roomPrefab : roomA.GetNextDoorRooms()) {
        for (const DoorLocation& doorB : roomPrefab->GetDoorLocations()) {
            Vector3 doorPosB = roomPrefab->GetGameObject().GetTransform().GetPosition() + roomPrefab->GetGameObject().GetTransform().GetOrientation() * doorB.pos;
            if (doorPosA.x == doorPosB.x && doorPosA.y == doorPosB.y && doorPosA.z == doorPosB.z)
                return true;
        }
    }
    return false;
}

bool DungeonComponent::TryCorrectDungeon(RoomPrefab& roomB, const std::vector<DoorLocation> bDoorLocations, const Transform& transformA,
    Transform& transformB, const DoorLocation& aDoorLoc) const 
{
    for (const DoorLocation& bDoorLoc : bDoorLocations) {
        if (TryCopyRoomToDungeon(roomB, aDoorLoc, bDoorLoc, transformA, transformB))
            return true;
    }
    return false;
}

bool DungeonComponent::ForceGenerateRooms(RoomPrefab& roomA, RoomPrefab& roomB) const {
    if (roomA.GetNextDoorRooms().size() >= roomA.GetDoorLocations().size())
        return false;

    auto const aDoorLocations = roomA.GetDoorLocations();
    auto const bDoorLocations = roomB.GetDoorLocations();
    Transform const& transformA = roomA.GetGameObject().GetTransform();
    Transform& transformB = roomB.GetGameObject().GetTransform();
    bool correctedGeneration = true;

    for (const DoorLocation& aDoorLoc : aDoorLocations) {
        if (IsConnectedDoor(roomA, aDoorLoc))
            continue;
        correctedGeneration = TryCorrectDungeon(roomB, bDoorLocations, transformA, transformB, aDoorLoc);
    }
    return correctedGeneration;
}

bool DungeonComponent::TryCopyRoomToDungeon(RoomPrefab& roomB, const DoorLocation& aDoorLoc,
    const DoorLocation& bDoorLoc, Transform const& transformA, Transform& transformB) const
{
    Quaternion orientationDifference = Quaternion::VectorsToQuaternion(bDoorLoc.dir, -aDoorLoc.dir);
    // Enforce flipping around the Y axis (no tilting the rooms)
    if (fabs(orientationDifference.x) >= FLT_EPSILON || fabs(orientationDifference.z) >= FLT_EPSILON)
        return false;
    
    SetTransform(transformA, transformB, orientationDifference, aDoorLoc, bDoorLoc);
    auto info = CollisionDetection::CollisionInfo();

    if (!CollisionDetection::ObjectIntersection(&roomB.GetGameObject(), &GetGameObject(), info)) {
        GameObject* copiedPrefab = roomB.GetGameObject().CopyGameObject();
        RoomPrefab* roomPrefab = copiedPrefab->TryGetComponent<RoomPrefab>();
        SetNeighbours(roomPrefab);
        GetGameObject().AddChild(copiedPrefab);
        return true;
    }
    return false;
}

bool DungeonComponent::TryGenerateNewRoom(RoomPrefab& roomA, RoomPrefab& roomB) const {
    if (roomA.GetNextDoorRooms().size() >= roomA.GetDoorLocations().size())
        return false;

    auto const aDoorLocations = Util::RandomiseVector(roomA.GetDoorLocations());
    auto const bDoorLocations = Util::RandomiseVector(roomB.GetDoorLocations());
    Transform const& transformA = roomA.GetGameObject().GetTransform();
    Transform& transformB = roomB.GetGameObject().GetTransform();

    for (const DoorLocation& aDoorLoc : aDoorLocations) {
        if (IsConnectedDoor(roomA, aDoorLoc))
            continue;
        for (const DoorLocation& bDoorLoc : bDoorLocations) {
            if (TryCopyRoomToDungeon(roomB, aDoorLoc, bDoorLoc, transformA, transformB))
                return true;
        }
    }
    return false;
}
