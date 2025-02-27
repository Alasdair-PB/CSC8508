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

            // 定义房间类型
            enum class RoomType {
                Start,
                Monster,
                Treasure,
                Boss,
                Normal
            };
      

            // 房间类定义
            class Room {
            public:
                int x, y;           // 房间左上角坐标
                int width, height;  // 房间宽高
                RoomType type;      // 房间类型
                std::vector<Room*> neighbors; // 邻接房间指针列表

                Room(int _x, int _y, int _w, int _h, RoomType _type = RoomType::Normal)
                    : x(_x), y(_y), width(_w), height(_h), type(_type) {
                }

                // 获取房间中心点
                std::pair<int, int> center() const {
                    return { x + width / 2, y + height / 2 };
                }

                // 打印房间信息，方便调试
                void print() const {
                    std::cout << "Room (" << x << ", " << y << ") "
                        << "Size: " << width << "x" << height
                        << " Type: " << static_cast<int>(type) << std::endl;
                }

				int maindun();
            };
        }
    }
};



