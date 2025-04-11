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
    static GameObject* GetRandom();
    static GameObject* GetRandom(RoomType type);
    static void ReturnPrefab(GameObject* prefab);
    static void ClearPrefabs();

    [[nodiscard]] static unsigned int GetSeed() { return seed; }
    [[nodiscard]] static std::vector<std::string> const& GetPrefabPaths() { return prefabPaths; }

protected:
    static unsigned int seed; // TODO: This needs to be smarter

    static std::vector<std::string> prefabPaths;
    static std::unordered_map<RoomType, vector<GameObject*>> prefabsByType;
    static vector<GameObject*> prefabs;
    static GameObject* LoadPrefab(std::string path);
};

#endif //ROOMMANAGER_H
