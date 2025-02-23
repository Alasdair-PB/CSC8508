
#include <vector>
#include <cstdlib>
#include <ctime>

class Dungeon {
public:
    std::vector<Room*> rooms;
    int gridWidth, gridHeight;
    // ���ö�ά����� vector<vector<int>> �洢������Ϣ����ʾǽ�塢���ȵ�

    Dungeon(int width, int height) : gridWidth(width), gridHeight(height) {
        std::srand(static_cast<unsigned>(std::time(nullptr))); // ��ʼ���������
    }

    ~Dungeon() {
        for (auto room : rooms)
            delete room;
    }

    // ���������ɷ��䡢���ӷ��䡢�����ͨ�ԡ���Ⱦ��ͼ��
    void generateRooms(int roomCount, int minSize, int maxSize);
    void connectRooms();
    bool checkOverlap(Room* a, Room* b);
    // ����������������
};
