#pragma once

const int   CELL          = 24;
const int   GRID_W        = 21;
const int   GRID_H        = 21;
const int   TOP_MARGIN    = 44;
const int   BOTTOM_MARGIN = 40;
const int   SCREEN_W      = CELL * GRID_W;
const int   SCREEN_H      = TOP_MARGIN + CELL * GRID_H + BOTTOM_MARGIN;
const float PI_F          = 3.14159265358979323846f;

enum class Dir { None, Up, Down, Left, Right };

struct Point { int x, y; };

inline Dir opposite(Dir d) {
    switch (d) {
        case Dir::Up:    return Dir::Down;
        case Dir::Down:  return Dir::Up;
        case Dir::Left:  return Dir::Right;
        case Dir::Right: return Dir::Left;
        default:         return Dir::None;
    }
}

inline Point dirVec(Dir d) {
    switch (d) {
        case Dir::Up:    return {0, -1};
        case Dir::Down:  return {0, 1};
        case Dir::Left:  return {-1, 0};
        case Dir::Right: return {1, 0};
        default:         return {0, 0};
    }
}

inline float dirAngleDeg(Dir d) {
    switch (d) {
        case Dir::Right: return 0.0f;
        case Dir::Down:  return 90.0f;
        case Dir::Left:  return 180.0f;
        case Dir::Up:    return 270.0f;
        default:         return 0.0f;
    }
}

inline Point neighborWrap(int cx, int cy, Dir d) {
    Point v = dirVec(d);
    int nx = cx + v.x, ny = cy + v.y;
    if (nx < 0) nx = GRID_W - 1;
    if (nx >= GRID_W) nx = 0;
    return {nx, ny};
}

inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

// Nội suy trục X có tính đến đường hầm (tunnel) xuyên map: nếu ô đích cách xa
// ô hiện tại hơn nửa bản đồ, coi như đã đi qua đường hầm thay vì cắt thẳng qua tường.
inline float lerpWrapX(float cx, float tx, float t) {
    float dx = tx - cx;
    if (dx > GRID_W / 2.0f) dx -= GRID_W;
    else if (dx < -GRID_W / 2.0f) dx += GRID_W;
    float x = cx + dx * t;
    if (x < 0.0f) x += GRID_W;
    if (x >= GRID_W) x -= GRID_W;
    return x;
}