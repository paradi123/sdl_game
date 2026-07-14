#pragma once
#include <SDL_mixer.h>

class Audio {
public:
    Mix_Chunk* chomp;
    Mix_Chunk* eatGhost;
    Mix_Chunk* death;
    Mix_Chunk* begin;
    Mix_Chunk* ghostStep;

    Audio();
    ~Audio();

    bool init();
    void close();

    void playChomp();
    void playEatGhost();
    void playDeath();
    void playBegin();

    // Một bản nhạc/tiếng động DUY NHẤT dùng chung cho cả bầy Ma, phát lặp liên tục
    // (không phát lại từ đầu mỗi khi có Ma khác lại gần) - chỉ tăng/giảm âm lượng
    // theo khoảng cách của con Ma gần nhất.
    void startGhostLoop(float proximity);   // Bắt đầu vòng lặp (chỉ gọi khi chưa phát)
    void updateGhostLoopVolume(float proximity); // Chỉnh âm lượng khi đang phát, KHÔNG phát lại
    void stopGhostLoop();                   // Dừng hẳn khi không còn Ma nào ở gần
    bool isGhostLoopPlaying() const;
};