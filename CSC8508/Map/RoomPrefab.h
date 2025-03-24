//
// Contributors: Alfie
//

#ifndef ROOMPREFAB_H
#define ROOMPREFAB_H

#include "AABBVolume.h"
#include "NavigationMesh.h"
#include "BoundsComponent.h"
#include "DoorLocation.h"

using namespace NCL::CSC8508;

class RoomPrefab {
public:
    RoomPrefab(GameObject* roomObject, NavigationMesh* navMesh) : roomObject(roomObject), navMesh(navMesh) { }

    [[nodiscard]] GameObject* GetRoomObject() const { return roomObject; }
    [[nodiscard]] NCL::AABBVolume GetEncasingVolume() const;
    [[nodiscard]] std::vector<DoorLocation> const& GetDoorLocations() const { return possibleDoorLocations; }

protected:
    GameObject* roomObject;
    NavigationMesh* navMesh;
    std::vector<Vector3> possibleItemSpawnLocations = std::vector<Vector3>();
    std::vector<DoorLocation> possibleDoorLocations = std::vector<DoorLocation>();
};

#endif //ROOMPREFAB_H
