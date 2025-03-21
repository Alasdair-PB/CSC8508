//
// Contributors: Alfie
//

#ifndef ROOMCOMPONENT_H
#define ROOMCOMPONENT_H

#define MAX_ITEM_SPAWN_LOCATIONS 5

#include "DungeonComponent.h"
#include "IComponent.h"
#include "Mesh.h"
#include "NavigationMesh.h"
#include "RoomPrefab.h"

using namespace NCL::CSC8508;
using namespace NCL::Rendering;

/**
 * Class that represents a physical room in the game.
 */
class RoomComponent final : public IComponent {
public:
    explicit RoomComponent(GameObject& gameObject, RoomPrefab* prefab, DungeonComponent* dungeon)
        : IComponent(gameObject), prefab(prefab), dungeon(dungeon) {
        std::srand(dungeon->GetSeed());
    }

    RoomComponent* GenerateNew();

    [[nodiscard]] std::vector<RoomComponent*> GetNextDoorRooms() const { return nextDoorRooms; }

private:
    RoomPrefab* prefab;
    DungeonComponent* dungeon;
    std::vector<Vector3> doorLocations = std::vector<Vector3>();
    std::vector<RoomComponent*> nextDoorRooms = std::vector<RoomComponent*>();
};

#endif //ROOMCOMPONENT_H
