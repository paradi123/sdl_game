#include "Renderer.h"
#include "PacMan.h" 
#include "Ghost.h"  
#include <cmath>
#include <vector>
#include <iostream>
#include <algorithm>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Renderer::Renderer() : window(nullptr), sdlRenderer(nullptr) {}

Renderer::~Renderer() {
    close();
}

bool Renderer::init(const char* title, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return false;
    }
    if (TTF_Init() != 0) {
        std::cerr << "TTF_Init failed: " << TTF_GetError() << "\n";
        return false;
    }
    
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window failed: " << SDL_GetError() << "\n";
        return false;
    }

    sdlRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!sdlRenderer) {
        std::cerr << "Renderer failed: " << SDL_GetError() << "\n";
        return false;
    }
    font = TTF_OpenFont("assets/Southern.ttf", 24);
    if (!font) {
        std::cerr << "Khong the tai font assets/Southern.ttf! Ban da de dung thu muc chua?\n";
        // Tạm thời không return false để game vẫn chạy dù thiếu font
    }

    ZBuffer.resize(SCREEN_W, 0.0f);

    return true;
}

void Renderer::close() {
    if (font) { TTF_CloseFont(font); font = nullptr; }
    TTF_Quit(); // <--- TẮT TTF
    if (sdlRenderer) { SDL_DestroyRenderer(sdlRenderer); sdlRenderer = nullptr; }
    if (window) { SDL_DestroyWindow(window); window = nullptr; }
    SDL_Quit();
}

void Renderer::clear() {
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255); // Màu nền đen
    SDL_RenderClear(sdlRenderer);
}

void Renderer::present() {
    SDL_RenderPresent(sdlRenderer);
}

// ------ CÁC HÀM VẼ CHI TIẾT ------

void Renderer::fillArc(float cx, float cy, float radius, SDL_Color col, float startDeg, float endDeg, int segments) {
    if (segments < 2) segments = 2;
    std::vector<SDL_Vertex> verts;
    std::vector<int> idx;
    verts.reserve(segments + 2);
    
    SDL_Vertex center;
    center.position = {cx, cy}; center.color = col; center.tex_coord = {0, 0};
    verts.push_back(center);
    
    float startRad = startDeg * PI_F / 180.0f;
    float endRad = endDeg * PI_F / 180.0f;
    
    for (int i = 0; i <= segments; i++) {
        float t = startRad + (endRad - startRad) * ((float)i / segments);
        SDL_Vertex v;
        v.position = {cx + radius * std::cos(t), cy + radius * std::sin(t)};
        v.color = col; v.tex_coord = {0, 0};
        verts.push_back(v);
    }
    
    for (int i = 1; i <= segments; i++) { 
        idx.push_back(0); idx.push_back(i); idx.push_back(i + 1); 
    }
    
    SDL_RenderGeometry(sdlRenderer, nullptr, verts.data(), (int)verts.size(), idx.data(), (int)idx.size());
}

void Renderer::drawWallTile(int x, int y) {
    SDL_SetRenderDrawColor(sdlRenderer, 12, 14, 110, 255);
    SDL_Rect outer{x, y, CELL, CELL};
    SDL_RenderFillRect(sdlRenderer, &outer);
    
    SDL_SetRenderDrawColor(sdlRenderer, 36, 70, 255, 255);
    SDL_Rect inner{x + 3, y + 3, CELL - 6, CELL - 6};
    SDL_RenderFillRect(sdlRenderer, &inner);
}

void Renderer::drawDoorTile(int x, int y) {
    SDL_SetRenderDrawColor(sdlRenderer, 255, 182, 222, 255);
    SDL_Rect r{x + 2, y + CELL / 2 - 2, CELL - 4, 4};
    SDL_RenderFillRect(sdlRenderer, &r);
}

void Renderer::drawMaze(const Maze& maze) {
    int ox = 0, oy = TOP_MARGIN;
    Uint32 ticks = SDL_GetTicks();

    for (int r = 0; r < GRID_H; r++) {
        for (int c = 0; c < GRID_W; c++) {
            int x = ox + c * CELL, y = oy + r * CELL;
            int t = maze.grid[r][c];
            
            if (t == WALL) {
                drawWallTile(x, y);
            } 
            else if (t == DOOR) {
                drawDoorTile(x, y);
            } 
            else if (t == PELLET) {
                SDL_Color col{255, 200, 170, 255};
                fillArc(x + CELL / 2.0f, y + CELL / 2.0f, 3.0f, col, 0, 360, 8);
            } 
            else if (t == POWER) {
                // Hiệu ứng hạt năng lượng to nhỏ liên tục theo thời gian (ticks)
                float rad = 6.0f + 2.0f * std::sin(ticks * 0.006f);
                SDL_Color col{255, 220, 180, 255};
                fillArc(x + CELL / 2.0f, y + CELL / 2.0f, rad, col, 0, 360, 16);
            }
        }
    }
}

