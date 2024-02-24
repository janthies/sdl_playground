PROJECTNAME = project
OUTPUT_DIR = build

CFLAGS= -Wno-narrowing -fpermissive -g -fsanitize=address

INCLUDE_DIRS = -Iinclude/SDL2 -Iinclude/imgui
LIB_DIRS = -Llib

LIBS = -lmingw32 -lSDL2main -lSDL2

SRC_DIR = src
IMGUI_DIR = imgui

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
HEADERS = $(wildcard $(SRC_DIR)/*.h)
IMGUI_SRCS = $(wildcard $(IMGUI_DIR)/*.cpp)

OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OUTPUT_DIR)/%.o)
IMGUI_OBJS = $(IMGUI_SRCS:$(IMGUI_DIR)/%.cpp=$(OUTPUT_DIR)/%.o)

TEST_FILE_HEADERS = src/types.h src/vector.h
TEST_FILE_SRC = src/vector.cpp

vector: $(TEST_FILE_SRC) $(TEST_FILE_HEADERS)
	clang $(CFLAGS) -o vector $(TEST_FILE_SRC) -lmingw32


default: $(OUTPUT_DIR)/$(PROJECTNAME)

$(OUTPUT_DIR)/$(PROJECTNAME): $(OBJS) $(IMGUI_OBJS)
	g++ $(CFLAGS) $^ -o $@ $(LIB_DIRS) $(LIBS)

$(OUTPUT_DIR)/%.o: $(SRC_DIR)/%.cpp
	g++ $(CFLAGS) -c $< -o $@ $(INCLUDE_DIRS)

$(OUTPUT_DIR)/%.o: $(IMGUI_DIR)/%.cpp
	g++ $(CFLAGS) -c $< -o $@ $(INCLUDE_DIRS)


.PHONY: run
run: vector
	./vector.exe