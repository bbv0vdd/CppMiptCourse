/*
g++ -std=c++23 -Wall -Wextra -Wpedantic -o 10.03 10.03.cpp -lboost_system
./10.03
*/

#include <iostream>
#include <boost/multi_array.hpp>
#include <unistd.h>

class GameOfLife {
private:
    static const int SIZE = 10;
    boost::multi_array<bool, 2> current;
    boost::multi_array<bool, 2> next;
    
    int countLiveNeighbors(int x, int y) const {
        int count = 0;
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                if (dx == 0 && dy == 0) continue;
                int nx = x + dx;
                int ny = y + dy;
                if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE) {
                    if (current[nx][ny]) count++;
                }
            }
        }
        return count;
    }
    
public:
    GameOfLife() : current(boost::extents[SIZE][SIZE]), 
                   next(boost::extents[SIZE][SIZE]) {
        for (int i = 0; i < SIZE; ++i) {
            for (int j = 0; j < SIZE; ++j) {
                current[i][j] = false;
                next[i][j] = false;
            }
        }
    }
    
    void setGliderPattern() {
        current[2][1] = true;
        current[3][2] = true;
        current[1][3] = true;
        current[2][3] = true;
        current[3][3] = true;
    }
    
    void update() {
        for (int i = 0; i < SIZE; ++i) {
            for (int j = 0; j < SIZE; ++j) {
                int neighbors = countLiveNeighbors(i, j);
                if (current[i][j]) {
                    next[i][j] = (neighbors == 2 || neighbors == 3);
                } else {
                    next[i][j] = (neighbors == 3);
                }
            }
        }
        current = next;
    }
    
    void print() const {
        for (int i = 0; i < SIZE; ++i) 
        {
            for (int j = 0; j < SIZE; ++j) 
            {
                std::cout << (current[i][j] ? "█ " : "· ");
            }
            std::cout << "\n";
        }
        std::cout << "___________________\n\n";
    }
    
    bool hasLiveCells() const {
        for (int i = 0; i < SIZE; ++i) {
            for (int j = 0; j < SIZE; ++j) {
                if (current[i][j]) return true;
            }
        }
        return false;
    }
};

int main() {
    GameOfLife game;
    game.setGliderPattern();
    
    game.print();
    
    int generation = 0;
    while (generation < 50 && game.hasLiveCells()) {
        usleep(500000);
        game.update();
        std::cout << "\033[2J\033[1;1H";
        game.print();
        generation++;
    }
    
    return 0;
}