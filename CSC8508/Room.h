#pragma once
#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>

namespace NCL {
    namespace CSC8508 {
        namespace DungeonGen {

            
            enum class RoomType {
                Start,
                Monster,
                Treasure,
                Boss,
                Normal
            };
      

            // Room type definition
            
            /// <summary>
            /// Initiates Rooms and their type, as well as randomly generate their positions.
            /// Can get room size, generates a room map
            class Room {
            public:
                int x, y;           
                int width, height;  
                RoomType type;      
                std::vector<Room*> neighbors;

                Room(int _x, int _y, int _w, int _h, RoomType _type = RoomType::Normal)
                    : x(_x), y(_y), width(_w), height(_h), type(_type) {
                }

                std::pair<int, int> center() const {
                    return { x + width / 2, y + height / 2 };
                }

                void print() const {
                    std::cout << "Room (" << x << ", " << y << ") "
                        << "Size: " << width << "x" << height
                        << " Type: " << static_cast<int>(type) << std::endl;
                }

				int MainDun();
            };
        }
    }
};



