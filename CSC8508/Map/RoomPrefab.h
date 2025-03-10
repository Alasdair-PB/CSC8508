//
// Contributors: Alfie
//

#ifndef ROOMPREFAB_H
#define ROOMPREFAB_H
#include "GameObject.h"
#include "NavigationMesh.h"

using namespace NCL::CSC8508;

/**
 * Class that represents a room prefab
 */
class RoomPrefab {
public:
    RoomPrefab(GameObject* roomObject, NavigationMesh* navMesh) : roomObject(roomObject), navMesh(navMesh) { }

protected:
    GameObject* roomObject;
    NavigationMesh* navMesh;
    std::vector<Vector3> possibleItemSpawnLocations = std::vector<Vector3>(); // TODO: Replace with more spatially efficient structure
    std::vector<Vector3> possibleDoorLocations = std::vector<Vector3>();
};

#endif //ROOMPREFAB_H
