//
// Contributors: Alfie
//

#ifndef ROOMPREFAB_H
#define ROOMPREFAB_H
#include "AABBVolume.h"
#include "NavigationMesh.h"
#include "BoundsComponent.h"

using namespace NCL::CSC8508;

class RoomPrefab {
public:

    struct DoorLocation {
        Vector3 const pos;
        Vector3 const dir;

        DoorLocation(Vector3 const position, Vector3 const direction)
            : pos(position), dir(direction) { }
    };


    RoomPrefab(GameObject* roomObject, NavigationMesh* navMesh) : roomObject(roomObject), navMesh(navMesh) { }

    [[nodiscard]] GameObject* GetRoomObject() const { return roomObject; }
    [[nodiscard]] NCL::AABBVolume GetEncasingVolume() const;
    [[nodiscard]] std::vector<DoorLocation> const& GetDoorLocations() const { return possibleDoorLocations; }

protected:
    GameObject* roomObject;
    NavigationMesh* navMesh;
    std::vector<Vector3> possibleItemSpawnLocations = std::vector<Vector3>(); // TODO: Replace with more spatially efficient structure
    std::vector<DoorLocation> possibleDoorLocations = std::vector<DoorLocation>();
};

#endif //ROOMPREFAB_H
