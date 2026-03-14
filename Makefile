# Makefile for MSYS2 MinGW64
# Usage: make
# Requires: pacman -S mingw-w64-x86_64-freeglut

CXX      = g++
CXXFLAGS = -std=c++11 -Wall -I/mingw64/include
LDFLAGS  = -L/mingw64/lib -lfreeglut -lglu32 -lopengl32

TARGET  = opengl_coloring
SOURCES = main.cpp

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET).exe $(LDFLAGS)

clean:
	rm -f $(TARGET).exe $(TARGET)
