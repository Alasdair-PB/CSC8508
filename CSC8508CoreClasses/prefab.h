#pragma once
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"
#include "PositionConstraint.h"
#include "OrientationConstraint.h"


struct Prefab {
    std::string name;        // Ԥ������
    std::string modelPath;   // 3D ģ��·��
    std::string collider;    // ��ײ������ (Box, Sphere, Capsule)
    bool isWalkable;         // �Ƿ�����ߣ�false ��ʾ�ϰ��
    float width, height, depth; // ��С
    std::vector<std::string> tags; // ����ı�� (e.g. "Room", "Corridor", "Door")

    // ���캯��
    Prefab(std::string _name, std::string _model, std::string _collider,
        bool _walkable, float w, float h, float d, std::vector<std::string> _tags)
        : name(_name), modelPath(_model), collider(_collider), isWalkable(_walkable),
        width(w), height(h), depth(d), tags(_tags) {}
};

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

