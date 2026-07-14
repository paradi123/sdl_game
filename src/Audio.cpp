#include "Audio.h"
#include <iostream>

Audio::Audio() : chomp(nullptr), eatGhost(nullptr), death(nullptr), begin(nullptr), ghostStep(nullptr) {}

Audio::~Audio() {
    close();
}

bool Audio::init() {
    // Khởi tạo kênh âm thanh với tần số chuẩn 44100Hz
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer loi: " << Mix_GetError() << "\n";
        return false;
    }

    // Tải các file âm thanh
    chomp = Mix_LoadWAV("assets/chomp.wav");
    eatGhost = Mix_LoadWAV("assets/eatghosh.wav");
    death = Mix_LoadWAV("assets/death.wav");
    begin = Mix_LoadWAV("assets/begin.wav");
    ghostStep = Mix_LoadWAV("assets/ghost_step.wav");

    if (!chomp || !eatGhost || !death || !begin || !ghostStep) {
        std::cout << "Canh bao: Thieu file .wav trong thu muc assets! Game van chay ma khong co tieng.\n";
    }

    return true;
}

void Audio::close() {
    if (chomp) { Mix_FreeChunk(chomp); chomp = nullptr; }
    if (eatGhost) { Mix_FreeChunk(eatGhost); eatGhost = nullptr; }
    if (death) { Mix_FreeChunk(death); death = nullptr; }
    if (begin) { Mix_FreeChunk(begin); begin = nullptr; }
    if (ghostStep) { Mix_FreeChunk(ghostStep); ghostStep = nullptr; }
    Mix_CloseAudio();
}

// Phát âm thanh trên các kênh (channel) khác nhau để chúng không bị đè lên nhau
void Audio::playChomp() { if (chomp) Mix_PlayChannel(0, chomp, 0); }
void Audio::playEatGhost() { if (eatGhost) Mix_PlayChannel(1, eatGhost, 0); }
void Audio::playDeath() { if (death) Mix_PlayChannel(2, death, 0); }
void Audio::playBegin() { if (begin) Mix_PlayChannel(3, begin, 0); }

static const int GHOST_LOOP_CHANNEL = 4;

void Audio::startGhostLoop(float proximity) {
    if (!ghostStep) return;
    // -1 lần lặp = phát lặp vô hạn, chỉ gọi 1 lần khi Ma vừa lọt vào tầm nghe
    Mix_PlayChannel(GHOST_LOOP_CHANNEL, ghostStep, -1);
    updateGhostLoopVolume(proximity);
}

void Audio::updateGhostLoopVolume(float proximity) {
    if (proximity < 0.0f) proximity = 0.0f;
    if (proximity > 1.0f) proximity = 1.0f;
    // Chỉnh âm lượng của KÊNH đang phát - không đụng tới Mix_PlayChannel nên không bị phát lại từ đầu
    Mix_Volume(GHOST_LOOP_CHANNEL, (int)(MIX_MAX_VOLUME * proximity));
}

void Audio::stopGhostLoop() {
    Mix_HaltChannel(GHOST_LOOP_CHANNEL);
}

bool Audio::isGhostLoopPlaying() const {
    return Mix_Playing(GHOST_LOOP_CHANNEL) != 0;
}