void Renderer::drawPacMan(const PacMan& pac) {
    int ox = 0, oy = TOP_MARGIN;
    
    // Tính toán pixel trên màn hình dựa vào hàm lerp
    float px = ox + lerp((float)pac.cx, (float)pac.tx, pac.moveT) * CELL + CELL / 2.0f;
    float py = oy + lerp((float)pac.cy, (float)pac.ty, pac.moveT) * CELL + CELL / 2.0f;
    float radius = CELL * 0.42f;
    
    // Tính góc mở miệng dựa trên hàm sin
    float mouth = 6.0f + 34.0f * std::fabs(std::sin(pac.animTime * 9.0f));
    if (pac.dir == Dir::None) mouth = 12.0f; // Đứng im thì ngậm miệng vừa phải
    
    SDL_Color yellow{255, 221, 0, 255};
    float faceAngle = dirAngleDeg(pac.facing);
    
    fillArc(px, py, radius, yellow, faceAngle + mouth, faceAngle + 360.0f - mouth, 24);
}

// --- BẮT ĐẦU PHẦN VẼ MA ---

// Hàm vẽ tam giác cơ bản (dùng cho chân răng cưa của ma)
void Renderer::fillTri(float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color col) {
    SDL_Vertex v[3];
    v[0].position = {x1, y1}; v[0].color = col; v[0].tex_coord = {0, 0};
    v[1].position = {x2, y2}; v[1].color = col; v[1].tex_coord = {0, 0};
    v[2].position = {x3, y3}; v[2].color = col; v[2].tex_coord = {0, 0};
    SDL_RenderGeometry(sdlRenderer, nullptr, v, 3, nullptr, 0);
}

// Hàm vẽ hình dáng vòm của ma
void Renderer::drawGhostShape(float cx, float cy, float radius, SDL_Color body, Dir dir, SDL_Color bg) {
    float domeCy = cy - radius * 0.15f;
    // Vẽ vòm đầu
    fillArc(cx, domeCy, radius, body, 180.0f, 360.0f, 16);
    
    // Vẽ thân hình chữ nhật
    SDL_SetRenderDrawColor(sdlRenderer, body.r, body.g, body.b, body.a);
    SDL_Rect bodyRect{ (int)(cx - radius), (int)domeCy, (int)(radius * 2), (int)(radius * 1.15f) };
    SDL_RenderFillRect(sdlRenderer, &bodyRect);

    // Vẽ 3 cái chân (răng cưa)
    float bottom = domeCy + radius * 1.15f;
    float left = cx - radius;
    float w = (radius * 2.0f) / 3.0f;
    for (int i = 0; i < 3; i++) {
        float x0 = left + i * w, x1 = left + (i + 1) * w, xm = (x0 + x1) * 0.5f;
        fillTri(x0, bottom - radius * 0.35f, x1, bottom - radius * 0.35f, xm, bottom, bg);
    }

    // Vẽ mắt nhìn theo hướng di chuyển
    drawEyesOnly(cx, cy, radius, dir);
}

// Hàm vẽ đôi mắt
void Renderer::drawEyesOnly(float cx, float cy, float radius, Dir dir) {
    Point dv = dirVec(dir == Dir::None ? Dir::Right : dir);
    float eyeR = radius * 0.30f, pupilR = radius * 0.15f;
    float ex1 = cx - radius * 0.35f, ex2 = cx + radius * 0.35f, ey = cy - radius * 0.2f;
    
    SDL_Color white{240, 240, 255, 255};
    fillArc(ex1, ey, eyeR, white, 0, 360, 12);
    fillArc(ex2, ey, eyeR, white, 0, 360, 12);
    
    SDL_Color pupil{20, 20, 140, 255};
    fillArc(ex1 + dv.x * eyeR * 0.4f, ey + dv.y * eyeR * 0.4f, pupilR, pupil, 0, 360, 10);
    fillArc(ex2 + dv.x * eyeR * 0.4f, ey + dv.y * eyeR * 0.4f, pupilR, pupil, 0, 360, 10);
}

