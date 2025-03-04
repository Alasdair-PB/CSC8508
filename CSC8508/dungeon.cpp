#include "dungeon.h"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

#include <algorithm>

using namespace NCL::CSC8508::DungeonGen;

/// <summary>
/// Creates a dungeon object using preset dungeonWidth and dungeonHeight. 
/// This then generates rooms connected via corridors to create generated map.
/// Currently spawns player in first room and has test movement
/// </summary>
/// <returns>Int 0</returns>
int CreateRoom() {

    const int dungeonWidth = 100;
    const int dungeonHeight = 100;

    Dungeon dungeon(dungeonWidth, dungeonHeight);

    dungeon.generateRooms(10, 5, 15);
    dungeon.connectRooms();
    dungeon.printDungeon();

    if (!dungeon.rooms.empty()) {
        auto [startX, startY] = dungeon.rooms.front()->center();
        Player player(startX, startY);
        std::cout << "Player starting at (" << startX << ", " << startY << ")" << std::endl;

        player.move(1, 0);
        player.move(0, 1);
    }

    std::cout << "Press any key to exit..." << std::endl;
    std::cin.get();
    return 0;
}