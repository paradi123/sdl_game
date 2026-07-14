#include "Maze.h"
#include <cstdio>

Maze::Maze() {
    pelletTotal = 0;
}

void Maze::build() {
    // 1. Viền tường ngoài cùng, bên trong rải thức ăn theo kiểu ô bàn cờ (caro)
    //    thay vì phủ kín từng ô -> giảm bớt số lượng thức ăn cần ăn để thắng,
    //    nhưng vẫn phải ăn hết 100% số hạt đã rải ra
    for (int r = 0; r < GRID_H; r++) {
        for (int c = 0; c < GRID_W; c++) {
            bool isBorder = (r == 0 || r == GRID_H - 1 || c == 0 || c == GRID_W - 1);
            if (isBorder) grid[r][c] = WALL;
            else grid[r][c] = ((r + c) % 2 == 0) ? PELLET : EMPTY;
        }
    }

    // 2. Đường hầm xuyên bản đồ (trái <-> phải) ở hàng giữa
    grid[10][0] = EMPTY;
    grid[10][GRID_W - 1] = EMPTY;

    // 3. Nhà cho Ma (Ghost House) - ở chính giữa bản đồ
    for (int c = 8; c <= 12; c++) grid[9][c] = WALL;
    grid[9][9] = DOOR; grid[9][10] = DOOR; grid[9][11] = DOOR;
    for (int c = 8; c <= 12; c++) grid[13][c] = WALL;
    for (int r = 10; r <= 12; r++) { grid[r][8] = WALL; grid[r][12] = WALL; }
    for (int r = 10; r <= 12; r++)
        for (int c = 9; c <= 11; c++)
            grid[r][c] = HOUSE;

    // 4. Cột trụ (pillar) 1x1 - chặn tầm nhìn (LoS) mà không chặn đường đi,
    //    được rải rác thay vì nối thành tường dài như bản đồ cũ
    static const int pillars1x1[][2] = {
        {2, 10}, {18, 10}, {10, 2}, {10, 18},
        {7, 7},  {7, 13},  {13, 7}, {13, 13},
        {4, 10}, {16, 10},
        // Thêm một chút để bản đồ khó hơn: chặn lối tiếp cận cửa nhà Ma
        // từ hướng bắc/nam, và mỗi đấu trường mở bị "gặm" nhẹ 1 góc trong
        {5, 8},  {5, 12},  {15, 8}, {15, 12},
        {4, 4},  {4, 16},  {16, 4}, {16, 16},
    };
    for (auto& p : pillars1x1) grid[p[0]][p[1]] = WALL;

    // 5. Cột trụ 2x2 - vật cản lớn hơn, đặt cạnh hai bên nhà Ma
    static const int pillars2x2[][2] = { // toạ độ góc trên-trái
        {9, 3}, {9, 15}, {12, 3}, {12, 15},
    };
    for (auto& p : pillars2x2) {
        int r = p[0], c = p[1];
        grid[r][c] = WALL;     grid[r][c + 1] = WALL;
        grid[r + 1][c] = WALL; grid[r + 1][c + 1] = WALL;
    }

    // 6. Bốn "đấu trường" mở 5x5 ở bốn góc bản đồ (Open Arenas) - không có
    //    trụ nào chắn bên trong, chỉ có Power Pellet ở góc ngoài cùng để
    //    người chơi có không gian rộng đối đầu / né tránh Ma
    grid[1][1] = POWER;
    grid[1][GRID_W - 2] = POWER;
    grid[GRID_H - 2][1] = POWER;
    grid[GRID_H - 2][GRID_W - 2] = POWER;

    // 7. Xóa thức ăn ở vị trí xuất phát của Pac-Man
    grid[17][10] = EMPTY;

    // Đếm tổng số lượng thức ăn
    pelletTotal = 0;
    for (int r = 0; r < GRID_H; r++)
        for (int c = 0; c < GRID_W; c++)
            if (grid[r][c] == PELLET || grid[r][c] == POWER) pelletTotal++;
    pelletsLeft = pelletTotal;
}

bool Maze::pacWalkable(int x, int y) const {
    int t = grid[y][x];
    // Pacman không thể đi xuyên tường, cửa nhà ma, và khu vực trong nhà ma
    return t != WALL && t != DOOR && t != HOUSE;
}

bool Maze::ghostWalkable(int x, int y, GMode mode) const {
    int t = grid[y][x];
    if (t == WALL) return false;
    
    // Ma bình thường không được vào lại nhà, trừ khi bị ăn (Eaten) hoặc đang trong tiến trình đi ra/vào
    if (t == DOOR || t == HOUSE)
        return mode == GMode::Eaten || mode == GMode::Entering ||
               mode == GMode::Exiting || mode == GMode::InHouse;
               
    return true;
}

bool Maze::validate() const {
    std::array<std::array<bool, GRID_W>, GRID_H> seen{};
    std::vector<Point> stack{{10, 17}};
    seen[17][10] = true;
    int reachablePellets = 0;
    
    if (grid[17][10] == PELLET || grid[17][10] == POWER) reachablePellets++;
    
    while (!stack.empty()) {
        Point p = stack.back(); stack.pop_back();
        static const Dir dirs[4] = {Dir::Up, Dir::Down, Dir::Left, Dir::Right};
        for (Dir d : dirs) {
            Point n = neighborWrap(p.x, p.y, d);
            if (n.y < 0 || n.y >= GRID_H) continue;
            if (seen[n.y][n.x]) continue;
            if (!pacWalkable(n.x, n.y)) continue;
            
            seen[n.y][n.x] = true;
            if (grid[n.y][n.x] == PELLET || grid[n.y][n.x] == POWER) reachablePellets++;
            stack.push_back(n);
        }
    }
    printf("validate: %d/%d pellets reachable\n", reachablePellets, pelletTotal);
    return reachablePellets == pelletTotal;
}