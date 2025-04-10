//
// Contributors: Alfie
//

#ifndef DUNGEONCOMPONENT_H
#define DUNGEONCOMPONENT_H

#define MAX_FAILURES 30

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
    }

    void GetAllItemSpawnLocations(std::vector<Vector3>& locations) const;

    [[nodiscard]] unsigned int GetSeed() const { return seed; }
    [[nodiscard]] std::vector<RoomPrefab*> GetRooms() const;

private:
    DoorLocation entrancePosition = DoorLocation(Vector3(0, 0, 0), Vector3(0, 0, -1));
    bool TryGenerateNewRoom(RoomPrefab& roomA, RoomPrefab& roomB) const;
    void SetTransform(const Transform& transformA, Transform& transformB, const Quaternion orientationDifference, 
        const DoorLocation aDoorLoc, const DoorLocation bDoorLoc) const;

    unsigned int seed = 14;
    int roomCount;
    bool GenerateRoom() const;
};

#endif //DUNGEONCOMPONENT_H
