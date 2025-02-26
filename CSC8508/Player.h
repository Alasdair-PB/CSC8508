#pragma once
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
class Player
{
public:
    int posX, posY;

    Player(int x, int y) : posX(x), posY(y) {}

    void move(int dx, int dy) {
        posX += dx;
        posY += dy;
        std::cout << "Player moved to (" << posX << ", " << posY << ")" << std::endl;
    }
};

