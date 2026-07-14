#pragma once
#include "Globals.h"
#include <array>
#include <vector>

// Định nghĩa các loại ô trên bản đồ
enum Tile { WALL = 0, PELLET, POWER, EMPTY, HOUSE, DOOR };

// Định nghĩa trạng thái của Ma (để biết ma có được đi xuyên cửa nhà hay không)
enum class GMode { InHouse, Exiting, Normal, Frightened, Eaten, Entering };

// Sử dụng using để đặt tên ngắn gọn cho mảng 2 chiều
using MazeGrid = std::array<std::array<int, GRID_W>, GRID_H>;

class Maze {
public:
    MazeGrid grid;
    int pelletTotal;
    int pelletsLeft;

    Maze(); // Hàm khởi tạo

    // Hàm xây dựng bản đồ tĩnh
    void build();

    // Kiểm tra xem Pac-Man có thể đi vào ô (x,y) không
    bool pacWalkable(int x, int y) const;

    // Kiểm tra xem Ma có thể đi vào ô (x,y) không (tùy thuộc trạng thái)
    bool ghostWalkable(int x, int y, GMode mode) const;

    // Thuật toán loang (Flood-fill) để đảm bảo mọi hạt thức ăn đều có thể ăn được
    bool validate() const;
};