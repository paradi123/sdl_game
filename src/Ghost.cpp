#include "Ghost.h"
#include <cmath>
#include <algorithm>

Ghost::Ghost(std::string n, SDL_Color c, int hCx, int hCy, int sCx, int sCy)
    : name(n), color(c), homeCx(hCx), homeCy(hCy), scatterCx(sCx), scatterCy(sCy) {
    // Mỗi con ma một hướng lệch hình + tốc độ hơi khác nhau -> đỡ chồng khít lên nhau
    // và đỡ cảm giác cả bầy di chuyển máy móc y hệt nhau
    if (name == "Blinky")      { visualOffsetX = -0.16f; visualOffsetY = -0.16f; speedJitter = 1.08f; }
    else if (name == "Pinky")  { visualOffsetX =  0.16f; visualOffsetY = -0.16f; speedJitter = 0.94f; }
    else if (name == "Inky")   { visualOffsetX = -0.16f; visualOffsetY =  0.16f; speedJitter = 1.12f; }
    else if (name == "Clyde")  { visualOffsetX =  0.16f; visualOffsetY =  0.16f; speedJitter = 0.88f; }
    else                       { visualOffsetX =  0.0f;  visualOffsetY =  0.0f;  speedJitter = 1.0f;  }
    reset(0.0f, 0);
}

void Ghost::reset(float delay, int index) {
    cx = homeCx; cy = homeCy;
    tx = homeCx; ty = homeCy;
    moveT = 0.0f;
    dir = Dir::Up;
    mode = GMode::InHouse;
    releaseDelay = delay;
    bobTime = (float)index;
}

float Ghost::getSpeed(int level) const {
    if (mode == GMode::Frightened) return 2.1f * speedJitter;
    if (mode == GMode::Eaten || mode == GMode::Entering) return 10.0f;
    if (mode == GMode::Exiting) return 2.0f;
    return (1.8f + std::min(level - 1, 6) * 0.15f) * speedJitter;
}

// Kiểm tra xem có con ma nào KHÁC đang đứng tại (x,y) hoặc đang nhắm tới (x,y) hay không.
// Chỉ tính các con ma đang "hoạt động" (Normal/Frightened/Exiting) - ma đã bị ăn hay đang
// chờ trong nhà thì không tính, để không khoá đường hồi nhà / ra khỏi nhà của các ma khác.
bool Ghost::cellClaimedByOther(const std::vector<Ghost>& allGhosts, int x, int y) const {
    for (const Ghost& g : allGhosts) {
        if (&g == this) continue;
        if (g.mode != GMode::Normal && g.mode != GMode::Frightened && g.mode != GMode::Exiting) continue;
        if ((g.cx == x && g.cy == y) || (g.tx == x && g.ty == y)) return true;
    }
    return false;
}

void Ghost::getTarget(const PacMan& pac, bool scatterMode, int& targetX, int& targetY) {
    if (scatterMode) {
        targetX = scatterCx; 
        targetY = scatterCy;
        return;
    }
    if (name == "Blinky") {
        // 1. Kẻ săn mồi (Đỏ): Nhắm trực tiếp vào ô Pac-Man đang đứng
        targetX = pac.cx;
        targetY = pac.cy;
    } 
    else if (name == "Pinky") {
        // 2. Kẻ phục kích (Hồng): Luôn nhắm vào ô phía trước mặt Pac-Man 4 bước để đón lõng
        Dir face = (pac.dir != Dir::None) ? pac.dir : pac.facing;
        Point dv = dirVec(face);
        targetX = pac.cx + dv.x * 4;
        targetY = pac.cy + dv.y * 4;
    } 
    else if (name == "Inky") {
        // 3. Kẻ bọc lót (Xanh lam): Nhắm vào 2 ô ở phía sau lưng Pac-Man để chặn đường lui
        Dir face = (pac.dir != Dir::None) ? pac.dir : pac.facing;
        Point dv = dirVec(face);
        targetX = pac.cx - dv.x * 2;
        targetY = pac.cy - dv.y * 2;
    } 
    else if (name == "Clyde") {
        // 4. Kẻ nhút nhát (Cam): Đuổi theo nếu ở xa, nhưng nếu lại gần hơn 8 ô thì hoảng sợ quay về góc nhà
        int dist = std::abs(cx - pac.cx) + std::abs(cy - pac.cy); // Khoảng cách Manhattan
        if (dist > 8) {
            targetX = pac.cx;
            targetY = pac.cy;
        } else {
            targetX = scatterCx;
            targetY = scatterCy;
        }
    } 
    else {
                 // Dự phòng
        targetX = pac.cx;
        targetY = pac.cy;
    }
}

Dir Ghost::chooseBestDir(Maze& maze, int targetX, int targetY, bool flee, const std::vector<Ghost>& allGhosts) {
    static const Dir order[4] = {Dir::Up, Dir::Left, Dir::Down, Dir::Right};
    Dir best = Dir::None;
    long bestScore = 0;
    bool have = false;

    for (Dir d : order) {
        if (dir != Dir::None && d == opposite(dir)) continue;

        Point n = neighborWrap(cx, cy, d);
        if (n.y < 0 || n.y >= GRID_H) continue;
        if (!maze.ghostWalkable(n.x, n.y, mode)) continue;

        long dx = n.x - targetX;
        long dy = n.y - targetY;
        long score = dx * dx + dy * dy;

        // Ô đã bị ma khác chiếm/nhắm tới -> giảm sức hấp dẫn để tránh chồng lên nhau
        if (cellClaimedByOther(allGhosts, n.x, n.y)) {
            score += flee ? -400 : 400;
        }

        if (!have || (flee ? score > bestScore : score < bestScore)) {
            have = true;
            bestScore = score;
            best = d;
        }
    }

    if (!have) {
        for (Dir d : order) {
            Point n = neighborWrap(cx, cy, d);
            if (n.y >= 0 && n.y < GRID_H && maze.ghostWalkable(n.x, n.y, mode)) { 
                best = d; break; 
            }
        }
    }
    return best;
}

