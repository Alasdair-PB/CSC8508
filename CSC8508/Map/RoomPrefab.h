//
// Contributors: Alfie
//

#ifndef ROOMPREFAB_H
#define ROOMPREFAB_H

#include "NavigationMesh.h"
#include "BoundsComponent.h"
#include "DoorLocation.h"

using namespace NCL::CSC8508;

class RoomPrefab : public IComponent {
public:
    RoomPrefab(GameObject& roomObject, NavigationMesh* navMesh) : IComponent(roomObject), navMesh(navMesh) { }

    [[nodiscard]] std::vector<DoorLocation> const& GetDoorLocations() const { return possibleDoorLocations; }

protected:
    NavigationMesh* navMesh;
    std::vector<Vector3> possibleItemSpawnLocations = std::vector<Vector3>();
    std::vector<DoorLocation> possibleDoorLocations = std::vector<DoorLocation>();
};

#endif //ROOMPREFAB_H