// Hàm vòng lặp vẽ toàn bộ ma trong danh sách
void Renderer::drawGhosts(const std::vector<Ghost>& ghosts, float frightenTimer) {
    int ox = 0, oy = TOP_MARGIN;
    SDL_Color bg{0, 0, 0, 255}; // Màu nền để vẽ khe hở giữa các chân ma
    float radius = CELL * 0.42f;

   for (const Ghost& gh : ghosts) {
        float gx, gy;
        if (gh.mode == GMode::InHouse) {
            gx = ox + gh.cx * CELL + CELL / 2.0f;
            gy = oy + gh.cy * CELL + CELL / 2.0f + std::sin(gh.bobTime * 4.0f) * 3.0f;
        } else {
            gx = ox + lerp((float)gh.cx, (float)gh.tx, gh.moveT) * CELL + CELL / 2.0f;
            gy = oy + lerp((float)gh.cy, (float)gh.ty, gh.moveT) * CELL + CELL / 2.0f;
        }

        // TÙY THEO TRẠNG THÁI MÀ VẼ HÌNH DÁNG KHÁC NHAU
        if (gh.mode == GMode::Eaten) {
            drawEyesOnly(gx, gy, radius, gh.dir); // Chỉ vẽ mắt bay về
        } 
        else if (gh.mode == GMode::Frightened) {
            // Hiệu ứng nhấp nháy trắng khi sắp hết thời gian năng lượng
            bool flash = (frightenTimer > 0.0f && frightenTimer < 2.0f && (int)(frightenTimer * 5) % 2 == 0);
            SDL_Color fCol = flash ? SDL_Color{255, 255, 255, 255} : SDL_Color{30, 30, 200, 255};
            drawGhostShape(gx, gy, radius, fCol, gh.dir, bg);
        } 
        else {
            drawGhostShape(gx, gy, radius, gh.color, gh.dir, bg);
        }
    }
}
// --- KẾT THÚC PHẦN VẼ MA ---
void Renderer::drawHUD(int score, int lives, bool gameOver, bool win) {
    if (!font) return; // Nếu lỗi không tải được font thì bỏ qua vẽ chữ

    SDL_Color white = {255, 255, 255, 255};
    SDL_Color yellow = {255, 221, 0, 255};
    SDL_Color red = {255, 50, 50, 255};
    SDL_Color green = {80, 255, 120, 255};
    
    char buf[64];

    // Vẽ Điểm số (Góc trên bên trái)
    sprintf(buf, "SCORE: %d", score);
    SDL_Surface* surf = TTF_RenderText_Solid(font, buf, white);
    if (surf) {
        SDL_Texture* tex = SDL_CreateTextureFromSurface(sdlRenderer, surf);
        SDL_Rect rect{ 10, 5, surf->w, surf->h };
        SDL_RenderCopy(sdlRenderer, tex, nullptr, &rect);
        SDL_DestroyTexture(tex);
        SDL_FreeSurface(surf);
    }

    // Vẽ Số mạng (Góc dưới bên trái)
    sprintf(buf, "LIVES: %d", lives);
    surf = TTF_RenderText_Solid(font, buf, yellow);
    if (surf) {
        SDL_Texture* tex = SDL_CreateTextureFromSurface(sdlRenderer, surf);
        SDL_Rect rect{ 10, SCREEN_H - 35, surf->w, surf->h };
        SDL_RenderCopy(sdlRenderer, tex, nullptr, &rect);
        SDL_DestroyTexture(tex);
        SDL_FreeSurface(surf);
    }

    // Vẽ chữ GAME OVER hoặc YOU WIN to ở giữa màn hình
    if (gameOver || win) {
        const char* msg = win ? "YOU WIN! - PRESS ENTER" : "GAME OVER - PRESS ENTER";
        SDL_Color msgColor = win ? green : red;

        surf = TTF_RenderText_Solid(font, msg, msgColor);
        if (surf) {
            SDL_Texture* tex = SDL_CreateTextureFromSurface(sdlRenderer, surf);
            SDL_Rect rect{ (SCREEN_W - surf->w) / 2, (SCREEN_H - surf->h) / 2, surf->w, surf->h };

            // Tạo nền đen mờ đằng sau chữ cho dễ đọc
            SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 200);
            SDL_Rect bgRect{rect.x - 10, rect.y - 10, rect.w + 20, rect.h + 20};
            SDL_RenderFillRect(sdlRenderer, &bgRect);
            SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_NONE);

            SDL_RenderCopy(sdlRenderer, tex, nullptr, &rect);
            SDL_DestroyTexture(tex);
            SDL_FreeSurface(surf);

            if (win) {
                sprintf(buf, "FINAL SCORE: %d", score);
                SDL_Surface* scoreSurf = TTF_RenderText_Solid(font, buf, white);
                if (scoreSurf) {
                    SDL_Texture* scoreTex = SDL_CreateTextureFromSurface(sdlRenderer, scoreSurf);
                    SDL_Rect scoreRect{ (SCREEN_W - scoreSurf->w) / 2, rect.y + rect.h + 12, scoreSurf->w, scoreSurf->h };
                    SDL_RenderCopy(sdlRenderer, scoreTex, nullptr, &scoreRect);
                    SDL_DestroyTexture(scoreTex);
                    SDL_FreeSurface(scoreSurf);
                }
            }
        }
    }
}

