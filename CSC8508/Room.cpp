#include "Room.h"
#include "Dungeon.h"
#include "GameObject.h"

using namespace NCL::CSC8508::DungeonGen;

/// <summary>
/// Creates a dungeon object using preset dungeonWidth and dungeonHeight. 
/// This then generates rooms connected via corridors to create generated map.
/// Currently spawns player in first room and has test movement
/// </summary>
/// <returns>Int 0</returns>
int Room::MainDun() {

    const int dungeonWidth = 100;
    const int dungeonHeight = 100;

    Dungeon dungeon(dungeonWidth, dungeonHeight);

    dungeon.GenerateRooms(10, 5, 15);
    dungeon.ConnectRooms();
    return 0;
}