Dir Ghost::findPathAStar(Maze& maze, int targetX, int targetY, const std::vector<Ghost>& allGhosts) {
    std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> openList;
    
    int gCost[GRID_H][GRID_W];
    Dir cameFrom[GRID_H][GRID_W];
    bool closedList[GRID_H][GRID_W];

    for(int i = 0; i < GRID_H; i++) {
        for(int j = 0; j < GRID_W; j++) {
            gCost[i][j] = 999999;       
            cameFrom[i][j] = Dir::None; 
            closedList[i][j] = false;
        }
    }

    gCost[cy][cx] = 0;
    openList.push({cx, cy, 0, 0});

    const Dir dirs[4] = {Dir::Up, Dir::Left, Dir::Down, Dir::Right};
    int finalX = cx, finalY = cy;
    bool found = false;

    while(!openList.empty()) {
        AStarNode current = openList.top();
        openList.pop();

        int ux = current.x, uy = current.y;

        if (ux == targetX && uy == targetY) {
            finalX = ux; finalY = uy;
            found = true; break;
        }

        if (closedList[uy][ux]) continue;
        closedList[uy][ux] = true;

        for (Dir d : dirs) {
            if (ux == cx && uy == cy && dir != Dir::None && d == opposite(dir)) continue; 

            Point n = neighborWrap(ux, uy, d);
            if (n.y < 0 || n.y >= GRID_H) continue;
            if (!maze.ghostWalkable(n.x, n.y, mode)) continue;
            if (closedList[n.y][n.x]) continue;

            // Cộng thêm chi phí nếu ô này đang bị ma khác chiếm/nhắm tới, để A* né
            // bớt các ô đông đúc thay vì buộc phải đi xuyên qua đội hình ma khác
            int occupancyPenalty = cellClaimedByOther(allGhosts, n.x, n.y) ? 4 : 0;
            int tentativeG = gCost[uy][ux] + 1 + occupancyPenalty;

            if (tentativeG < gCost[n.y][n.x]) {
                gCost[n.y][n.x] = tentativeG;
                cameFrom[n.y][n.x] = d;

                int h_dx = std::min(std::abs(n.x - targetX), GRID_W - std::abs(n.x - targetX));
                int h = h_dx + std::abs(n.y - targetY);
                int f = tentativeG + h;

                openList.push({n.x, n.y, f, tentativeG});
            }
        }
    }

    if (!found) return Dir::None;

    int currX = finalX, currY = finalY;
    Dir firstStep = Dir::None;

    while (currX != cx || currY != cy) {
        Dir d = cameFrom[currY][currX];
        firstStep = d;
        Point p = neighborWrap(currX, currY, opposite(d)); 
        currX = p.x; currY = p.y;
    }

    return firstStep;
}

void Ghost::arrive(Maze& maze, const PacMan& pac, bool scatterMode, const std::vector<Ghost>& allGhosts) {
    if (mode == GMode::InHouse) return;

    if (mode == GMode::Exiting) {
        if (cy == 8) {
            mode = GMode::Normal;
        } else {
            Dir d = (cx != 10) ? (cx < 10 ? Dir::Right : Dir::Left) : Dir::Up;
            Point n = neighborWrap(cx, cy, d);
            dir = d; tx = n.x; ty = n.y;
            return;
        }
    }

    int targetX, targetY;
    bool flee = false;

    if (mode == GMode::Normal) {
        getTarget(pac, scatterMode, targetX, targetY);
    } 
    else if (mode == GMode::Frightened) {
        targetX = pac.cx; 
        targetY = pac.cy;
        flee = true;
    }
    else if (mode == GMode::Eaten) {
        targetX = homeCx; targetY = homeCy;
        if (std::abs(cx - homeCx) <= 1 && std::abs(cy - homeCy) <= 1) {
            mode = GMode::InHouse;
            releaseDelay = 0.0f; 
            return;
        }
    } 
    else return; 

    Dir d = Dir::None;
    if (flee) {
        d = chooseBestDir(maze, targetX, targetY, flee, allGhosts);
    } else {
        d = findPathAStar(maze, targetX, targetY, allGhosts);
        if (d == Dir::None) d = chooseBestDir(maze, targetX, targetY, flee, allGhosts);
    }

    if (d == Dir::None) return;
    Point n = neighborWrap(cx, cy, d);
    dir = d; tx = n.x; ty = n.y;
}

void Ghost::update(float dt, Maze& maze, const PacMan& pac, bool scatterMode, const std::vector<Ghost>& allGhosts) {
    // Luôn chạy bobTime (không chỉ khi ở trong nhà) để Renderer có thể dùng nó
    // tạo hiệu ứng đung đưa nhẹ khi ma di chuyển bình thường, đỡ cảm giác máy móc.
    bobTime += dt;

    if (mode == GMode::InHouse) {
        releaseDelay -= dt;
        if (releaseDelay <= 0.0f) {
            mode = GMode::Exiting;
            dir = Dir::Up;
        }
        return;
    }

    if (moveT == 0.0f) arrive(maze, pac, scatterMode, allGhosts);

    if (dir != Dir::None && (tx != cx || ty != cy)) {
        moveT += getSpeed(1) * dt;
        if (moveT >= 1.0f) {
            cx = tx; cy = ty;
            moveT = 0.0f;
        }
    }
}