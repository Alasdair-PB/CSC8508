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
    struct RoomPrefabDataStruct : ISerializedData {
        std::vector<Vector3> possibleItemSpawnLocations;
        std::vector<DoorLocation> possibleDoorLocations;

        RoomPrefabDataStruct() : possibleItemSpawnLocations(std::vector<Vector3>()), possibleDoorLocations(std::vector<DoorLocation>()) { }
        RoomPrefabDataStruct(std::vector<Vector3> const& possibleItemSpawnLocations, std::vector<DoorLocation> const& possibleDoorLocations)
            : possibleItemSpawnLocations(possibleItemSpawnLocations), possibleDoorLocations(possibleDoorLocations) { }

        static auto GetSerializedFields() {
            return std::make_tuple(
                SERIALIZED_FIELD(RoomPrefabDataStruct, possibleItemSpawnLocations),
                SERIALIZED_FIELD(RoomPrefabDataStruct, possibleDoorLocations)
                );
        }
    };

    explicit RoomPrefab(
        GameObject& roomObject,
        std::vector<Vector3> const& possibleItemSpawnLocations = std::vector<Vector3>(),
        std::vector<DoorLocation> const& possibleDoorLocations = std::vector<DoorLocation>()
        )
    : IComponent(roomObject), possibleItemSpawnLocations(possibleItemSpawnLocations), possibleDoorLocations(possibleDoorLocations) { }

    [[nodiscard]] std::vector<Vector3> const& GetItemSpawnLocations() const { return possibleItemSpawnLocations; }
    [[nodiscard]] std::vector<DoorLocation> const& GetDoorLocations() const { return possibleDoorLocations; }

    size_t Save(std::string assetPath, size_t* allocationStart) override;
    void Load(std::string assetPath, size_t allocationStart) override;

protected:
    std::vector<Vector3> possibleItemSpawnLocations = std::vector<Vector3>();
    std::vector<DoorLocation> possibleDoorLocations = std::vector<DoorLocation>();
};

#endif //ROOMPREFAB_H
