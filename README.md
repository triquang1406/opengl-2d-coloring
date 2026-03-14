# OpenGL 2D Object Coloring Lab

Lab-02: 2D Object Coloring with OpenGL - Boundary Fill Implementation

## Overview
This project implements a 2D object drawing and coloring application using OpenGL with the boundary fill algorithm.

## Features
- Draw 2D shapes (rectangle, circle, ellipse, polygon)
- Color shapes using boundary fill algorithm
- Mouse-based seed point selection
- Context menu for shape and color selection

## Requirements
- MSYS2 MinGW64
- OpenGL libraries (installed via MSYS2)
- GLFW and GLEW

## Build Instructions
```bash
cd opengl-2d-coloring
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

## Usage
- Left-click inside a shape to fill it
- Right-click to open context menu
- Select shapes and colors from menu