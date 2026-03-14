/*
 * Lab-02: 2D Object Coloring with OpenGL
 * Boundary Fill Algorithm Implementation
 *
 * Build (MSYS2 MinGW64):
 *   g++ -std=c++11 main.cpp -o opengl_coloring.exe -lfreeglut -lglu32 -lopengl32
 *
 * Usage:
 *   Right-click  -> context menu (Shape / Color)
 *   Left-click   -> boundary fill from seed point
 */

#include <GL/glut.h>
#include <cmath>
#include <cstdio>
#include <stack>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ============================================================
 * Window state (updated in Reshape callback)
 * ============================================================ */
int w = 800, h = 600;

/* ============================================================
 * RGBColor structure
 * ============================================================ */
struct RGBColor {
    unsigned char r, g, b;
    RGBColor() : r(0), g(0), b(0) {}
    RGBColor(unsigned char r, unsigned char g, unsigned char b)
        : r(r), g(g), b(b) {}
};

/* ============================================================
 * Application state
 * ============================================================ */
RGBColor fillColor(255, 0, 0);      /* currently selected fill color (red)  */
RGBColor boundaryColor(0, 0, 0);    /* boundary (outline) color – black      */

/*
 * currentShape:
 *   0 = none, 1 = rectangle, 2 = square,   3 = circle,
 *   4 = ellipse, 5 = triangle, 6 = pentagon, 7 = hexagon, 8 = octagon
 */
int currentShape = 0;

/* ============================================================
 * Core pixel functions
 * ============================================================ */

/*
 * GetPixel – read the color of the pixel at screen position (x, y).
 * Coordinate system: (0,0) is top-left; y increases downward.
 * OpenGL framebuffer has (0,0) at bottom-left, so we flip y.
 */
RGBColor GetPixel(int x, int y)
{
    unsigned char pixels[3] = {0};
    glReadPixels(x, h - 1 - y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    return RGBColor(pixels[0], pixels[1], pixels[2]);
}

/*
 * PutPixel – write one pixel at screen position (x, y) using
 * glRasterPos2i and glDrawPixels as required by the assignment.
 *
 * With gluOrtho2D(0, w, h, 0):
 *   raster window-y = h - y_world
 * To align with glReadPixels(x, h-1-y, ...):
 *   y_world = y + 1  =>  raster window-y = h - (y+1) = h-1-y  ✓
 */
void PutPixel(int x, int y, RGBColor color)
{
    unsigned char pixel[3] = {color.r, color.g, color.b};
    glRasterPos2i(x, y + 1);
    glDrawPixels(1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
}

/* IsSameColor – compare two RGBColor values for equality */
bool IsSameColor(RGBColor c1, RGBColor c2)
{
    return c1.r == c2.r && c1.g == c2.g && c1.b == c2.b;
}

/* ============================================================
 * Boundary Fill Algorithm
 * ============================================================ */

/*
 * SetRasterBottomLeft – position the OpenGL raster at window pixel (0,0)
 * (bottom-left corner) so that glDrawPixels covers the entire window.
 *
 * We temporarily switch to a standard ortho (y up) to set the raster at
 * the mathematical origin (0,0) = bottom-left, then restore the original
 * matrices.  This avoids the need for the glWindowPos2i extension.
 */
static void SetRasterBottomLeft()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, w, 0, h, -1, 1); /* y increases upward */
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glRasterPos2i(0, 0); /* (0,0) in standard ortho = bottom-left corner */

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

/*
 * BoundaryFill – iterative 4-connected flood fill.
 *
 * Algorithm:
 *   1. Read the entire framebuffer into a RAM buffer (single glReadPixels).
 *   2. Perform the fill entirely in RAM (fast, no GPU round-trips per pixel).
 *   3. Write the modified buffer back to the framebuffer (single glDrawPixels).
 *
 * This avoids per-pixel glReadPixels calls (extremely slow) and also avoids
 * call-stack overflow caused by the naive recursive version.
 *
 * Parameters:
 *   x, y    – seed point (screen coordinates, (0,0) = top-left)
 *   F_Color – fill color
 *   B_Color – boundary color (stop condition)
 */
void BoundaryFill(int x, int y, RGBColor F_Color, RGBColor B_Color)
{
    if (x < 0 || x >= w || y < 0 || y >= h) return;

    /* Read entire framebuffer.
     * glReadPixels stores rows bottom-to-top:
     *   buffer[(h-1-py)*w + px] == screen pixel (px, py from top) */
    std::vector<unsigned char> buffer(w * h * 3);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());

    /* Helper: buffer index for screen pixel (px, py from top) */
    auto bufIdx = [&](int px, int py) -> int {
        return ((h - 1 - py) * w + px) * 3;
    };

    auto getBuf = [&](int px, int py) -> RGBColor {
        int i = bufIdx(px, py);
        return RGBColor(buffer[i], buffer[i + 1], buffer[i + 2]);
    };

    auto putBuf = [&](int px, int py, RGBColor c) {
        int i = bufIdx(px, py);
        buffer[i]     = c.r;
        buffer[i + 1] = c.g;
        buffer[i + 2] = c.b;
    };

    /* Do not fill if seed is already on boundary or already filled */
    RGBColor startColor = getBuf(x, y);
    if (IsSameColor(startColor, B_Color) || IsSameColor(startColor, F_Color))
        return;

    /* Iterative 4-connected flood fill using an explicit stack */
    struct Point { int x, y; };
    std::stack<Point> stk;
    stk.push({x, y});

    while (!stk.empty()) {
        Point p = stk.top();
        stk.pop();

        if (p.x < 0 || p.x >= w || p.y < 0 || p.y >= h) continue;

        RGBColor cur = getBuf(p.x, p.y);
        if (IsSameColor(cur, B_Color) || IsSameColor(cur, F_Color)) continue;

        putBuf(p.x, p.y, F_Color);

        stk.push({p.x + 1, p.y});
        stk.push({p.x - 1, p.y});
        stk.push({p.x,     p.y + 1});
        stk.push({p.x,     p.y - 1});
    }

    /* Write the modified buffer back to the framebuffer */
    SetRasterBottomLeft();
    glDrawPixels(w, h, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
    glFlush();
}

/* Convenience wrappers that start a fill one step in each direction */
void FillLeft(int x, int y, RGBColor F_Color, RGBColor B_Color)
{
    if (x > 0) BoundaryFill(x - 1, y, F_Color, B_Color);
}

void FillRight(int x, int y, RGBColor F_Color, RGBColor B_Color)
{
    if (x < w - 1) BoundaryFill(x + 1, y, F_Color, B_Color);
}

void FillTop(int x, int y, RGBColor F_Color, RGBColor B_Color)
{
    if (y > 0) BoundaryFill(x, y - 1, F_Color, B_Color);
}

void FillBottom(int x, int y, RGBColor F_Color, RGBColor B_Color)
{
    if (y < h - 1) BoundaryFill(x, y + 1, F_Color, B_Color);
}

/* ============================================================
 * Shape Drawing Functions
 * ============================================================ */

void DrawRectangle(int x1, int y1, int x2, int y2)
{
    glColor3ub(boundaryColor.r, boundaryColor.g, boundaryColor.b);
    glBegin(GL_LINE_LOOP);
        glVertex2i(x1, y1);
        glVertex2i(x2, y1);
        glVertex2i(x2, y2);
        glVertex2i(x1, y2);
    glEnd();
}

void DrawSquare(int cx, int cy, int size)
{
    int lo = size / 2;
    int hi = (size + 1) / 2; /* handles both even and odd sizes */
    DrawRectangle(cx - lo, cy - lo, cx + hi, cy + hi);
}

void DrawCircle(int cx, int cy, int r)
{
    /* Add a small margin so adjacent integer steps never leave a gap
     * even when cos/sin rounding reduces the effective circumference. */
    int steps = (int)(2.0 * M_PI * r) + 4;
    glColor3ub(boundaryColor.r, boundaryColor.g, boundaryColor.b);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < steps; i++) {
        double angle = 2.0 * M_PI * i / steps;
        glVertex2f((float)(cx + r * cos(angle)),
                   (float)(cy + r * sin(angle)));
    }
    glEnd();
}

