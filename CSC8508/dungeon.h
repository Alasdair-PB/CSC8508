
#include <vector>
#include <cstdlib>
#include <ctime>

class Dungeon {
public:
    std::vector<Room*> rooms;  
    int gridWidth, gridHeight;

    Dungeon(int width, int height)
        : gridWidth(width), gridHeight(height) {
        std::srand(static_cast<unsigned>(std::time(nullptr))); /*initialize random seed*//*��ʼ���������*/
    }

    ~Dungeon() {
        for (auto room : rooms)
            delete room;
    }

    // ���ɷ��䣬������������������С�ߴ硢���ߴ�
    /* Generate rooms, parameters: room count, minimum size, maximum size */
    void generateRooms(int roomCount, int minSize, int maxSize) {
        int attempts = 0; // ���Դ���������������ѭ��
        /* Try to generate rooms until the desired count is reached or too many attempts are made */
        while (rooms.size() < static_cast<size_t>(roomCount) && attempts < roomCount * 5) {
            int w = minSize + std::rand() % (maxSize - minSize + 1);
            int h = minSize + std::rand() % (maxSize - minSize + 1);
            int x = std::rand() % (gridWidth - w);
            int y = std::rand() % (gridHeight - h);

            Room* newRoom = new Room(x, y, w, h);

            // ����ǵ�һ�����䣬������Ϊ��ʼ����
            /* If it is the first room, it can be set as the starting room */
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
                /* If there is an overlap, delete the room */
            }
            attempts++;
        }
    }

    // ������������Ƿ��ص�
    /* Check if two rooms overlap */
    bool checkOverlap(Room* a, Room* b) {
        // ��� a �� b �Ƿ����ص�������һ���߾�Ҳ���ԣ�
        /* Check if a and b have overlap (with a small margin) */
        return !(a->x + a->width <= b->x || b->x + b->width <= a->x ||
            a->y + a->height <= b->y || b->y + b->height <= a->y);
    }

    // �����ӷ��䣺���շ������ĵ�˳����������
    /* Simple connection of rooms: connect rooms in order of center point */
    void connectRooms() {
        if (rooms.empty()) return;

        // �Է��䰴�����ĵ� x ��������Ҳ���Ը��ݾ������ MST ��С��������
        /* Sort rooms by center point x coordinate (or distance for MST) */
        std::sort(rooms.begin(), rooms.end(), [](Room* a, Room* b) {
            return a->center().first < b->center().first;
            });

        // ��˳������ÿ����������һ������
        /* Connect each room to the next room in order */
        for (size_t i = 1; i < rooms.size(); ++i) {
            Room* roomA = rooms[i - 1];
            Room* roomB = rooms[i];

            // �����ǻ�������ڽ��б�
            /* Add them to each other's neighbor list */
            roomA->neighbors.push_back(roomB);
            roomB->neighbors.push_back(roomA);

            // ������Ե����������ȵĺ�����������������������·����¼����
            /* Here, you can call a function to generate a corridor between the two rooms, and record the path in the corridor map */
            createCorridor(roomA, roomB);
        }
    }

   
    void createCorridor(Room* a, Room* b) {
        auto [ax, ay] = a->center();
        auto [bx, by] = b->center();
        std::cout << "Corridor between (" << ax << ", " << ay << ") and ("
            << bx << ", " << by << ")" << std::endl;
        
    }

   
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
