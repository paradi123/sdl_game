#include "PacMan.h"
#include <cmath>


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

PacMan::PacMan() {
    reset();
}

void PacMan::reset() {
    // Tọa độ xuất phát đổi thành số thực (float) đặt ở giữa ô xuất phát (17,10)
    x = 10.5f;
    y = 17.5f;
    cx = 10;
    cy = 17;
    tx = 10;
    ty = 17;
    angle = M_PI; // Nhìn sang trái ban đầu
    score = 0;
    atePower = false;
    facing = Dir::Left;
}

void PacMan::updateFreeMove(float dt, Maze& maze, const Uint8* keyState) {
    // 1. XỬ LÝ QUAY GÓC NHÌN BẰNG CHUỘT (Giữ nguyên)
    int mouseX, mouseY;
    SDL_GetRelativeMouseState(&mouseX, &mouseY);
    
    float mouseSensitivity = 0.002f; 
    angle += mouseX * mouseSensitivity;
    
    if (angle < 0) angle += 2.0f * M_PI;
    if (angle > 2.0f * M_PI) angle -= 2.0f * M_PI;

    // 2. TÍNH TOÁN VÉC-TƠ DI CHUYỂN
    float moveSpeed = 4.0f; 
    float moveStep = moveSpeed * dt;

    float dirX = std::cos(angle);
    float dirY = std::sin(angle);
    float strafeX = -std::sin(angle);
    float strafeY = std::cos(angle);

    float moveX = 0.0f;
    float moveY = 0.0f;

    if (keyState[SDL_SCANCODE_W] || keyState[SDL_SCANCODE_UP]) {
        moveX += dirX; moveY += dirY;
    }
    if (keyState[SDL_SCANCODE_S] || keyState[SDL_SCANCODE_DOWN]) {
        moveX -= dirX; moveY -= dirY;
    }
    if (keyState[SDL_SCANCODE_A]) {
        moveX -= strafeX; moveY -= strafeY;
    }
    if (keyState[SDL_SCANCODE_D]) {
        moveX += strafeX; moveY += strafeY;
    }

    // Chuẩn hóa véc-tơ di chuyển để đi chéo không bị nhanh hơn đi thẳng
    float len = std::sqrt(moveX * moveX + moveY * moveY);
    if (len > 0.0f) {
        moveX = (moveX / len) * moveStep;
        moveY = (moveY / len) * moveStep;
    }

    // 3. THUẬT TOÁN TRƯỢT TƯỜNG MƯỢT MÀ (Tách biệt hoàn toàn 2 trục)
    float hitbox = 0.25f; // Khoảng cách an toàn với tường

    // Kiểm tra và di chuyển trục X trước
    float newX = x + moveX;
    bool collideX = false;
    
    // Kiểm tra va chạm ở cả phía trên và phía dưới của hitbox trục X
    if (maze.grid[(int)(y - hitbox)][(int)(newX + (moveX > 0 ? hitbox : -hitbox))] == WALL ||
        maze.grid[(int)(y + hitbox)][(int)(newX + (moveX > 0 ? hitbox : -hitbox))] == WALL) {
        collideX = true;
    }
    
    // Nếu trơn tru không vướng tường, cập nhật X
    if (!collideX) {
        x = newX;
    }

    // Kiểm tra và di chuyển trục Y sau
    float newY = y + moveY;
    bool collideY = false;
    
    // Kiểm tra va chạm ở cả phía trái và phía phải của hitbox trục Y
    if (maze.grid[(int)(newY + (moveY > 0 ? hitbox : -hitbox))][(int)(x - hitbox)] == WALL ||
        maze.grid[(int)(newY + (moveY > 0 ? hitbox : -hitbox))][(int)(x + hitbox)] == WALL) {
        collideY = true;
    }
    
    // Nếu trơn tru không vướng tường, cập nhật Y
    if (!collideY) {
        y = newY;
    }

    // 4. ĐƯỜNG HẦM VÀ ĐỒNG BỘ LƯỚI (Giữ nguyên)
    if (x < 0.0f) x += GRID_W;
    if (x >= GRID_W) x -= GRID_W;

    cx = (int)x; 
    cy = (int)y; 

    // 5. LOGIC ĂN HẠT THỨ CĂN
    int cell = maze.grid[cy][cx];
    if (cell == PELLET || cell == POWER) {
        maze.grid[cy][cx] = EMPTY;
        maze.pelletsLeft--;
        if (cell == POWER) {
            score += 50;
            atePower = true;
        }
        else score += 10;
    }
}