void DrawEllipse(int cx, int cy, int rx, int ry)
{
    /* Average radius approximates circumference; +4 margin guards against gaps. */
    int steps = (int)(2.0 * M_PI * ((rx + ry) / 2.0)) + 4;
    glColor3ub(boundaryColor.r, boundaryColor.g, boundaryColor.b);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < steps; i++) {
        double angle = 2.0 * M_PI * i / steps;
        glVertex2f((float)(cx + rx * cos(angle)),
                   (float)(cy + ry * sin(angle)));
    }
    glEnd();
}

/*
 * DrawPolygon – draw a regular n-sided polygon centred at (cx, cy)
 * with circumradius r.  The first vertex is placed at the top (-π/2).
 */
void DrawPolygon(int cx, int cy, int r, int n)
{
    glColor3ub(boundaryColor.r, boundaryColor.g, boundaryColor.b);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < n; i++) {
        double angle = (2.0 * M_PI * i / n) - M_PI / 2.0;
        glVertex2f((float)(cx + r * cos(angle)),
                   (float)(cy + r * sin(angle)));
    }
    glEnd();
}

/* ============================================================
 * GLUT Callbacks
 * ============================================================ */

void Display()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); /* white background */
    glClear(GL_COLOR_BUFFER_BIT);

    if (currentShape > 0) {
        int cx = w / 2, cy = h / 2;
        switch (currentShape) {
            case 1: DrawRectangle(cx - 150, cy - 100, cx + 150, cy + 100); break;
            case 2: DrawSquare(cx, cy, 200);                                break;
            case 3: DrawCircle(cx, cy, 120);                                break;
            case 4: DrawEllipse(cx, cy, 180, 100);                          break;
            case 5: DrawPolygon(cx, cy, 130, 3); break; /* triangle  */
            case 6: DrawPolygon(cx, cy, 130, 5); break; /* pentagon  */
            case 7: DrawPolygon(cx, cy, 130, 6); break; /* hexagon   */
            case 8: DrawPolygon(cx, cy, 130, 8); break; /* octagon   */
        }
    }

    glFlush();
}

