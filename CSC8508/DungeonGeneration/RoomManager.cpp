//
// Contributors: Alfie
//

#include "RoomManager.h"
#include "Assets.h"
#include "GameWorld.h"

std::vector<std::string> RoomManager::prefabPaths = std::vector<std::string>();

void RoomManager::LoadPrefabs(){
    prefabPaths.push_back(NCL::Assets::PFABDIR + "rooms/RoomA.pfab");
    prefabPaths.push_back(NCL::Assets::PFABDIR + "rooms/RoomB.pfab");
    prefabPaths.push_back(NCL::Assets::PFABDIR + "rooms/RoomC.pfab");
    prefabPaths.push_back(NCL::Assets::PFABDIR + "rooms/DepositRoom.pfab");
    prefabPaths.push_back(NCL::Assets::PFABDIR + "rooms/ExitRoom.pfab");
    prefabPaths.push_back(NCL::Assets::PFABDIR + "rooms/DropOffRoom.pfab");
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

