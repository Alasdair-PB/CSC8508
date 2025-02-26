
#include <vector>
#include <cstdlib>
#include <ctime>
#include <Room.h>


// ���³��ඨ��
class Dungeon {
public:
    std::vector<Room*> rooms;  // �洢�������ɵķ���
    int gridWidth, gridHeight; // ���³�����ߴ�

    Dungeon(int width, int height)
        : gridWidth(width), gridHeight(height) {
        std::srand(static_cast<unsigned>(std::time(nullptr))); // ��ʼ���������
    }

    ~Dungeon() {
        for (auto room : rooms)
            delete room;
    }

    // ���ɷ��䣬������������������С�ߴ硢���ߴ�
    void generateRooms(int roomCount, int minSize, int maxSize) {
        int attempts = 0; // ���Դ���������������ѭ��
        while (rooms.size() < static_cast<size_t>(roomCount) && attempts < roomCount * 5) {
            int w = minSize + std::rand() % (maxSize - minSize + 1);
            int h = minSize + std::rand() % (maxSize - minSize + 1);
            int x = std::rand() % (gridWidth - w);
            int y = std::rand() % (gridHeight - h);

            Room* newRoom = new Room(x, y, w, h);

            // ����ǵ�һ�����䣬������Ϊ��ʼ����
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
                delete newRoom; // �ص���ɾ������
            }
            attempts++;
        }
    }

    // ������������Ƿ��ص�
    bool checkOverlap(Room* a, Room* b) {
        // ��� a �� b �Ƿ����ص�������һ���߾�Ҳ���ԣ�
        return !(a->x + a->width <= b->x || b->x + b->width <= a->x ||
            a->y + a->height <= b->y || b->y + b->height <= a->y);
    }

    // �����ӷ��䣺���շ������ĵ�˳����������
    void connectRooms() {
        if (rooms.empty()) return;

        // �Է��䰴�����ĵ� x ��������Ҳ���Ը��ݾ������ MST ��С��������
        std::sort(rooms.begin(), rooms.end(), [](Room* a, Room* b) {
            return a->center().first < b->center().first;
            });

        // ��˳������ÿ����������һ������
        for (size_t i = 1; i < rooms.size(); ++i) {
            Room* roomA = rooms[i - 1];
            Room* roomB = rooms[i];

            // �����ǻ�������ڽ��б�
            roomA->neighbors.push_back(roomB);
            roomB->neighbors.push_back(roomA);

            // ������Ե����������ȵĺ�����������������������·����¼����
            createCorridor(roomA, roomB);
        }
    }

    // �������ȣ�����ֻ��ʾ�⣬ʵ�ʿ��Ը�������ֱ�߻� L �����ȣ�
    void createCorridor(Room* a, Room* b) {
        auto [ax, ay] = a->center();
        auto [bx, by] = b->center();
        std::cout << "Corridor between (" << ax << ", " << ay << ") and ("
            << bx << ", " << by << ")" << std::endl;
        // ʵ��ʵ�֣����Խ�ֱ��·���ϵĸ��ӱ��Ϊ����
    }

    // ��ӡ�������³���Ϣ��������Ϣ�����������
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