void Renderer::draw3D(const PacMan& pac, const Maze& maze) {
   // 1. LẤY TRỰC TIẾP TỌA ĐỘ SỐ THỰC FLOAT (Sửa thành dòng này)
    float px = pac.x;
    float py = pac.y;
    
    // Góc nhìn hiện tại bắt từ chuột (Giữ nguyên)
    float pa = pac.angle; 
    
    // Thông số Camera (Giữ nguyên)
    float fov = 60.0f * (M_PI / 180.0f);
    float halfFov = fov / 2.0f;

    // Vẽ bầu trời (Trần nhà) và Mặt đất
    SDL_SetRenderDrawColor(sdlRenderer, 50, 50, 50, 255); // Xám đậm
    SDL_Rect ceiling = {0, 0, SCREEN_W, SCREEN_H / 2};
    SDL_RenderFillRect(sdlRenderer, &ceiling);
    
    SDL_SetRenderDrawColor(sdlRenderer, 30, 30, 30, 255); // Xám đen
    SDL_Rect floor = {0, SCREEN_H / 2, SCREEN_W, SCREEN_H / 2};
    SDL_RenderFillRect(sdlRenderer, &floor);

    // 2. VÒNG LẶP DDA QUÉT TỪNG CỘT PIXEL TRÊN MÀN HÌNH
    for (int x = 0; x < SCREEN_W; x++) {
        // Tính toán góc của tia hiện tại
        float rayAngle = (pa - halfFov) + ((float)x / (float)SCREEN_W) * fov;
        
        // Véc-tơ hướng của tia
        float rayDirX = cos(rayAngle);
        float rayDirY = sin(rayAngle);

        // Tọa độ ô lưới mà tia đang đứng
        int mapX = (int)px;
        int mapY = (int)py;

        // Tính khoảng cách delta như công thức toán học
        float deltaDistX = std::abs(1.0f / rayDirX);
        float deltaDistY = std::abs(1.0f / rayDirY);

        float sideDistX, sideDistY;
        int stepX, stepY;
        bool hit = false; // Đã chạm tường chưa?
        int side = 0;     // Chạm vào mặt đứng hay mặt ngang của bức tường?

        // Tính toán bước nhảy ban đầu
        if (rayDirX < 0) {
            stepX = -1;
            sideDistX = (px - mapX) * deltaDistX;
        } else {
            stepX = 1;
            sideDistX = (mapX + 1.0f - px) * deltaDistX;
        }

        if (rayDirY < 0) {
            stepY = -1;
            sideDistY = (py - mapY) * deltaDistY;
        } else {
            stepY = 1;
            sideDistY = (mapY + 1.0f - py) * deltaDistY;
        }

        // 3. THỰC THI BƯỚC NHẢY DDA (Không nhân/chia)
        while (!hit) {
            // Nhảy sang ô tiếp theo
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0; // Chạm tường dọc (Trục X)
            } else {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1; // Chạm tường ngang (Trục Y)
            }

            // Kiểm tra xem tia có văng ra khỏi bản đồ không
            if (mapX < 0 || mapX >= GRID_W || mapY < 0 || mapY >= GRID_H) {
                hit = true; 
            } 
            // Nếu đập vào khối WALL
            else if (maze.grid[mapY][mapX] == WALL || maze.grid[mapY][mapX] == DOOR) {
                hit = true;
            }
        }

       // 4. TÍNH TOÁN KHOẢNG CÁCH THỰC (Đã triệt tiêu hiệu ứng Mắt cá - Fisheye)
        float perpWallDist;
        if (side == 0) perpWallDist = (mapX - px + (1 - stepX) / 2) / rayDirX;
        else           perpWallDist = (mapY - py + (1 - stepY) / 2) / rayDirY;

        perpWallDist *= cos(rayAngle - pa);
        if (perpWallDist <= 0.001f) perpWallDist = 0.001f;
        // xử lý Z-buffer để vẽ ma sau này
        ZBuffer[x] = perpWallDist;

        // --- THÊM SƯƠNG MÙ (FOG) VÀO TƯỜNG ---
        float MAX_FOG = 6.0f; 
        float fog = 1.0f - (perpWallDist / MAX_FOG);
        if (fog < 0.0f) fog = 0.0f;

        int lineHeight = (int)(SCREEN_H / perpWallDist);
        int drawStart = -lineHeight / 2 + SCREEN_H / 2;
        if (drawStart < 0) drawStart = 0;
        
        int drawEnd = lineHeight / 2 + SCREEN_H / 2;
        if (drawEnd >= SCREEN_H) drawEnd = SCREEN_H - 1;

        SDL_Color wallColor = {30, 30, 200, 255}; 
        if (side == 1) { 
            wallColor.r /= 2; wallColor.g /= 2; wallColor.b /= 2; 
        }
        if (maze.grid[mapY][mapX] == DOOR) {
            wallColor = {255, 180, 180, 255};
        }

        // Áp dụng lớp đen sương mù lên màu tường
        wallColor.r = (Uint8)(wallColor.r * fog);
        wallColor.g = (Uint8)(wallColor.g * fog);
        wallColor.b = (Uint8)(wallColor.b * fog);

        SDL_SetRenderDrawColor(sdlRenderer, wallColor.r, wallColor.g, wallColor.b, 255);
        SDL_RenderDrawLine(sdlRenderer, x, drawStart, x, drawEnd);
    }
}

