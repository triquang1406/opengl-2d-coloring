cmake_minimum_required(VERSION 3.10)
project(opengl_coloring CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Place the compiled binary in <build>/bin/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(OpenGL REQUIRED)

add_executable(opengl_coloring main.cpp)

if(WIN32)
    # MSYS2 MinGW64 paths
    target_include_directories(opengl_coloring PRIVATE /mingw64/include)
    target_link_directories(opengl_coloring PRIVATE /mingw64/lib)
    target_link_libraries(opengl_coloring
        ${OPENGL_LIBRARIES}
        freeglut
        glu32
        opengl32
    )
else()
    # Linux / macOS
    find_package(GLUT REQUIRED)
    target_link_libraries(opengl_coloring
        ${OPENGL_LIBRARIES}
        ${GLUT_LIBRARIES}
    )
endif()
