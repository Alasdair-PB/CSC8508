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

    struct RoomPrefabDataStruct;

    struct SpawnLocation {
        int probability;
        Vector3 location;
    };

    explicit RoomPrefab(
        GameObject& roomObject,
        std::vector<Vector3> const& possibleItemSpawnLocations = std::vector<Vector3>(),
        std::vector<DoorLocation> const& doorLocations = std::vector<DoorLocation>()
        )
    : IComponent(roomObject), 
        possibleItemSpawnLocations(possibleItemSpawnLocations), 
        doorLocations(doorLocations),
        required(false) {}

    [[nodiscard]] std::vector<Vector3> const& GetItemSpawnLocations() const { return possibleItemSpawnLocations; }
    [[nodiscard]] std::vector<DoorLocation> const& GetDoorLocations() const { return doorLocations; }

    size_t Save(std::string assetPath, size_t* allocationStart) override;
    void Load(std::string assetPath, size_t allocationStart) override;
    void PushIComponentElementsInspector(UIElementsGroup& elementsGroup, float scale) override;

protected:
    std::vector<Vector3> possibleItemSpawnLocations = std::vector<Vector3>();
    std::vector<DoorLocation> doorLocations = std::vector<DoorLocation>();

    std::vector<SpawnLocation> itemSpawnLocations = std::vector<SpawnLocation>();
    std::vector<SpawnLocation> enemySpawnLocations = std::vector<SpawnLocation>();
    bool required;
    int spawnProbability;
};

#endif //ROOMPREFAB_H
