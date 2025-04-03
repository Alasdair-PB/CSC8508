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
                void GenerateRooms(int roomCount, int minSize, int maxSize) {
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
                            if (CheckOverlap(newRoom, room)) {
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
                bool CheckOverlap(Room* a, Room* b) {
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
                void ConnectRooms() {
                    if (rooms.empty()) return;

                    std::sort(rooms.begin(), rooms.end(), [](Room* a, Room* b) {
                        return a->center().first < b->center().first;
                        });

                    for (size_t i = 1; i < rooms.size(); ++i) {
                        Room* roomA = rooms[i - 1];
                        Room* roomB = rooms[i];

                        roomA->neighbors.push_back(roomB);
                        roomB->neighbors.push_back(roomA);

                        CreateCorridor(roomA, roomB);
                    }
                }

                void CreateCorridor(Room* a, Room* b) {
                    auto [ax, ay] = a->center();
                    auto [bx, by] = b->center();
                }
            };
        }
	}
};