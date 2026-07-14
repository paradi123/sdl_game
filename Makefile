# SDL2 project - same layout as Sky-Garden, updated library versions:
#   SDL2 2.32.10, SDL2_image 2.8.12, SDL2_mixer 2.8.2, SDL2_ttf 2.24.0
#
# Build (Windows, MinGW-w64):
#   mingw32-make          (or just: make)
# Then run main.exe

COMPILER       = g++
SRC_DIR        = src
BUILD_DIR      = build
INCLUDE_PATH   = -Iinclude -Iinclude/SDL2
LIBRARY_PATH   = -Llib
COMPILER_FLAGS = -O2 -Wall -std=c++17 -MMD -MP
LINKER_FLAGS   = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -static-libgcc -static-libstdc++

# Every .cpp under src/ is picked up automatically - just add new files there.
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

all: main

main: $(OBJS)
	$(COMPILER) $(LIBRARY_PATH) -o main.exe $(OBJS) $(LINKER_FLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(COMPILER) $(COMPILER_FLAGS) $(INCLUDE_PATH) -c $< -o $@

$(BUILD_DIR):
	-@mkdir $(BUILD_DIR) 2>nul || mkdir -p $(BUILD_DIR) 2>/dev/null

-include $(DEPS)

clean:
	-del /Q $(BUILD_DIR)\*.o $(BUILD_DIR)\*.d main.exe 2>nul || rm -rf $(BUILD_DIR) main main.exe

.PHONY: all clean
