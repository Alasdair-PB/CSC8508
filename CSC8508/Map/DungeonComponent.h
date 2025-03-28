//
// Contributors: Alfie
//

#ifndef DUNGEONCOMPONENT_H
#define DUNGEONCOMPONENT_H

#define MAX_FAILURES 30

#include "DoorLocation.h"
#include "IComponent.h"
#include "RoomComponent.h"

using namespace NCL::CSC8508;

class RoomPrefab;

class DungeonComponent final : public IComponent {
public:
    explicit DungeonComponent(GameObject& gameObject) : IComponent(gameObject) { }

    DungeonComponent(GameObject& gameObject, DoorLocation const& entrancePosition)
        : IComponent(gameObject), entrancePosition(entrancePosition) { }

    /**
     * Procedurally generates rooms to fill the dungeon
     * @param roomCount int number of rooms to generate in this dungeon
     */
    void Generate(int roomCount) const;

    void GetAllItemSpawnLocations(std::vector<Vector3>& locations) const;

    [[nodiscard]] unsigned int GetSeed() const { return seed; }
    [[nodiscard]] std::vector<RoomComponent*> GetRooms() const;

private:
    DoorLocation entrancePosition = DoorLocation(Vector3(0, 0, 0), Vector3(0, 0, -1));

    unsigned int seed = 14; // TODO: This needs to be smarter

    bool GenerateRoom() const;
};

#endif //DUNGEONCOMPONENT_H
