//
// Contributors: Alfie
//

#ifndef ROOMPREFAB_H
#define ROOMPREFAB_H
#include "AABBVolume.h"
#include "GameObject.h"
#include "NavigationMesh.h"

using namespace NCL::CSC8508;

class RoomPrefab {
public:
    RoomPrefab(GameObject* roomObject, NavigationMesh* navMesh) : roomObject(roomObject), navMesh(navMesh) { }

    [[nodiscard]] NCL::AABBVolume GetEncasingVolume() const;

protected:
    GameObject* roomObject;
    NavigationMesh* navMesh;
    std::vector<Vector3> possibleItemSpawnLocations = std::vector<Vector3>(); // TODO: Look at spatially efficient structures
    std::vector<Vector3> possibleDoorLocations = std::vector<Vector3>();
};

#endif //ROOMPREFAB_H
