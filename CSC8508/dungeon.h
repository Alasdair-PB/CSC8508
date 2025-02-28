#include "Room.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>


namespace NCL {
    namespace CSC8508 {
        namespace DungeonGen{

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

            class Player
            {
            public:
                int posX, posY;

                Player(int x, int y) : posX(x), posY(y) {}

                void move(int dx, int dy) {
                    posX += dx;
                    posY += dy;
                    std::cout << "Player moved to (" << posX << ", " << posY << ")" << std::endl;
                }
            };

            struct Prefab {
                std::string name;        // 预设名称
                std::string modelPath;   // 3D 模型路径
                std::string collider;    // 碰撞体类型 (Box, Sphere, Capsule)
                bool isWalkable;         // 是否可行走（false 表示障碍物）
                float width, height, depth; // 大小
                std::vector<std::string> tags; // 额外的标记 (e.g. "Room", "Corridor", "Door")

                // 构造函数
                Prefab(std::string _name, std::string _model, std::string _collider,
                    bool _walkable, float w, float h, float d, std::vector<std::string> _tags)
                    : name(_name), modelPath(_model), collider(_collider), isWalkable(_walkable),
                    width(w), height(h), depth(d), tags(_tags) {
                }
            };

            class PrefabManager {
            private:
                std::vector<Prefab> prefabs; // 存储所有预设

            public:

                // 从文件加载 Prefab（假设用 JSON）
                void loadPrefabs() {
                    // 假设你有一个 JSON 解析库
                    prefabs.push_back(Prefab("SmallRoom", "models/small_room.obj", "Box", true, 5, 3, 5, { "Room" }));
                    prefabs.push_back(Prefab("Corridor", "models/corridor.obj", "Box", true, 3, 3, 10, { "Corridor" }));
                    prefabs.push_back(Prefab("Door", "models/door.obj", "Box", true, 1, 2, 0.1, { "Door" }));
                    prefabs.push_back(Prefab("Table", "models/table.obj", "Box", false, 2, 1, 2, { "Furniture" }));
                }

                // 按类型随机选择一个 Prefab
                Prefab* getRandomPrefabByTag(const std::string& tag) {
                    std::vector<Prefab*> filtered;
                    for (auto& p : prefabs) {
                        if (std::find(p.tags.begin(), p.tags.end(), tag) != p.tags.end()) {
                            filtered.push_back(&p);
                        }
                    }
                    if (filtered.empty()) return nullptr;
                    return filtered[rand() % filtered.size()];
                }

            };
        }
	}
};