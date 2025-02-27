
#include <vector>
#include <cstdlib>
#include <ctime>

class Dungeon {
public:
    std::vector<Room*> rooms;  
    int gridWidth, gridHeight;

    Dungeon(int width, int height)
        : gridWidth(width), gridHeight(height) {
        std::srand(static_cast<unsigned>(std::time(nullptr))); /*initialize random seed*//*初始化随机种子*/
    }

    ~Dungeon() {
        for (auto room : rooms)
            delete room;
    }

    // 生成房间，参数：房间数量、最小尺寸、最大尺寸
    /* Generate rooms, parameters: room count, minimum size, maximum size */
    void generateRooms(int roomCount, int minSize, int maxSize) {
        int attempts = 0; // 尝试次数计数，避免死循环
        /* Try to generate rooms until the desired count is reached or too many attempts are made */
        while (rooms.size() < static_cast<size_t>(roomCount) && attempts < roomCount * 5) {
            int w = minSize + std::rand() % (maxSize - minSize + 1);
            int h = minSize + std::rand() % (maxSize - minSize + 1);
            int x = std::rand() % (gridWidth - w);
            int y = std::rand() % (gridHeight - h);

            Room* newRoom = new Room(x, y, w, h);

            // 如果是第一个房间，可以设为起始房间
            /* If it is the first room, it can be set as the starting room */
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
                /* If there is an overlap, delete the room */
            }
            attempts++;
        }
    }

    // 检查两个房间是否重叠
    /* Check if two rooms overlap */
    bool checkOverlap(Room* a, Room* b) {
        // 检测 a 与 b 是否有重叠（留有一定边距也可以）
        /* Check if a and b have overlap (with a small margin) */
        return !(a->x + a->width <= b->x || b->x + b->width <= a->x ||
            a->y + a->height <= b->y || b->y + b->height <= a->y);
    }

    // 简单连接房间：按照房间中心点顺序依次连接
    /* Simple connection of rooms: connect rooms in order of center point */
    void connectRooms() {
        if (rooms.empty()) return;

        // 对房间按照中心点 x 坐标排序（也可以根据距离进行 MST 最小生成树）
        /* Sort rooms by center point x coordinate (or distance for MST) */
        std::sort(rooms.begin(), rooms.end(), [](Room* a, Room* b) {
            return a->center().first < b->center().first;
            });

        // 按顺序连接每个房间与下一个房间
        /* Connect each room to the next room in order */
        for (size_t i = 1; i < rooms.size(); ++i) {
            Room* roomA = rooms[i - 1];
            Room* roomB = rooms[i];

            // 将它们互相加入邻接列表
            /* Add them to each other's neighbor list */
            roomA->neighbors.push_back(roomB);
            roomB->neighbors.push_back(roomA);

            // 这里可以调用生成走廊的函数，将两房间中心相连的路径记录下来
            /* Here, you can call a function to generate a corridor between the two rooms, and record the path in the corridor map */
            createCorridor(roomA, roomB);
        }
    }

   
    void createCorridor(Room* a, Room* b) {
        auto [ax, ay] = a->center();
        auto [bx, by] = b->center();
        std::cout << "Corridor between (" << ax << ", " << ay << ") and ("
            << bx << ", " << by << ")" << std::endl;
        
    }

   
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
