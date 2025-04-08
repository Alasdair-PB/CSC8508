//
// Contributors: Alfie
//

#include "RoomManager.h"
#include "Assets.h"
#include "GameWorld.h"

std::vector<std::string> RoomManager::prefabPaths = std::vector<std::string>();

void RoomManager::LoadPrefabs(){
    prefabPaths.push_back(NCL::Assets::PFABDIR + "rooms/Default.pfab");
}

GameObject* RoomManager::GetRandom() {
    int const index = std::rand() % prefabPaths.size();
    return LoadPrefab(prefabPaths[index]);
}

GameObject* RoomManager::LoadPrefab(std::string path) {
    GameObject* myObjectToLoad = new GameObject(true);
    myObjectToLoad->Load(path);
    GameWorld::Instance().AddGameObject(myObjectToLoad);
    return myObjectToLoad; // TODO: Swap to continue when it's a loop
}

