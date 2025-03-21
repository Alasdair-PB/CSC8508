//
// Contributors: Alfie
//

#ifndef DUNGEONCOMPONENT_H
#define DUNGEONCOMPONENT_H

#include <utility>

#include "DoorLocation.h"
#include "IComponent.h"

using namespace NCL::CSC8508;

class RoomPrefab;

class DungeonComponent final : public IComponent {
public:
    explicit DungeonComponent(GameObject& gameObject) : IComponent(gameObject) { }

    DungeonComponent(GameObject& gameObject, DoorLocation entrancePosition)
        : IComponent(gameObject), entrancePosition(std::move(entrancePosition)) { }

    /**
     * Procedurally generates rooms to fill the dungeon
     */
    void Generate();

    [[nodiscard]] unsigned int GetSeed() const { return seed; }

private:
    DoorLocation entrancePosition = DoorLocation(Vector3(0, 0, 0), Vector3(0, 0, -1));

    unsigned int seed = 14; // TODO: This needs to be smarter

    // /**
    //  * Attempts to generate an attempted prefab room at a door location
    //  * @param prefab Attempted RoomPrefab
    //  * @param doorLocation Attempted world-space DoorLocation
    //  * @return RoomComponent if successful, NULLPTR if not
    //  */
    // RoomComponent* TryGenerateRoom(RoomPrefab const& prefab, DoorLocation const& doorLocation);
};

#endif //DUNGEONCOMPONENT_H
