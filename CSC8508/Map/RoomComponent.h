//
// Contributors: Alfie
//

#ifndef ROOMCOMPONENT_H
#define ROOMCOMPONENT_H

#define MAX_ITEM_SPAWN_LOCATIONS 5

#include "Mesh.h"
#include "NavigationMesh.h"
#include "RoomPrefab.h"

using namespace NCL::CSC8508;
using namespace NCL::Rendering;

/**
 * Class that represents a physical room in the game. Its GameObject should ALWAYS be the direct child of a game object
 * with a DungeonComponent.
 */
class RoomComponent final : public IComponent {
public:
     RoomComponent(GameObject& gameObject, RoomPrefab* prefab)
        : IComponent(gameObject), prefab(prefab) {
    }

    RoomComponent* GenerateNew();

    [[nodiscard]] std::vector<RoomComponent*> GetNextDoorRooms() const { return nextDoorRooms; }
     /**
      * Gets the room's dungeon
      * @return Dungeon or NULLPTR if dungeon layout was not set up properly
      */
     [[nodiscard]] GameObject* GetDungeon() const { return this->GetGameObject().TryGetParent(); }

private:
    RoomPrefab* prefab;
    std::vector<Vector3> doorLocations = std::vector<Vector3>();
    std::vector<RoomComponent*> nextDoorRooms = std::vector<RoomComponent*>();
};

#endif //ROOMCOMPONENT_H
