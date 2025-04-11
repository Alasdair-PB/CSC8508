//
// Contributors: Alfie
//

#include "RoomManager.h"
#include "Assets.h"
#include "GameWorld.h"

std::vector<GameObject*> RoomManager::prefabs = std::vector<GameObject*>();
std::unordered_map<RoomType, vector<GameObject*>> RoomManager::prefabsByType = std::unordered_map<RoomType, vector<GameObject*>>();
std::vector<std::string> RoomManager::prefabPaths = std::vector<std::string>();

void RoomManager::LoadPrefabs(){
    prefabPaths.push_back(NCL::Assets::PFABDIR + "rooms/RoomA.pfab");
    prefabPaths.push_back(NCL::Assets::PFABDIR + "rooms/RoomB.pfab");
    prefabPaths.push_back(NCL::Assets::PFABDIR + "rooms/RoomC.pfab");
    prefabPaths.push_back(NCL::Assets::PFABDIR + "rooms/RoomD.pfab");
    prefabPaths.push_back(NCL::Assets::PFABDIR + "rooms/RoomE.pfab");
    prefabPaths.push_back(NCL::Assets::PFABDIR + "rooms/DepositRoom.pfab");
    prefabPaths.push_back(NCL::Assets::PFABDIR + "rooms/ExitRoom.pfab");
    prefabPaths.push_back(NCL::Assets::PFABDIR + "rooms/DropOffRoom.pfab");

    for (std::string path : prefabPaths) {
        GameObject* prefab = LoadPrefab(path);
        RoomType roomType = prefab->TryGetComponent<RoomPrefab>()->GetRoomType();
        prefabs.push_back(prefab);
        prefabsByType[roomType].push_back(prefab);
    }
}

void RoomManager::ReturnPrefab(GameObject* prefab) {
    prefab->GetTransform().SetPosition(Vector3(-1000,-1000,-1000));
}

void RoomManager::ClearPrefabs() {
    for (GameObject* prefab : prefabs) {
        prefab->GetTransform().SetPosition(Vector3(-1000, -1000, -1000));
    }
}

GameObject* RoomManager::GetRandom(RoomType type) {
    if (prefabsByType[type].size() == 0)
        return nullptr;
    int const index = std::rand() % prefabsByType[type].size();
    GameObject* randomPrefab = prefabsByType[type][index];
    randomPrefab->SetEnabled(true);
    randomPrefab->GetTransform().SetPosition(Vector3());
    randomPrefab->GetTransform().SetOrientation(Quaternion());
    return randomPrefab;
}

GameObject* RoomManager::GetRandom() {
    int const index = std::rand() % prefabs.size();
    GameObject* randomPrefab = prefabs[index];
    randomPrefab->SetEnabled(true);
    randomPrefab->GetTransform().SetPosition(Vector3());
    randomPrefab->GetTransform().SetOrientation(Quaternion());
    return randomPrefab;
}

GameObject* RoomManager::LoadPrefab(std::string path) {
    GameObject* myObjectToLoad = new GameObject(true);
    myObjectToLoad->Load(path);
    GameWorld::Instance().AddGameObject(myObjectToLoad);
    return myObjectToLoad;
}

