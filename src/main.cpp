#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "Globals.h"
#include "Maze.h"
#include "Renderer.h"
#include "PacMan.h"
#include "Audio.h"
#include "Ghost.h"
#include <vector>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- KIỂM TRA VA CHẠM (Cập nhật tọa độ x, y thực của Pac-Man) ---
bool checkCollision(const PacMan& pac, const Ghost& ghost) {
    if (ghost.mode == GMode::InHouse) return false;

    // Lấy trực tiếp tọa độ FPS của Pac-Man
    float px = pac.x;
    float py = pac.y;
    
    // Tính toán tọa độ nội suy của Bầy Ma
    float gx = ghost.cx + (ghost.tx - ghost.cx) * ghost.moveT + 0.5f;
    float gy = ghost.cy + (ghost.ty - ghost.cy) * ghost.moveT + 0.5f;
    
    float dx = px - gx;
    float dy = py - gy;
    
    // Hitbox thu nhỏ lại còn 0.5 để tạo cảm giác rón rén lách qua ma
    return (dx * dx + dy * dy) < 0.5f; 
}

int main(int argc, char* argv[]) {
    SDL_SetMainReady();

    Renderer renderer;
    if (!renderer.init("Pac-Man 3D Survival Horror", SCREEN_W, SCREEN_H)) return 1;

    Audio audio;
    audio.init();
    audio.playBegin();

    Maze maze;
    maze.build();

    PacMan pac;
    std::vector<Ghost> ghosts;
    ghosts.push_back(Ghost("Blinky", {230, 30, 30, 255},  10, 11, GRID_W - 2, 1));
    ghosts.push_back(Ghost("Pinky",  {255, 140, 200, 255}, 10, 10, 1, 1));
    ghosts.push_back(Ghost("Inky",   {60, 220, 220, 255},  9, 11, GRID_W - 2, GRID_H - 2));
    ghosts.push_back(Ghost("Clyde",  {255, 170, 40, 255},  11, 11, 1, GRID_H - 2));

    auto resetEntities = [&]() {
        pac.reset();
        ghosts[0].reset(0.0f, 0); 
        ghosts[1].reset(2.0f, 1); 
        ghosts[2].reset(5.0f, 2); 
        ghosts[3].reset(8.0f, 3); 
    };
    resetEntities();
    
    // --- KÍCH HOẠT CHẾ ĐỘ CHUỘT FPS ---
    SDL_SetRelativeMouseMode(SDL_TRUE);

    int lives = 3;
    bool gameOver = false;
    bool win = false;
    bool running = true;
    float frightenTimer = 0.0f; 
    SDL_Event e;
    Uint32 lastTicks = SDL_GetTicks();
    
    bool scatterMode = true;
    float modeTimer = 0.0f;

    while (running) {
        // 1. CHỈ BẮT SỰ KIỆN PHÍM BẤM 1 LẦN (Thoát & Khởi động lại)
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE: running = false; break;
                    case SDLK_RETURN:
                        if (gameOver || win) {
                            maze.build();
                            lives = 3;
                            gameOver = false;
                            win = false;
                            resetEntities();
                            audio.stopGhostLoop();
                            audio.playBegin();
                        }
                        break;
                }
            }
        }

        // Lấy trạng thái giữ phím liên tục cho di chuyển tự do
        const Uint8* keyState = SDL_GetKeyboardState(NULL);

        Uint32 now = SDL_GetTicks();
        float dt = (now - lastTicks) / 1000.0f;
        lastTicks = now;
        if (dt > 0.1f) dt = 0.1f; 
        if (dt <= 0.0f) dt = 0.001f;

        if (!gameOver && !win) {
            // Logic thời gian của bầy ma
            if (frightenTimer <= 0.0f) {
                modeTimer += dt;
                if (scatterMode && modeTimer > 7.0f) {
                    scatterMode = false; 
                    modeTimer = 0.0f;
                    for (auto& ghost : ghosts) ghost.dir = opposite(ghost.dir);
                } 
                else if (!scatterMode && modeTimer > 20.0f) {
                    scatterMode = true;  
                    modeTimer = 0.0f;
                    for (auto& ghost : ghosts) ghost.dir = opposite(ghost.dir);
                }
            }

            int oldScore = pac.score;
            
            // 2. CẬP NHẬT DI CHUYỂN FPS CỦA PAC-MAN
            pac.updateFreeMove(dt, maze, keyState);

            // Đồng bộ góc quay của Pac-Man (Radian) thành hướng (Dir) cho hàm vẽ 3D
            if (pac.angle >= 7.0f * M_PI / 4.0f || pac.angle < M_PI / 4.0f) pac.facing = Dir::Right;
            else if (pac.angle >= M_PI / 4.0f && pac.angle < 3.0f * M_PI / 4.0f) pac.facing = Dir::Down;
            else if (pac.angle >= 3.0f * M_PI / 4.0f && pac.angle < 5.0f * M_PI / 4.0f) pac.facing = Dir::Left;
            else pac.facing = Dir::Up;

            if (pac.score > oldScore) {
                audio.playChomp();
            }

            // Ăn hết toàn bộ thức ăn trên bản đồ -> Thắng!
            if (maze.pelletsLeft <= 0) {
                win = true;
                audio.stopGhostLoop();
            }

            if (pac.atePower) {
                pac.atePower = false;
                frightenTimer = 6.0f; 
                for (auto& ghost : ghosts) {
                    if (ghost.mode == GMode::Normal || ghost.mode == GMode::Exiting) {
                        ghost.mode = GMode::Frightened;
                        ghost.dir = opposite(ghost.dir); 
                    }
                }
            }

            if (frightenTimer > 0.0f) {
                frightenTimer -= dt;
                if (frightenTimer <= 0.0f) {
                    for (auto& ghost : ghosts) {
                        if (ghost.mode == GMode::Frightened) ghost.mode = GMode::Normal;
                    }
                }
            }
// bất động ma
           for (auto& ghost : ghosts) {
            ghost.update(dt, maze, pac, scatterMode, ghosts);
            }

            // --- ÂM THANH CẢNH BÁO MA: một bản nhạc DUY NHẤT dùng chung cho cả bầy,
            //     phát lặp liên tục khi có Ma ở gần, không phát lại mỗi khi có Ma khác đến ---
            {
                const float DETECT_RADIUS = 8.0f; // Bán kính nghe thấy (ô), xa hơn tầm nhìn có sương mù (6 ô)
                float nearestDist = -1.0f;

                for (const auto& ghost : ghosts) {
                    // Chỉ cảnh báo Ma đang thực sự săn đuổi; Ma bị hoảng sợ/đã ăn/trong nhà thì không đe doạ
                    if (ghost.mode != GMode::Normal) continue;
                    float gx = ghost.cx + (ghost.tx - ghost.cx) * ghost.moveT + 0.5f;
                    float gy = ghost.cy + (ghost.ty - ghost.cy) * ghost.moveT + 0.5f;
                    float dx = gx - pac.x, dy = gy - pac.y;
                    float d = std::sqrt(dx * dx + dy * dy);
                    if (nearestDist < 0.0f || d < nearestDist) nearestDist = d;
                }

                if (nearestDist >= 0.0f && nearestDist < DETECT_RADIUS) {
                    float closeness = 1.0f - (nearestDist / DETECT_RADIUS); // 0 = xa, 1 = sát bên
                    if (!audio.isGhostLoopPlaying()) audio.startGhostLoop(closeness);
                    else audio.updateGhostLoopVolume(closeness);
                } else if (audio.isGhostLoopPlaying()) {
                    audio.stopGhostLoop();
                }
            }

            // Va chạm
            for (auto& ghost : ghosts) {
                if (checkCollision(pac, ghost)) {
                    if (ghost.mode == GMode::Frightened) {
                        ghost.mode = GMode::Eaten;
                        pac.score += 200; 
                        audio.playEatGhost();
                    } 
                    else if (ghost.mode == GMode::Normal || ghost.mode == GMode::Exiting) {
                        lives--;
                        audio.stopGhostLoop();
                        audio.playDeath();
                        if (lives <= 0) gameOver = true;
                        else resetEntities();
                        break;
                    }
                }
            }
        }

        // --- KẾT XUẤT ĐỒ HỌA 3D VÀ UI ---
        renderer.clear();
        renderer.draw3D(pac, maze);
        renderer.drawPellets3D(pac, maze);
        renderer.drawGhosts3D(pac, ghosts, frightenTimer); 
        
        if (!gameOver && !win) renderer.drawMiniMap(pac, ghosts, maze);
        renderer.drawHUD(pac.score, lives, gameOver, win);
        
        renderer.present();
    }

    return 0;
}