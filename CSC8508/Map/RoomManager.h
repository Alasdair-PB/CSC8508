//
// Contributors: Alfie
//

#ifndef ROOMMANAGER_H
#define ROOMMANAGER_H
#include "RoomPrefab.h"

class RoomManager {
public:
    /**
     * Loads room prefabs from save files and seeds the rand() function with the current time (so random prefab's can
     * be grabbed)
     */
    static void LoadPrefabs();

    /**
     * Gets a random room prefab
     * @return Random prefab
     */
    static RoomPrefab* GetRandom();

    [[nodiscard]] static std::vector<RoomPrefab*> const& GetPrefabs() { return prefabs; }

protected:
    static std::vector<RoomPrefab*> prefabs;
};

#endif //ROOMMANAGER_H
