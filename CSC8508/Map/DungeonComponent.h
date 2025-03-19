//
// Contributors: Alfie
//

#ifndef DUNGEONCOMPONENT_H
#define DUNGEONCOMPONENT_H

#include "IComponent.h"
#include "RoomComponent.h"

using namespace NCL::CSC8508;

class DungeonComponent final : public IComponent {
public:
    explicit DungeonComponent(GameObject& gameObject) : IComponent(gameObject) { }

    DungeonComponent(GameObject& gameObject, RoomPrefab::DoorLocation const& entrancePosition)
        : IComponent(gameObject), entrancePosition(entrancePosition) { }

    /**
     * Procedurally generates rooms to fill the dungeon
     */
    void Generate();

private:
    std::vector<GameObject*> rooms; // TODO: Maybe replace with RoomComponent? Whichever becomes more helpful

    RoomPrefab::DoorLocation entrancePosition = RoomPrefab::DoorLocation(Vector3(0, 0, 0), Vector3(0, 0, -1));

    /**
     * Attempts to generate an attempted prefab room at a door location
     * @param prefab Attempted RoomPrefab
     * @param doorLocation Attempted world-space DoorLocation
     * @return RoomComponent if successful, NULLPTR if not
     */
    RoomComponent* TryGenerateRoom(RoomPrefab const& prefab, RoomPrefab::DoorLocation const& doorLocation);
};

#endif //DUNGEONCOMPONENT_H
