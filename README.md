# SDL2 Game

Install:
https://github.com/niXman/mingw-builds-binaries/releases
x86_64-16.1.0-release-posix-seh-ucrt-rt_v14-rev1.7z
Add mingw64/bin to environment variable
Compile: mingw32-make


| Library | Version |
|---|---|
| SDL2 | 2.32.10 |
| SDL2_image | 2.8.12 |
| SDL2_mixer | 2.8.2 |
| SDL2_ttf | 2.24.0 |

Everything is bundled: headers in `include/SDL2/`, import libs in `lib/`, runtime DLLs in the project root. No separate SDL installation needed.

## Game: Pac-Man

A from-scratch classic Pac-Man clone — procedurally-built maze, pixel-smooth
grid movement, 4 ghosts with distinct classic AI (Blinky direct-chases,
Pinky ambushes ahead, Inky uses the Blinky-reflection targeting trick, Clyde
chases/retreats based on distance), scatter/chase mode cycling, power
pellets with frightened/eaten ghost states, lives, levels, and score. All
sprites (Pac-Man, ghosts, pellets) are drawn procedurally via
`SDL_RenderGeometry` — no image assets needed.

- Arrow keys / WASD — move
- Enter — start / restart after game over
- Esc — quit
- `main.exe --validate` — runs a maze-connectivity flood-fill check and exits (no window)

It exercises SDL2 rendering (including `SDL_RenderGeometry` for circular
sprites) + input + timing, SDL2_ttf (HUD text, `assets/Southern.ttf`), and
SDL2_mixer (`assets/*.wav` sound effects). SDL2_image is initialized and
ready for PNG/JPG loading if you want to swap in real sprites later.

## Notes

- The Makefile builds every `*.cpp` in the folder, so just add new source files and rebuild.
- SDL2_image was built with PNG/JPG support (stb). SDL2_mixer supports WAV + OGG. SDL2_ttf uses vendored FreeType.
- `libwinpthread-1.dll` is required by MinGW-built executables; keep it next to `main.exe` when distributing.
