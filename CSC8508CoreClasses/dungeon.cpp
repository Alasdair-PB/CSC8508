#include "C:/Users/12022/source/repos/CSC8508/CSC8508CoreClasses/CMakeFiles/CSC8508CoreClasses.dir/Debug/cmake_pch.hxx"
#include "dungeon.h"
#include <iostream>
#include <vector>
#include <cstdlib>

struct Room {
    int x, y, width, height;
};
/*BSP(Binary Space Partitioning)*/
struct Node {
    int x, y, width, height;
    Node* left = nullptr;
    Node* right = nullptr;
    Room* room = nullptr;

    Node(int _x, int _y, int _width, int _height)
        : x(_x), y(_y), width(_width), height(_height) {}

    bool split() {
        if (left || right) return false; // 已分割
        bool splitH = (rand() % 2 == 0); // 随机决定水平或垂直分割
        if (width > height) splitH = false; // 如果宽度更大，优先垂直分割
        if (height > width) splitH = true;  // 如果高度更大，优先水平分割

        int maxSize = (splitH ? height : width) - 10;
        if (maxSize <= 10) return false; // 区域太小，不再分割

        int splitPos = rand() % (maxSize - 10) + 5;

        if (splitH) {
            left = new Node(x, y, width, splitPos);
            right = new Node(x, y + splitPos, width, height - splitPos);
        }
        else {
            left = new Node(x, y, splitPos, height);
            right = new Node(x + splitPos, y, width - splitPos, height);
        }
        return true;
    }
};

// 递归分割
void generateBSP(Node* node) {
    if (rand() % 100 < 75) { // 75% 的几率继续分割
        if (node->split()) {
            generateBSP(node->left);
            generateBSP(node->right);
        }
    }
}

// 打印地图
void printDungeon(Node* node, char map[50][50]) {
    if (!node) return;
    if (node->left || node->right) {
        printDungeon(node->left, map);
        printDungeon(node->right, map);
    }
    else {
        for (int i = node->x; i < node->x + node->width; i++) {
            for (int j = node->y; j < node->y + node->height; j++) {
                map[i][j] = '.';
            }
        }
    }
}

/*int main() {
    srand(time(0));
    Node root(0, 0, 50, 50);
    generateBSP(&root);

    char map[50][50] = {};
    for (int i = 0; i < 50; i++)
        for (int j = 0; j < 50; j++)
            map[i][j] = '#';

    printDungeon(&root, map);

    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            std::cout << map[i][j];
        }
        std::cout << std::endl;
    }
    return 0;
}
*/