// Tính vị trí vẽ thực tế của 1 con ma trên bản đồ: nội suy ô hiện tại -> ô kế tiếp,
// cộng thêm lệch hình riêng (visualOffset) và hiệu ứng đung đưa nhẹ (wobble) để các
// con ma không bao giờ trông như dính khít vào nhau và không di chuyển cứng như robot.
static void ghostRenderPos(const Ghost& gh, float& gx, float& gy) {
    gx = lerpWrapX((float)gh.cx, (float)gh.tx, gh.moveT) + 0.5f + gh.visualOffsetX;
    gy = gh.cy + (gh.ty - gh.cy) * gh.moveT + 0.5f + gh.visualOffsetY;

    if (gh.mode == GMode::InHouse) {
        gy += std::sin(gh.bobTime * 4.0f) * 0.1f;
    } else {
        // Đung đưa nhẹ vuông góc theo thời gian, lệch pha riêng theo từng con ma
        gx += std::sin(gh.bobTime * 5.0f + gh.visualOffsetX * 10.0f) * 0.05f;
        gy += std::cos(gh.bobTime * 5.0f + gh.visualOffsetY * 10.0f) * 0.05f;
    }
}

void Renderer::drawGhosts3D(const PacMan& pac, const std::vector<Ghost>& ghosts, float frightenTimer) {
    if (ghosts.empty()) return;

    // 1. Lấy vị trí và góc nhìn hiện tại của Pac-Man (tương tự như hàm draw3D)
    float px = pac.x;
    float py = pac.y;
    
    float dirAngle = dirAngleDeg(pac.facing);
    float pa = pac.angle; // Góc nhìn hiện tại của Pac
    
    float fov = 60.0f * (M_PI / 180.0f);

    // Cấu trúc tạm để lưu các con ma kèm khoảng cách, dùng để sắp xếp (Sort)
    // Nguyên tắc 3D: Con nào ở xa phải vẽ trước, con ở gần vẽ sau để đè lên con ở xa
    struct SpriteOrder {
        const Ghost* ghost;
        float distance;
    };
    std::vector<SpriteOrder> spriteList;

    for (const auto& gh : ghosts) {
        float gx, gy;
        ghostRenderPos(gh, gx, gy);

        float dx = gx - px;
        float dy = gy - py;
        float dist = std::sqrt(dx * dx + dy * dy);

        spriteList.push_back({&gh, dist});
    }

    // Sắp xếp giảm dần theo khoảng cách (Far to Near)
    std::sort(spriteList.begin(), spriteList.end(), [](const SpriteOrder& a, const SpriteOrder& b) {
        return a.distance > b.distance;
    });

    // 2. VÒNG LẶP VẼ TỪNG CON MA SAU KHI ĐÃ SẮP XẾP
    for (const auto& item : spriteList) {
        const Ghost& gh = *(item.ghost);
        float dist = item.distance;

        if (dist < 0.1f) continue; // Quá sát mắt thì bỏ qua không vẽ để tránh lỗi toán học

        float gx, gy;
        ghostRenderPos(gh, gx, gy);

        float spriteX = gx - px;
        float spriteY = gy - py;

        // Tính góc tuyệt đối của con ma so với Pac-Man
        float spriteAngle = std::atan2(spriteY, spriteX);
        
        // Tính góc lệch giữa hướng con ma và hướng nhìn của góc Camera
        float beta = spriteAngle - pa;

        // Chuẩn hóa góc beta về khoảng [-PI, PI]
        // Chuẩn hóa góc beta về khoảng [-PI, PI]
        while (beta < -M_PI) beta += 2.0f * M_PI;
        while (beta >  M_PI) beta -= 2.0f * M_PI;

        if (std::abs(beta) > (fov / 2.0f) + 0.5f) continue;

        dist *= std::cos(beta);
        if (dist <= 0.1f) continue;

        // --- THÊM SƯƠNG MÙ CHO MA (Ngoài 6 ô là tàng hình) ---
        float MAX_FOG = 6.0f;
        float fog = 1.0f - (dist / MAX_FOG);
        if (fog <= 0.0f) continue; 

        float ghostScale = 0.6f; 
        
        int spriteHeight = (int)((SCREEN_H / dist) * ghostScale);
        int spriteWidth = spriteHeight;

        float screenXCenter = (SCREEN_W / 2.0f) + (beta / (fov / 2.0f)) * (SCREEN_W / 2.0f);
        int drawStartX = (int)(screenXCenter - spriteWidth / 2.0f);
        int verticalOffset = (int)(SCREEN_H / dist * (1.0f - ghostScale) * 0.5f);
        int drawStartY = (int)(-spriteHeight / 2.0f + SCREEN_H / 2.0f) + verticalOffset;
        // Vẽ 1 cột (stripe) rộng 1px tại screenX, chỉ khi không có tường nào
        // gần hơn con ma đang che khuất đúng cột đó -> clip chính xác theo từng pixel ngang.
        auto drawColumnClipped = [&](int screenX, int y, int h) {
            if (screenX < 0 || screenX >= SCREEN_W) return;
            if (dist > ZBuffer[screenX]) return; // Tường ở cột này gần hơn ma -> bỏ qua cột đó
            SDL_Rect col = { screenX, y, 1, h };
            SDL_RenderFillRect(sdlRenderer, &col);
        };

        SDL_Color color = gh.color;
        if (gh.mode == GMode::Eaten) {
            color = {200, 200, 255, 100};
        } else if (gh.mode == GMode::Frightened) {
            bool flash = (frightenTimer > 0.0f && frightenTimer < 2.0f && (int)(frightenTimer * 5) % 2 == 0);
            color = flash ? SDL_Color{255, 255, 255, 255} : SDL_Color{30, 30, 230, 255};
        }

        // Áp dụng lớp đen sương mù lên màu của ma
        color.r = (Uint8)(color.r * fog);
        color.g = (Uint8)(color.g * fog);
        color.b = (Uint8)(color.b * fog);

        // --- Vẽ thân ma, từng cột (stripe) một, có clip theo ZBuffer ---
        SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a);
        for (int stripe = 0; stripe < spriteWidth; stripe++) {
            drawColumnClipped(drawStartX + stripe, drawStartY, spriteHeight);
        }

        // --- Vẽ đốm mắt, cũng clip theo từng cột để không "lòi" qua tường ---
        SDL_SetRenderDrawColor(sdlRenderer, (Uint8)(255 * fog), (Uint8)(255 * fog), (Uint8)(255 * fog), 255);
        int eyeSize = std::max(2, spriteWidth / 6);
        int eyeY = drawStartY + spriteHeight / 4;
        int leftEyeX = drawStartX + spriteWidth / 4;
        int rightEyeX = drawStartX + (3 * spriteWidth) / 4 - eyeSize;
        for (int i = 0; i < eyeSize; i++) {
            drawColumnClipped(leftEyeX + i, eyeY, eyeSize);
            drawColumnClipped(rightEyeX + i, eyeY, eyeSize);
        }
    }
}

