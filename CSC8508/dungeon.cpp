#include "dungeon.h"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

#include <algorithm>

using namespace NCL::CSC8508::DungeonGen;

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