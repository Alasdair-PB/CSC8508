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
        // ѡ��һ�����ʵķ���
        Prefab* roomPrefab = prefabManager.getRandomPrefabByTag("Room");
        if (roomPrefab) {
            std::cout << "Placing Room: " << roomPrefab->name << " at ("
                << bspNode->x << ", " << bspNode->y << ")\n";
        }

        // ���ܼӸ���
        if (rand() % 100 < 50) { // 50% ���ʷ���
            Prefab* doorPrefab = prefabManager.getRandomPrefabByTag("Door");
            if (doorPrefab) {
                std::cout << "Placing Door: " << doorPrefab->name << " in Room\n";
            }
        }
    }
}*/