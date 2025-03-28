//
// Contributors: Alfie
//

#include "RoomManager.h"

#include "Assets.h"

std::vector<std::string> RoomManager::prefabPaths = std::vector<std::string>();

void RoomManager::LoadPrefabs() {

    prefabPaths.push_back(NCL::Assets::PFABDIR + "rooms/RP_C.pfab");
    prefabPaths.push_back(NCL::Assets::PFABDIR + "rooms/RP_C.pfab");
}

RoomPrefab* RoomManager::GetRandom() {
    int const index = std::rand() % prefabPaths.size();
    return LoadPrefab(prefabPaths[index]);
}


RoomPrefab* RoomManager::LoadPrefab(std::string path) {
    auto* myObjectToLoad = new GameObject(true);
    myObjectToLoad->Load(path);
    auto* component = myObjectToLoad->TryGetComponent<RoomPrefab>();
    return component; // TODO: Swap to continue when it's a loop
}

