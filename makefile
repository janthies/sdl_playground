PROJECTNAME = project
OUTPUT_DIR = build

CFLAGS= -Wno-narrowing -fpermissive -g

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

test: src/test.cpp src/types.h 
	clang $(CFLAGS) -o test src/test.cpp -lmingw32

vector: src/vector.cpp src/types.h src/cvec.h
	clang $(CFLAGS) -o vector src/vector.cpp -lmingw32

default: $(OUTPUT_DIR)/$(PROJECTNAME)

$(OUTPUT_DIR)/$(PROJECTNAME): $(OBJS) $(IMGUI_OBJS)
	g++ $(CFLAGS) $^ -o $@ $(LIB_DIRS) $(LIBS)

$(OUTPUT_DIR)/%.o: $(SRC_DIR)/%.cpp
	g++ $(CFLAGS) -c $< -o $@ $(INCLUDE_DIRS)

$(OUTPUT_DIR)/%.o: $(IMGUI_DIR)/%.cpp
	g++ $(CFLAGS) -c $< -o $@ $(INCLUDE_DIRS)

pre: 
	g++ $(CFLAGS) -E $(SRCS) $(HEADERS) $(IMGUI_SRCS)
