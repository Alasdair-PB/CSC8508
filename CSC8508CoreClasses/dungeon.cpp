#include "C:/Users/12022/source/repos/CSC8508/CSC8508CoreClasses/CMakeFiles/CSC8508CoreClasses.dir/Debug/cmake_pch.hxx"
#include "dungeon.h"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

#include <algorithm>

bool Dungeon::checkOverlap(Room* a, Room* b) {
    return !(a->x + a->width < b->x || b->x + b->width < a->x ||
        a->y + a->height < b->y || b->y + b->height < a->y);
}

void Dungeon::generateRooms(int roomCount, int minSize, int maxSize) {
    for (int i = 0; i < roomCount; ++i) {
        int w = minSize + std::rand() % (maxSize - minSize + 1);
        int h = minSize + std::rand() % (maxSize - minSize + 1);
        int x = std::rand() % (gridWidth - w);
        int y = std::rand() % (gridHeight - h);

        Room* newRoom = new Room(x, y, w, h);
        bool overlap = false;
        for (auto room : rooms) {
            if (checkOverlap(newRoom, room)) {
                overlap = true;
                break;
            }
        }
        if (!overlap) {
            rooms.push_back(newRoom);
        }
        else {
            delete newRoom; // 如果重叠则丢弃该房间
        }
    }
}

void Dungeon::connectRooms() {
    // 可以按照房间中心点排序，然后依次连接
    for (size_t i = 1; i < rooms.size(); ++i) {
        Room* roomA = rooms[i - 1];
        Room* roomB = rooms[i];
        // 将 roomA 与 roomB 连接，比如将 roomB 加入 roomA 的邻接列表
        roomA->neighbors.push_back(roomB);
        roomB->neighbors.push_back(roomA);
        // 此处可以添加走廊绘制的逻辑，例如记录走廊在网格中的占用单元
    }
}
// 主函数：运行地下城生成示例
// ----------------------------
int creatroom() {
    // 设置地下城总体网格尺寸
    const int dungeonWidth = 100;
    const int dungeonHeight = 100;

    // 创建地下城对象
    Dungeon dungeon(dungeonWidth, dungeonHeight);

    // 生成 10 个房间，房间尺寸在 5 到 15 之间
    dungeon.generateRooms(10, 5, 15);

    // 连接房间（生成走廊）
    dungeon.connectRooms();

    // 打印地下城信息
    dungeon.printDungeon();

    // 创建玩家并放置在起始房间中心（假定第一个房间为起始房间）
    if (!dungeon.rooms.empty()) {
        auto [startX, startY] = dungeon.rooms.front()->center();
        Player player(startX, startY);
        std::cout << "Player starting at (" << startX << ", " << startY << ")" << std::endl;

        // 示例：玩家移动
        player.move(1, 0);
        player.move(0, 1);
    }

    // 运行完毕，等待用户输入后退出
    std::cout << "Press any key to exit..." << std::endl;
    std::cin.get();
    return 0;
}