void Renderer::drawPellets3D(const PacMan& pac, const Maze& maze) {
    float px = pac.x;
    float py = pac.y;
    float pa = pac.angle;
    float fov = 60.0f * (M_PI / 180.0f);

    struct PelletSprite {
        float x, y, dist; int type; 
    };
    std::vector<PelletSprite> pelletList;

    int startX = std::max(0, (int)px - 12);
    int endX = std::min(GRID_W, (int)px + 12);
    int startY = std::max(0, (int)py - 12);
    int endY = std::min(GRID_H, (int)py + 12);

    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            int cell = maze.grid[y][x];
            if (cell == PELLET || cell == POWER) {
                float cellX = x + 0.5f; float cellY = y + 0.5f;
                float dx = cellX - px; float dy = cellY - py;
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist < 0.2f || dist > 12.0f) continue;
                pelletList.push_back({cellX, cellY, dist, cell});
            }
        }
    }

    std::sort(pelletList.begin(), pelletList.end(), [](const PelletSprite& a, const PelletSprite& b) {
        return a.dist > b.dist;
    });

    for (const auto& pellet : pelletList) {
        float dx = pellet.x - px; float dy = pellet.y - py;
        float spriteAngle = std::atan2(dy, dx);
        float beta = spriteAngle - pa;

        while (beta < -M_PI) beta += 2.0f * M_PI;
        while (beta >  M_PI) beta -= 2.0f * M_PI;

        if (std::abs(beta) > (fov / 2.0f) + 0.2f) continue;

        float correctedDist = pellet.dist * std::cos(beta);
        if (correctedDist <= 0.1f) continue;

        // Sương mù cho hạt
        float MAX_FOG = 6.0f;
        float fog = 1.0f - (correctedDist / MAX_FOG);
        if (fog <= 0.0f) continue;

        int baseHeight = (int)(SCREEN_H / correctedDist);
        int size = (pellet.type == POWER) ? (baseHeight / 3) : (baseHeight / 6);
        size = std::max(2, std::min(size, SCREEN_H / 4));

        float screenXCenter = (SCREEN_W / 2.0f) + (beta / (fov / 2.0f)) * (SCREEN_W / 2.0f);
        int drawStartX = (int)(screenXCenter - size / 2.0f);
        int drawStartY = (SCREEN_H / 2) + (baseHeight / 8) - (size / 2);

        int checkX = (int)screenXCenter;
        if (checkX >= 0 && checkX < SCREEN_W) {
            // Nếu khoảng cách của hạt xa hơn bức tường đang chắn trước mặt -> Hủy vẽ!
            if (correctedDist > ZBuffer[checkX]) continue;
        }

        Uint8 r = 255, g = 200, b = 150;
        if (pellet.type == POWER) {
            Uint32 ticks = SDL_GetTicks();
            if ((ticks / 200) % 2 == 0) { r = 255; g = 100; b = 100; } 
            else { r = 255; g = 255; b = 255; }
        }
        
        SDL_SetRenderDrawColor(sdlRenderer, (Uint8)(r * fog), (Uint8)(g * fog), (Uint8)(b * fog), 255);
        SDL_Rect pelletRect = { drawStartX, drawStartY, size, size };
        SDL_RenderFillRect(sdlRenderer, &pelletRect);
    }
}

