#include "C:/Users/12022/source/repos/CSC8508/CSC8508CoreClasses/CMakeFiles/CSC8508CoreClasses.dir/Debug/cmake_pch.hxx"
#include "Room.h"

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
