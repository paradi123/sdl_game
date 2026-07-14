#pragma once
#include "Globals.h"
#include "Maze.h"
#include "PacMan.h"
#include <SDL.h>
#include <string>
#include <queue>
#include <vector>

// Cấu trúc Node dùng riêng cho thuật toán A*
struct AStarNode {
    int x, y;
    int f, g; // f = g + h (tổng chi phí)
    
    // Nạp chồng toán tử để ưu tiên Node có chi phí F nhỏ nhất trong priority_queue
    bool operator>(const AStarNode& other) const {
        return f > other.f;
    }
};

class Ghost {
public:
    std::string name;
    SDL_Color color;
    
    int homeCx, homeCy;
    int scatterCx, scatterCy;
    int cx, cy, tx, ty;

    float moveT;
    Dir dir;
    GMode mode;
    float releaseDelay;
    float bobTime;

    // Lệch hình nhỏ + hệ số tốc độ riêng cho từng con ma, để chúng không bao giờ
    // vẽ đè khít lên nhau 100% và không di chuyển đồng bộ như một khối máy móc
    float visualOffsetX, visualOffsetY;
    float speedJitter;

    Ghost(std::string n, SDL_Color c, int hCx, int hCy, int sCx, int sCy);

    void reset(float delay, int index);
    void update(float dt, Maze& maze, const PacMan& pac, bool scatterMode, const std::vector<Ghost>& allGhosts);

private:
    float getSpeed(int level) const;
    void arrive(Maze& maze, const PacMan& pac, bool scatterMode, const std::vector<Ghost>& allGhosts);

    void getTarget(const PacMan& pac, bool scatterMode, int& targetX, int& targetY);
    Dir chooseBestDir(Maze& maze, int targetX, int targetY, bool flee, const std::vector<Ghost>& allGhosts);
    Dir findPathAStar(Maze& maze, int targetX, int targetY, const std::vector<Ghost>& allGhosts);
    bool cellClaimedByOther(const std::vector<Ghost>& allGhosts, int x, int y) const;
};