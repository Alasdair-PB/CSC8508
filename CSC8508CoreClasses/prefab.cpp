#include <prefab.h>
#include <iostream>
#include <cstdlib>

/*void generateDungeon(PrefabManager& prefabManager, Node* bspNode) {
    if (!bspNode) return;

    if (bspNode->left || bspNode->right) {
        generateDungeon(prefabManager, bspNode->left);
        generateDungeon(prefabManager, bspNode->right);
    }
    else {
        // 选择一个合适的房间
        Prefab* roomPrefab = prefabManager.getRandomPrefabByTag("Room");
        if (roomPrefab) {
            std::cout << "Placing Room: " << roomPrefab->name << " at ("
                << bspNode->x << ", " << bspNode->y << ")\n";
        }

        // 可能加个门
        if (rand() % 100 < 50) { // 50% 概率放门
            Prefab* doorPrefab = prefabManager.getRandomPrefabByTag("Door");
            if (doorPrefab) {
                std::cout << "Placing Door: " << doorPrefab->name << " in Room\n";
            }
        }
    }
}*/