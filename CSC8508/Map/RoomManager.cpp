//
// Contributors: Alfie
//

#include "RoomManager.h"

#include "Assets.h"

std::vector<std::string> RoomManager::prefabPaths = std::vector<std::string>();

void RoomManager::LoadPrefabs() {

    // TODO: Make this loop and check for any files in the folder
    std::string const pfabPath = NCL::Assets::PFABDIR + "rooms/RP_A.pfab";
    prefabPaths.push_back(pfabPath);
}

RoomPrefab* RoomManager::GetRandom() {
    int const index = std::rand() % prefabPaths.size();
    return LoadPrefab(prefabPaths[index]);
}


RoomPrefab* RoomManager::LoadPrefab(std::string path) {
    auto* myObjectToLoad = new GameObject();
    myObjectToLoad->Load(path);
    auto* component = myObjectToLoad->TryGetComponent<RoomPrefab>();
    return component; // TODO: Swap to continue when it's a loop
}