void Reshape(int width, int height)
{
    w = width;
    h = height;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    /* (0,0) = top-left, (w,h) = bottom-right – matches GLUT mouse coords */
    gluOrtho2D(0.0, w, h, 0.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glutPostRedisplay();
}

/*
 * XuLyMouse – GLUT mouse callback.
 * Left-click  triggers boundary fill at the click position.
 * Right-click is handled by the GLUT context menu (glutAttachMenu).
 */
void XuLyMouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (currentShape > 0) {
            /* x, y are already in our coordinate system (top-left origin) */
            BoundaryFill(x, y, fillColor, boundaryColor);
        }
    }
}

/* ============================================================
 * Context Menu
 * ============================================================ */

void ColorMenu(int id)
{
    switch (id) {
        case 1: fillColor = RGBColor(255,   0,   0); break; /* Red     */
        case 2: fillColor = RGBColor(  0, 255,   0); break; /* Green   */
        case 3: fillColor = RGBColor(  0,   0, 255); break; /* Blue    */
        case 4: fillColor = RGBColor(255, 255,   0); break; /* Yellow  */
        case 5: fillColor = RGBColor(  0, 255, 255); break; /* Cyan    */
        case 6: fillColor = RGBColor(255,   0, 255); break; /* Magenta */
        case 7: fillColor = RGBColor(255, 128,   0); break; /* Orange  */
        case 8: fillColor = RGBColor(128,   0, 128); break; /* Purple  */
    }
}

void PolygonMenu(int id)
{
    switch (id) {
        case 1: currentShape = 5; break; /* triangle */
        case 2: currentShape = 6; break; /* pentagon */
        case 3: currentShape = 7; break; /* hexagon  */
        case 4: currentShape = 8; break; /* octagon  */
    }
    glutPostRedisplay();
}

void ShapeMenu(int id)
{
    switch (id) {
        case 1: currentShape = 1; break; /* rectangle */
        case 2: currentShape = 2; break; /* square    */
        case 3: currentShape = 3; break; /* circle    */
        case 4: currentShape = 4; break; /* ellipse   */
    }
    glutPostRedisplay();
}

void MainMenu(int id)
{
    if (id == 0) exit(0);
}

void CreateContextMenu()
{
    /* Color submenu */
    int colorMenuId = glutCreateMenu(ColorMenu);
    glutAddMenuEntry("Red",     1);
    glutAddMenuEntry("Green",   2);
    glutAddMenuEntry("Blue",    3);
    glutAddMenuEntry("Yellow",  4);
    glutAddMenuEntry("Cyan",    5);
    glutAddMenuEntry("Magenta", 6);
    glutAddMenuEntry("Orange",  7);
    glutAddMenuEntry("Purple",  8);

    /* Polygon type submenu */
    int polygonMenuId = glutCreateMenu(PolygonMenu);
    glutAddMenuEntry("Triangle", 1);
    glutAddMenuEntry("Pentagon", 2);
    glutAddMenuEntry("Hexagon",  3);
    glutAddMenuEntry("Octagon",  4);

    /* Shape submenu */
    int shapeMenuId = glutCreateMenu(ShapeMenu);
    glutAddMenuEntry("Rectangle", 1);
    glutAddMenuEntry("Square",    2);
    glutAddMenuEntry("Circle",    3);
    glutAddMenuEntry("Ellipse",   4);
    glutAddSubMenu("Polygon",     polygonMenuId);

    /* Main (root) menu */
    glutCreateMenu(MainMenu);
    glutAddSubMenu("Shape", shapeMenuId);
    glutAddSubMenu("Color", colorMenuId);
    glutAddMenuEntry("Exit", 0);

    /* Attach to right mouse button */
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

/* ============================================================
 * Entry Point
 * ============================================================ */

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(w, h);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Lab-02: 2D Object Coloring - Boundary Fill");

    /* Disable dithering so stored pixel colors exactly match what we set –
     * critical for the IsSameColor comparisons in BoundaryFill.          */
    glDisable(GL_DITHER);
    /* Disable anti-aliasing on lines to ensure pixel-perfect boundaries. */
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);

    /* Set up 2D projection:
     *   top-left  = (0, 0)
     *   bot-right = (w, h)
     * This matches GLUT mouse coordinates so no conversion is needed.   */
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, w, h, 0.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Register GLUT callbacks */
    glutDisplayFunc(Display);
    glutReshapeFunc(Reshape);
    glutMouseFunc(XuLyMouse);

    /* Build and attach the right-click context menu */
    CreateContextMenu();

    printf("=== Lab-02: 2D Object Coloring – Boundary Fill ===\n");
    printf("Right-click : open context menu\n");
    printf("  Shape     > Rectangle / Square / Circle / Ellipse / Polygon\n");
    printf("  Color     > Red / Green / Blue / Yellow / Cyan / Magenta / Orange / Purple\n");
    printf("Left-click  : fill the shape at the clicked seed point\n");
    printf("===================================================\n");

    glutMainLoop();
    return 0;
}
