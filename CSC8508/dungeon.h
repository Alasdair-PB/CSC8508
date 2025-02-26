
#include <vector>
#include <cstdlib>
#include <ctime>
#include <Room.h>


// 地下城类定义
class Dungeon {
public:
    std::vector<Room*> rooms;  // 存储所有生成的房间
    int gridWidth, gridHeight; // 地下城整体尺寸

    Dungeon(int width, int height)
        : gridWidth(width), gridHeight(height) {
        std::srand(static_cast<unsigned>(std::time(nullptr))); // 初始化随机种子
    }

    ~Dungeon() {
        for (auto room : rooms)
            delete room;
    }

    // 生成房间，参数：房间数量、最小尺寸、最大尺寸
    void generateRooms(int roomCount, int minSize, int maxSize) {
        int attempts = 0; // 尝试次数计数，避免死循环
        while (rooms.size() < static_cast<size_t>(roomCount) && attempts < roomCount * 5) {
            int w = minSize + std::rand() % (maxSize - minSize + 1);
            int h = minSize + std::rand() % (maxSize - minSize + 1);
            int x = std::rand() % (gridWidth - w);
            int y = std::rand() % (gridHeight - h);

            Room* newRoom = new Room(x, y, w, h);

            // 如果是第一个房间，可以设为起始房间
            if (rooms.empty()) {
                newRoom->type = RoomType::Start;
            }

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
                delete newRoom; // 重叠则删除房间
            }
            attempts++;
        }
    }

    // 检查两个房间是否重叠
    bool checkOverlap(Room* a, Room* b) {
        // 检测 a 与 b 是否有重叠（留有一定边距也可以）
        return !(a->x + a->width <= b->x || b->x + b->width <= a->x ||
            a->y + a->height <= b->y || b->y + b->height <= a->y);
    }

    // 简单连接房间：按照房间中心点顺序依次连接
    void connectRooms() {
        if (rooms.empty()) return;

        // 对房间按照中心点 x 坐标排序（也可以根据距离进行 MST 最小生成树）
        std::sort(rooms.begin(), rooms.end(), [](Room* a, Room* b) {
            return a->center().first < b->center().first;
            });

        // 按顺序连接每个房间与下一个房间
        for (size_t i = 1; i < rooms.size(); ++i) {
            Room* roomA = rooms[i - 1];
            Room* roomB = rooms[i];

            // 将它们互相加入邻接列表
            roomA->neighbors.push_back(roomB);
            roomB->neighbors.push_back(roomA);

            // 这里可以调用生成走廊的函数，将两房间中心相连的路径记录下来
            createCorridor(roomA, roomB);
        }
    }

    // 生成走廊（这里只是示意，实际可以根据需求画直线或 L 形走廊）
    void createCorridor(Room* a, Room* b) {
        auto [ax, ay] = a->center();
        auto [bx, by] = b->center();
        std::cout << "Corridor between (" << ax << ", " << ay << ") and ("
            << bx << ", " << by << ")" << std::endl;
        // 实际实现：可以将直线路径上的格子标记为走廊
    }

    // 打印整个地下城信息（房间信息及连接情况）
    void printDungeon() {
        std::cout << "Dungeon Info:" << std::endl;
        for (auto room : rooms) {
            room->print();
            if (!room->neighbors.empty()) {
                std::cout << "  Connected to: ";
                for (auto neighbor : room->neighbors) {
                    std::cout << "(" << neighbor->x << ", " << neighbor->y << ") ";
                }
                std::cout << std::endl;
            }
        }
    }
};
