PROJECTNAME = project
OUTPUT_DIR = build

CFLAGS= -Wno-narrowing 

INCLUDE_DIRS = -Iinclude/SDL2 -Iinclude/imgui
LIB_DIRS = -Llib

LIBS = -lmingw32 -lSDL2main -lSDL2

SRC = $(wildcard src/*.cpp) $(wildcard src/*.h) $(wildcard imgui/*.cpp)

default: 
	g++ $(CFLAGS) $(SRC) -o $(OUTPUT_DIR)/$(PROJECTNAME) $(INCLUDE_DIRS) $(LIB_DIRS) $(LIBS)

pre: 
	g++ $(CFLAGS) -E $(SRC)