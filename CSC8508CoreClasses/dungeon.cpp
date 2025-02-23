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
