#include "C:/Users/12022/source/repos/CSC8508/CSC8508CoreClasses/CMakeFiles/CSC8508CoreClasses.dir/Debug/cmake_pch.hxx"
#include "Room.h"
#include "dungeon.h"

// �����������е��³�����ʾ��
// ----------------------------
int maindun() {
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