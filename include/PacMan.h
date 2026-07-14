#pragma once
#include "Globals.h"
#include "Maze.h"
#include <SDL.h> // <--- Bắt buộc phải có để nhận diện kiểu dữ liệu Uint8

class PacMan {
public:
    // --- CÁC BIẾN TỌA ĐỘ VẬT LÝ FPS MỚI ---
    float x, y;      // Vị trí thực trên bản đồ
    float angle;     // Góc nhìn Camera (tính theo Radian)
    
    // Giữ lại để đồng bộ với AI của hệ thống ma và tính điểm
    int cx, cy;      
    int tx, ty;      
    float moveT;
    Dir dir;
    Dir wishDir;
    Dir facing;
    float animTime;
    int score;
    bool atePower;

    PacMan();
    void reset();
    
    // Khai báo chuẩn hàm cập nhật di chuyển tự do bằng chuột/bàn phím
    void updateFreeMove(float dt, Maze& maze, const Uint8* keyState);
};