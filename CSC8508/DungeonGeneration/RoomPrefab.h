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

    explicit RoomPrefab(
        GameObject& roomObject,
        std::vector<SpawnLocation> const& itemSpawnLocations = std::vector<SpawnLocation>(),
        std::vector<SpawnLocation> const& enemySpawnLocations = std::vector<SpawnLocation>(),
        std::vector<DoorLocation> const& doorLocations = std::vector<DoorLocation>(),
        RoomType roomType = Empty,
        float spawnProbability = 1
        )
    : IComponent(roomObject), 
        itemSpawnLocations(itemSpawnLocations),
        enemySpawnLocations(enemySpawnLocations),
        spawnProbability(spawnProbability),
        doorLocations(doorLocations),
        roomType(roomType)
    {}

    [[nodiscard]] std::vector<SpawnLocation> const& GetItemSpawnLocations() const { return itemSpawnLocations; }
    [[nodiscard]] std::vector<DoorLocation> const& GetDoorLocations() const { return doorLocations; }

    size_t Save(std::string assetPath, size_t* allocationStart) override;
    void Load(std::string assetPath, size_t allocationStart) override;
    void PushIComponentElementsInspector(UIElementsGroup& elementsGroup, float scale) override;

protected:
    std::vector<DoorLocation> doorLocations = std::vector<DoorLocation>();
    std::vector<SpawnLocation> itemSpawnLocations = std::vector<SpawnLocation>();
    std::vector<SpawnLocation> enemySpawnLocations = std::vector<SpawnLocation>();

    RoomType roomType;
    float spawnProbability;
};
#endif //ROOMPREFAB_H
