//
// Contributors: Alfie
//

#ifndef ROOMMANAGER_H
#define ROOMMANAGER_H
#include "RoomPrefab.h"

class RoomManager {
public:
    /**
     * Loads room prefabs from save files
     */
    static void LoadPrefabs();

    [[nodiscard]] static std::vector<RoomPrefab*> const& GetPrefabs() { return prefabs; }

protected:
    static std::vector<RoomPrefab*> prefabs;
};

#endif //ROOMMANAGER_H
