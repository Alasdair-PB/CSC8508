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

            // ���巿������
            enum class RoomType {
                Start,
                Monster,
                Treasure,
                Boss,
                Normal
            };
      

            // �����ඨ��
            class Room {
            public:
                int x, y;           // �������Ͻ�����
                int width, height;  // ������
                RoomType type;      // ��������
                std::vector<Room*> neighbors; // �ڽӷ���ָ���б�

                Room(int _x, int _y, int _w, int _h, RoomType _type = RoomType::Normal)
                    : x(_x), y(_y), width(_w), height(_h), type(_type) {
                }

                // ��ȡ�������ĵ�
                std::pair<int, int> center() const {
                    return { x + width / 2, y + height / 2 };
                }

                // ��ӡ������Ϣ���������
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



