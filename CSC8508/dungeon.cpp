#include "dungeon.h"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

#include <algorithm>

using namespace NCL::CSC8508::DungeonGen;

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