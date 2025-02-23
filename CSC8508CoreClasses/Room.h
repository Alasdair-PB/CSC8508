#pragma once
#include <vector>

enum class RoomType {
    Start,
    Monster,
    Treasure,
    Boss,
    Normal
};

class Room {
public:
    int x, y;      // 房间左上角坐标
    int width, height;
    RoomType type;
    std::vector<Room*> neighbors;

    Room(int _x, int _y, int _w, int _h, RoomType _type = RoomType::Normal)
        : x(_x), y(_y), width(_w), height(_h), type(_type) {}
};


