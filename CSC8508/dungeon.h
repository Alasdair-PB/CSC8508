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

                /// <summary>
                /// initializes a random seed for dungeon
                /// </summary>
                /// <param name="width"></param>
                /// <param name="height"></param>
                Dungeon(int width, int height)
                    : gridWidth(width), gridHeight(height) {
                    std::srand(static_cast<unsigned>(std::time(nullptr)));
                }

                ~Dungeon() {
                    for (auto room : rooms)
                        delete room;
                }

                /// <summary>
                /// Generate rooms, parameters: room count, minimum size, maximum size
                /// If it is the first room, it can be set as the starting room
                /// If there is an overlap in rooms, delete the room
                /// </summary>
                /// <param name="roomCount"></param>
                /// <param name="minSize"></param>
                /// <param name="maxSize"></param>
                void generateRooms(int roomCount, int minSize, int maxSize) {
                    int attempts = 0;

                    while (rooms.size() < static_cast<size_t>(roomCount) && attempts < roomCount * 5) {
                        int w = minSize + std::rand() % (maxSize - minSize + 1);
                        int h = minSize + std::rand() % (maxSize - minSize + 1);
                        int x = std::rand() % (gridWidth - w);
                        int y = std::rand() % (gridHeight - h);

                        Room* newRoom = new Room(x, y, w, h);

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
                            delete newRoom;
                        }
                        attempts++;
                    }
                }

                /// <summary>
                /// Check if room a and room b have overlap (with a small margin)
                /// </summary>
                /// <param name="a"></param>
                /// <param name="b"></param>
                /// <returns></returns>
                bool checkOverlap(Room* a, Room* b) {
                    return !(a->x + a->width <= b->x || b->x + b->width <= a->x ||
                        a->y + a->height <= b->y || b->y + b->height <= a->y);
                }

                /// <summary>
                /// Simple connection of rooms: connect rooms in order of center point
                /// Sort rooms by center point x coordinate (or distance for MST)
                /// Connects each room to the next room in order
                /// Add them to each other's neighbor list
                /// Calls a function to creat corridor between rooms and record path in corridor map
                /// </summary>
                void connectRooms() {
                    if (rooms.empty()) return;

                    std::sort(rooms.begin(), rooms.end(), [](Room* a, Room* b) {
                        return a->center().first < b->center().first;
                        });

                    for (size_t i = 1; i < rooms.size(); ++i) {
                        Room* roomA = rooms[i - 1];
                        Room* roomB = rooms[i];

                        roomA->neighbors.push_back(roomB);
                        roomB->neighbors.push_back(roomA);

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

            /// <summary>
            /// collider: Box, Sphere, Capsule
            /// tags: e.g. "Room", "Corridor", "Door"
            /// </summary>
            struct Prefab {
                std::string name; 
                std::string modelPath;
                std::string collider; 
                bool isWalkable;         
                float width, height, depth; 
                std::vector<std::string> tags; 

                Prefab(std::string _name, std::string _model, std::string _collider,
                    bool _walkable, float w, float h, float d, std::vector<std::string> _tags)
                    : name(_name), modelPath(_model), collider(_collider), isWalkable(_walkable),
                    width(w), height(h), depth(d), tags(_tags) {
                }
            };

            class PrefabManager {
            private:
                std::vector<Prefab> prefabs;

            public:
                /// <summary>
                /// Loads prefab from a JSON configuration file
                /// </summary>
                void loadPrefabs() {
                    prefabs.push_back(Prefab("SmallRoom", "models/small_room.obj", "Box", true, 5, 3, 5, { "Room" }));
                    prefabs.push_back(Prefab("Corridor", "models/corridor.obj", "Box", true, 3, 3, 10, { "Corridor" }));
                    prefabs.push_back(Prefab("Door", "models/door.obj", "Box", true, 1, 2, 0.1, { "Door" }));
                    prefabs.push_back(Prefab("Table", "models/table.obj", "Box", false, 2, 1, 2, { "Furniture" }));
                }

                /// <summary>
                /// Sets room type as a Prefab
                /// </summary>
                /// <param name="tag"></param>
                /// <returns>Filtered Rooms</returns>
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