void Renderer::drawMiniMap(const PacMan& pac, const std::vector<Ghost>& ghosts, const Maze& maze) {
    // --- Radar la bàn: luôn xoay sao cho hướng nhìn của Pac-Man chỉ thẳng lên trên,
    //     và chỉ hiện những gì trong bán kính "cảm nhận" (Fog of War) ---
    const float RADAR_WORLD_RADIUS = 7.0f;               // Bán kính cảm nhận, tính theo số ô
    const float RADAR_PIXEL_RADIUS = 70.0f;               // Bán kính khung radar trên màn hình
    const float scale = RADAR_PIXEL_RADIUS / RADAR_WORLD_RADIUS;

    const int radarCX = SCREEN_W - (int)RADAR_PIXEL_RADIUS - 20;
    const int radarCY = (int)RADAR_PIXEL_RADIUS + 20;

    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
    fillArc((float)radarCX, (float)radarCY, RADAR_PIXEL_RADIUS + 2.0f, SDL_Color{200, 200, 255, 160}, 0, 360, 40); // viền
    fillArc((float)radarCX, (float)radarCY, RADAR_PIXEL_RADIUS, SDL_Color{0, 0, 0, 195}, 0, 360, 40);              // nền

    // Ma trận xoay: quay thế giới quanh Pac-Man theo -(pac.angle + 90°) để hướng nhìn
    // hiện tại luôn trùng với đỉnh (12 giờ) của radar, giống la bàn xoay theo hướng nhìn.
    float theta = -(pac.angle + (float)M_PI / 2.0f);
    float cosT = std::cos(theta), sinT = std::sin(theta);

    // Chuyển toạ độ thế giới (wx,wy) sang toạ độ màn hình trên radar (rx,ry).
    // Trả về false nếu ở ngoài bán kính cảm nhận -> bị "Sương mù chiến tranh" che khuất.
    auto worldToRadar = [&](float wx, float wy, float& outX, float& outY, float& outDist) -> bool {
        float dx = wx - pac.x;
        float dy = wy - pac.y;
        outDist = std::sqrt(dx * dx + dy * dy);
        if (outDist > RADAR_WORLD_RADIUS) return false;

        float rx = dx * cosT - dy * sinT;
        float ry = dx * sinT + dy * cosT;
        outX = radarCX + rx * scale;
        outY = radarCY + ry * scale;
        return true;
    };

    // --- Sóng ra-đa quét tường + hạt định kỳ ---
    // Không hiện liên tục nữa (tránh việc chỉ cần nhìn radar là đi thẳng ăn hết bản đồ mà
    // không cần chơi màn 3D). Thay vào đó, cứ mỗi WAVE_INTERVAL giây một vòng sóng lan ra
    // từ tâm; ô nào vừa bị sóng quét qua sẽ sáng lên rồi mờ dần, sau đó tối om cho tới đợt quét kế tiếp.
    const float WAVE_INTERVAL = 5.0f;    // Khoảng cách giữa 2 lần quét (giây) - chỉnh ở đây nếu muốn quét dày/thưa hơn
    const float WAVE_TRAVEL_TIME = 1.4f; // Thời gian sóng lan từ tâm ra rìa radar
    const float WAVE_AFTERGLOW = 0.5f;   // Một ô còn sáng bao lâu sau khi sóng vừa quét qua

    Uint32 ticks = SDL_GetTicks();
    float cyclePos = std::fmod(ticks / 1000.0f, WAVE_INTERVAL); // Vị trí hiện tại trong chu kỳ quét

    // Vòng sóng đang lan ra: một quầng sáng mờ dần khi mở rộng, vẽ trước để hạt/tường đè lên trên
    if (cyclePos <= WAVE_TRAVEL_TIME) {
        float waveWorldRadius = (cyclePos / WAVE_TRAVEL_TIME) * RADAR_WORLD_RADIUS;
        float wavePixelRadius = waveWorldRadius * scale;
        Uint8 waveAlpha = (Uint8)(140.0f * (1.0f - cyclePos / WAVE_TRAVEL_TIME));
        fillArc((float)radarCX, (float)radarCY, wavePixelRadius, SDL_Color{140, 220, 255, waveAlpha}, 0, 360, 40);
    }

    int ix = (int)pac.x, iy = (int)pac.y;
    int rTiles = (int)std::ceil(RADAR_WORLD_RADIUS);
    for (int y = iy - rTiles; y <= iy + rTiles; y++) {
        if (y < 0 || y >= GRID_H) continue;
        for (int x = ix - rTiles; x <= ix + rTiles; x++) {
            int wx = ((x % GRID_W) + GRID_W) % GRID_W; // Vòng qua đường hầm trái/phải
            int cell = maze.grid[y][wx];
            if (cell != PELLET && cell != POWER && cell != WALL) continue;

            float sx, sy, dist;
            if (!worldToRadar((float)wx + 0.5f, (float)y + 0.5f, sx, sy, dist)) continue;

            // Thời điểm (trong chu kỳ) mà vòng sóng quét ngang qua đúng ô này
            float hitTime = (dist / RADAR_WORLD_RADIUS) * WAVE_TRAVEL_TIME;
            float sinceHit = cyclePos - hitTime;
            if (sinceHit < 0.0f || sinceHit > WAVE_AFTERGLOW) continue; // Chưa bị quét tới, hoặc đã mờ hẳn

            float glow = 1.0f - (sinceHit / WAVE_AFTERGLOW); // 1 = vừa quét qua, 0 = sắp tắt hẳn
            if (cell == WALL) {
                SDL_SetRenderDrawColor(sdlRenderer, (Uint8)(90 * glow), (Uint8)(90 * glow), (Uint8)(220 * glow), 255);
                SDL_Rect wr{ (int)sx - 2, (int)sy - 2, 4, 4 };
                SDL_RenderFillRect(sdlRenderer, &wr);
            } else {
                SDL_Color c = (cell == POWER) ? SDL_Color{255, 100, 100, 255} : SDL_Color{220, 220, 100, 255};
                SDL_SetRenderDrawColor(sdlRenderer, (Uint8)(c.r * glow), (Uint8)(c.g * glow), (Uint8)(c.b * glow), 255);
                SDL_Rect pr{ (int)sx - 1, (int)sy - 1, 2, 2 };
                SDL_RenderFillRect(sdlRenderer, &pr);
            }
        }
    }

    // --- Ma vẫn hiện LIÊN TỤC (không phụ thuộc sóng quét) - đây là máy dò nguy hiểm,
    //     cần cảnh báo ngay lập tức chứ không đợi tới đợt quét kế tiếp ---
    for (const auto& gh : ghosts) {
        if (gh.mode == GMode::InHouse) continue;
        float gx = gh.cx + (gh.tx - gh.cx) * gh.moveT + 0.5f;
        float gy = gh.cy + (gh.ty - gh.cy) * gh.moveT + 0.5f;

        float sx, sy, dist;
        if (!worldToRadar(gx, gy, sx, sy, dist)) continue;

        float fade = 1.0f - (dist / RADAR_WORLD_RADIUS);
        SDL_SetRenderDrawColor(sdlRenderer, (Uint8)(gh.color.r * fade), (Uint8)(gh.color.g * fade), (Uint8)(gh.color.b * fade), 255);
        SDL_Rect gr{ (int)sx - 3, (int)sy - 3, 6, 6 };
        SDL_RenderFillRect(sdlRenderer, &gr);
    }

    // --- Pac-Man luôn đứng yên tại tâm radar, mũi tên luôn chỉ thẳng lên trên vì thế giới đã xoay quanh nó ---
    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 0, 255);
    SDL_Rect pr{ radarCX - 3, radarCY - 3, 6, 6 };
    SDL_RenderFillRect(sdlRenderer, &pr);
    SDL_RenderDrawLine(sdlRenderer, radarCX, radarCY, radarCX, radarCY - 10);

    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_NONE);
}