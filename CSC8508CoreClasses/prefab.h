#pragma once

#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"
#include "PositionConstraint.h"
#include "OrientationConstraint.h"



using namespace NCL;
using namespace CSC8508;
class PrefabManager {
private:
    std::vector<Prefab> prefabs; // �洢����Ԥ��

public:
    void loadPrefabs(); // ����Ԥ�裨�� JSON / �������ļ���
    Prefab* getRandomPrefabByTag(const std::string& tag); // ����������ȡ
};

// ���ļ����� Prefab�������� JSON��
void PrefabManager::loadPrefabs() {
    // ��������һ�� JSON ������
    prefabs.push_back(Prefab("SmallRoom", "models/small_room.obj", "Box", true, 5, 3, 5, { "Room" }));
    prefabs.push_back(Prefab("Corridor", "models/corridor.obj", "Box", true, 3, 3, 10, { "Corridor" }));
    prefabs.push_back(Prefab("Door", "models/door.obj", "Box", true, 1, 2, 0.1, { "Door" }));
    prefabs.push_back(Prefab("Table", "models/table.obj", "Box", false, 2, 1, 2, { "Furniture" }));
}

// ���������ѡ��һ�� Prefab
Prefab* PrefabManager::getRandomPrefabByTag(const std::string& tag) {
    std::vector<Prefab*> filtered;
    for (auto& p : prefabs) {
        if (std::find(p.tags.begin(), p.tags.end(), tag) != p.tags.end()) {
            filtered.push_back(&p);
        }
    }
    if (filtered.empty()) return nullptr;
    return filtered[rand() % filtered.size()];
}
