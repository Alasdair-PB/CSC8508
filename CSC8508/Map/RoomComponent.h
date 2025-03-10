//
// Contributors: Alfie
//

#ifndef ROOMCOMPONENT_H
#define ROOMCOMPONENT_H

#define MAX_ITEM_SPAWN_LOCATIONS 5

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
    explicit RoomComponent(RoomPrefab* prefab, GameObject& gameObject) : IComponent(gameObject), prefab(prefab) { }

private:
    RoomPrefab* prefab;
    std::vector<Vector3> doorLocations = std::vector<Vector3>();
};

#endif //ROOMCOMPONENT_H
