#include "C:/Users/12022/source/repos/CSC8508/CSC8508CoreClasses/CMakeFiles/CSC8508CoreClasses.dir/Debug/cmake_pch.hxx"
#include "Room.h"

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
