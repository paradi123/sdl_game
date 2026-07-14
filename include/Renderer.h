#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include "Maze.h"
#include <vector>

class PacMan;
class Ghost;

class Renderer {
public:
    SDL_Window* window;
    SDL_Renderer* sdlRenderer;
    TTF_Font* font;

    std::vector<float> ZBuffer;// Z-buffer để vẽ 3D

    Renderer();
    ~Renderer();


    bool init(const char* title, int width, int height);
    void close();
    void clear();
    void present();

    void drawMaze(const Maze& maze);
    void drawPacMan(const PacMan& pac);
    void drawGhosts(const std::vector<Ghost>& ghosts, float frightenTimer);
    void drawHUD(int score, int lives, bool gameOver, bool win);

    void draw3D(const PacMan& pac, const Maze& maze);
    void drawGhosts3D(const PacMan& pac, const std::vector<Ghost>& ghosts, float frightenTimer);
    void drawPellets3D(const PacMan& pac, const Maze& maze);
    void drawMiniMap(const PacMan& pac, const std::vector<Ghost>& ghosts, const Maze& maze); 

private:
    void drawWallTile(int x, int y);
    void drawDoorTile(int x, int y);
    void fillArc(float cx, float cy, float radius, SDL_Color col, float startDeg, float endDeg, int segments = 20);
    
    // Đảm bảo bạn cũng có các khai báo này ở phần private:
    void drawGhostShape(float cx, float cy, float radius, SDL_Color body, Dir dir, SDL_Color bg);
    void drawEyesOnly(float cx, float cy, float radius, Dir dir);
    void drawGhostFrightened(float cx, float cy, float radius, bool flashWhite, SDL_Color bg);
    void fillTri(float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color col);
};