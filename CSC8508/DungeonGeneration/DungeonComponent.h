//
// Contributors: Alfie & Alasdair
//

#ifndef DUNGEONCOMPONENT_H
#define DUNGEONCOMPONENT_H

#define MAX_FAILURES 30
#include "RoomManager.h"

#include "DoorLocation.h"
#include "IComponent.h"

using namespace NCL::CSC8508;

class RoomPrefab;

class DungeonComponent final : public IComponent {
public:
    DungeonComponent(GameObject& gameObject) : IComponent(gameObject), roomCount(12) { }
    DungeonComponent(GameObject& gameObject, DoorLocation const& entrancePosition, int roomCount)
        : IComponent(gameObject), entrancePosition(entrancePosition), roomCount(roomCount) { }

    /**
     * Procedurally generates rooms to fill the dungeon
     * @param roomCount int number of rooms to generate in this dungeon
     */
    bool Generate(int roomCount) const;

    void OnAwake() override {
        Generate(roomCount);
        EndDungeonPaths();
        RoomManager::ClearPrefabs();
    }

    void GetAllItemSpawnLocations(std::vector<Vector3>& locations) const;

    [[nodiscard]] unsigned int GetSeed() const { return seed; }
    [[nodiscard]] std::vector<RoomPrefab*> GetRooms() const;

private:
    DoorLocation entrancePosition = DoorLocation(Vector3(0, 0, 0), Vector3(0, 0, -1));
    bool TryGenerateNewRoom(RoomPrefab& roomA, RoomPrefab& roomB) const;
    bool ForceGenerateRooms(RoomPrefab& roomA, RoomPrefab& roomB) const;
    void SetTransform(const Transform& transformA, Transform& transformB, const Quaternion orientationDifference, 
        const DoorLocation aDoorLoc, const DoorLocation bDoorLoc) const;

    void SetEntranceTransform(RoomPrefab* roomPrefab, Transform& transformA) const;
    void SetNeighbours(RoomPrefab* roomPrefab) const;
    unsigned int seed = 4;
    int roomCount;
    bool GenerateRoom() const;
    bool EndDungeonPaths();
    bool IsConnectedDoor(RoomPrefab& roomA, const DoorLocation& aDoorLoc) const;
    bool TryCopyRoomToDungeon(RoomPrefab& roomB, const DoorLocation& aDoorLoc, const DoorLocation& bDoorLoc, 
        Transform const& transformA, Transform& transformB) const;
    bool TryCorrectDungeon(RoomPrefab& roomB, const std::vector<DoorLocation> bDoorLocations, const Transform& transformA,
        Transform& transformB, const DoorLocation& aDoorLoc) const;
    const float threshold = 0.2f;
};

#endif //DUNGEONCOMPONENT_H
