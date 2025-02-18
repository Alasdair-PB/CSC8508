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
    std::vector<Prefab> prefabs; // 存储所有预设

public:
    void loadPrefabs(); // 加载预设（从 JSON / 二进制文件）
    Prefab* getRandomPrefabByTag(const std::string& tag); // 按类别随机获取
};

// 从文件加载 Prefab（假设用 JSON）
void PrefabManager::loadPrefabs() {
    // 假设你有一个 JSON 解析库
    prefabs.push_back(Prefab("SmallRoom", "models/small_room.obj", "Box", true, 5, 3, 5, { "Room" }));
    prefabs.push_back(Prefab("Corridor", "models/corridor.obj", "Box", true, 3, 3, 10, { "Corridor" }));
    prefabs.push_back(Prefab("Door", "models/door.obj", "Box", true, 1, 2, 0.1, { "Door" }));
    prefabs.push_back(Prefab("Table", "models/table.obj", "Box", false, 2, 1, 2, { "Furniture" }));
}

// 按类型随机选择一个 Prefab
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
