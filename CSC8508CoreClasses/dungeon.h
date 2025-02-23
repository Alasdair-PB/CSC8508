
#include <vector>
#include <cstdlib>
#include <ctime>

class Dungeon {
public:
    std::vector<Room*> rooms;
    int gridWidth, gridHeight;
    // 可用二维数组或 vector<vector<int>> 存储网格信息，表示墙体、走廊等

    Dungeon(int width, int height) : gridWidth(width), gridHeight(height) {
        std::srand(static_cast<unsigned>(std::time(nullptr))); // 初始化随机种子
    }

    ~Dungeon() {
        for (auto room : rooms)
            delete room;
    }

    // 方法：生成房间、连接房间、检查连通性、渲染地图等
    void generateRooms(int roomCount, int minSize, int maxSize);
    void connectRooms();
    bool checkOverlap(Room* a, Room* b);
    // 其他辅助方法……
};
