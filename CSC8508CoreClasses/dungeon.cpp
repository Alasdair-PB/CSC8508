#include "C:/Users/12022/source/repos/CSC8508/CSC8508CoreClasses/CMakeFiles/CSC8508CoreClasses.dir/Debug/cmake_pch.hxx"
#include "dungeon.h"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

#include <algorithm>

bool Dungeon::checkOverlap(Room* a, Room* b) {
    return !(a->x + a->width < b->x || b->x + b->width < a->x ||
        a->y + a->height < b->y || b->y + b->height < a->y);
}

void Dungeon::generateRooms(int roomCount, int minSize, int maxSize) {
    for (int i = 0; i < roomCount; ++i) {
        int w = minSize + std::rand() % (maxSize - minSize + 1);
        int h = minSize + std::rand() % (maxSize - minSize + 1);
        int x = std::rand() % (gridWidth - w);
        int y = std::rand() % (gridHeight - h);

        Room* newRoom = new Room(x, y, w, h);
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
            delete newRoom; // ����ص������÷���
        }
    }
}

void Dungeon::connectRooms() {
    // ���԰��շ������ĵ�����Ȼ����������
    for (size_t i = 1; i < rooms.size(); ++i) {
        Room* roomA = rooms[i - 1];
        Room* roomB = rooms[i];
        // �� roomA �� roomB ���ӣ����罫 roomB ���� roomA ���ڽ��б�
        roomA->neighbors.push_back(roomB);
        roomB->neighbors.push_back(roomA);
        // �˴�����������Ȼ��Ƶ��߼��������¼�����������е�ռ�õ�Ԫ
    }
}
// �����������е��³�����ʾ��
// ----------------------------
int creatroom() {
    // ���õ��³���������ߴ�
    const int dungeonWidth = 100;
    const int dungeonHeight = 100;

    // �������³Ƕ���
    Dungeon dungeon(dungeonWidth, dungeonHeight);

    // ���� 10 �����䣬����ߴ��� 5 �� 15 ֮��
    dungeon.generateRooms(10, 5, 15);

    // ���ӷ��䣨�������ȣ�
    dungeon.connectRooms();

    // ��ӡ���³���Ϣ
    dungeon.printDungeon();

    // ������Ҳ���������ʼ�������ģ��ٶ���һ������Ϊ��ʼ���䣩
    if (!dungeon.rooms.empty()) {
        auto [startX, startY] = dungeon.rooms.front()->center();
        Player player(startX, startY);
        std::cout << "Player starting at (" << startX << ", " << startY << ")" << std::endl;

        // ʾ��������ƶ�
        player.move(1, 0);
        player.move(0, 1);
    }

    // ������ϣ��ȴ��û�������˳�
    std::cout << "Press any key to exit..." << std::endl;
    std::cin.get();
    return 0;
}