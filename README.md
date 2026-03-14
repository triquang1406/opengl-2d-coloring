# OpenGL 2D Object Coloring Lab

Lab-02: 2D Object Coloring with OpenGL – Boundary Fill Implementation

## Overview

A complete OpenGL application that draws 2D shapes and fills them using the
**boundary fill algorithm**.  The application is single-file (`main.cpp`) and
uses **freeglut** for window management and context menus.

## Features

| Feature | Details |
|---------|---------|
| Shapes | Rectangle, Square, Circle, Ellipse, Triangle, Pentagon, Hexagon, Octagon |
| Fill algorithm | Iterative 4-connected boundary fill (no stack overflow) |
| Colors | Red, Green, Blue, Yellow, Cyan, Magenta, Orange, Purple |
| Context menu | Right-click → Shape / Color submenus |
| Mouse input | Left-click = seed point for fill |

## Key Functions

| Function | Description |
|----------|-------------|
| `GetPixel(x, y)` | Read pixel color with `glReadPixels` |
| `PutPixel(x, y, color)` | Write pixel with `glRasterPos2i` + `glDrawPixels` |
| `IsSameColor(c1, c2)` | Color equality helper |
| `BoundaryFill(x, y, F, B)` | Main fill algorithm (iterative, batch I/O) |
| `FillLeft/Right/Top/Bottom` | Directional fill helpers |
| `XuLyMouse(btn, state, x, y)` | GLUT mouse event handler |

## Requirements

- **MSYS2 MinGW64** – https://www.msys2.org/
- **freeglut** (OpenGL + GLUT context menus)

Install dependencies in MSYS2:
```bash
pacman -S mingw-w64-x86_64-freeglut
```

## Build

### Option A – CMake (recommended)
```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
./bin/opengl_coloring.exe
```

### Option B – Direct g++ (Makefile)
```bash
make
./opengl_coloring.exe
```

### Option C – Single command
```bash
g++ -std=c++11 main.cpp -o opengl_coloring.exe -lfreeglut -lglu32 -lopengl32
./opengl_coloring.exe
```

## Usage

1. **Right-click** anywhere to open the context menu.
2. Choose **Shape → \<shape name\>** – the shape is drawn in the centre of the window.
3. Choose **Color → \<color name\>** – sets the fill colour.
4. **Left-click inside** the shape – the boundary fill algorithm fills the shape.
5. Repeat steps 2–4 to try different shapes and colours.

> **Note:** Filling a large shape may take a moment; use reasonably sized
> shapes (the defaults are already sized appropriately).  Resizing the window
> clears the canvas and redraws the current shape without fills.

## Coordinate System

```
(0,0) ─────────────────── (w,0)
  │                          │
  │   gluOrtho2D(0,w,h,0)   │
  │   y increases downward   │
  │                          │
(0,h) ─────────────────── (w,h)
```

GLUT mouse coordinates match this system directly – no y-inversion is needed
when passing the click position to `BoundaryFill`.