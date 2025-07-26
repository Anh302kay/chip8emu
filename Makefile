CC=g++
CFLAGS = -Wall -c -std=c++20 -g
INCLUDES = include
SOURCES  = source 
BUILD = build
LIBS = -lmingw32 -lgdi32 -lopengl32 -limm32 -lSDL3 

vpath %.cpp $(SOURCES)

SRC = $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
OBJ = $(SRC:%.cpp=$(BUILD)/%.o)
TARGET = bin/debug/$(notdir $(CURDIR))

#Mac OS bit not tested
UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
	CC = clang++

	INCFLAGS += -I$(PATH_SDL)/include
	LIBS += $(shell $(BIN)/sdl/sdl2-config --prefix=$(BIN) --static-libs) -L/usr/local/lib
endif

all: $(TARGET).exe

$(TARGET).exe : $(OBJ)
	$(CC) $(OBJ) -o $(TARGET).exe $(LIBS)

$(BUILD)/%.o : %.cpp
	$(CC) $(CFLAGS) $(LIBS) -I$(INCLUDES) $< -o $@

clean:
	rm build/*.o $(TARGET).exe

#not working but put here as example
web:
	em++ -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sUSE_SDL_TTF=2 -sUSE_SDL_MIXER=2 -sMAX_WEBGL_VERSION=2 -sMIN_WEBGL_VERSION=2 -s SDL2_IMAGE_FORMATS='["png"]' -std=c++20 -Iinclude -Ires source/main.cpp source/window_draw.cpp source/window_events.cpp source/renderwindow.cpp source/entity.cpp --preload-file .\res -o b.html --use-preload-plugins -mnontrapping-fptoint
