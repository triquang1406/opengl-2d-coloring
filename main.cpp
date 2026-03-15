/*
 * Lab-02: 2D Object Coloring with OpenGL
 * Boundary Fill Algorithm Implementation
 */

#include <GL/glut.h>
#include <cmath>
#include <cstdio>
#include <stack>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int w = 800, h = 600;

struct RGBColor {
    unsigned char r, g, b;
    RGBColor() : r(0), g(0), b(0) {}
    RGBColor(unsigned char r, unsigned char g, unsigned char b)
        : r(r), g(g), b(b) {}
};

RGBColor fillColor(0, 0, 255);      /* Blue */
RGBColor boundaryColor(0, 0, 0);    /* Black boundary */

int currentShape = 0;

/* ============================================================
 * Core pixel functions
 * ============================================================ */

RGBColor GetPixel(int x, int y)
{
    unsigned char pixels[3] = {0};
    glReadPixels(x, h - 1 - y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    return RGBColor(pixels[0], pixels[1], pixels[2]);
}

/*
 * PutPixel - write one pixel at screen position (x, y) using
 * glRasterPos2i and glDrawPixels as required by the assignment.
 */
void PutPixel(int x, int y, RGBColor color)
{
    unsigned char pixel[3] = {color.r, color.g, color.b};
    glRasterPos2i(x, y + 1);
    glDrawPixels(1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
    glFlush();
}

bool IsSameColor(RGBColor c1, RGBColor c2)
{
    return c1.r == c2.r && c1.g == c2.g && c1.b == c2.b;
}

static void SetRasterBottomLeft()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, w, 0, h, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glRasterPos2i(0, 0);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void BoundaryFill(int x, int y, RGBColor F_Color, RGBColor B_Color)
{
    if (x < 0 || x >= w || y < 0 || y >= h) return;

    printf("BoundaryFill called at (%d, %d)\n", x, y);

    std::vector<unsigned char> buffer(w * h * 3);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());

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

    RGBColor startColor = getBuf(x, y);
    printf("Start color: (%d, %d, %d)\n", startColor.r, startColor.g, startColor.b);

    if (IsSameColor(startColor, B_Color) || IsSameColor(startColor, F_Color)) {
        printf("Start is boundary or already filled\n");
        return;
    }

    struct Point { int x, y; };
    std::stack<Point> stk;
    stk.push({x, y});

    int fillCount = 0;

    while (!stk.empty()) {
        Point p = stk.top();
        stk.pop();

        if (p.x < 0 || p.x >= w || p.y < 0 || p.y >= h) continue;

        RGBColor cur = getBuf(p.x, p.y);
        if (IsSameColor(cur, B_Color) || IsSameColor(cur, F_Color)) continue;

        putBuf(p.x, p.y, F_Color);
        fillCount++;

        stk.push({p.x + 1, p.y});
        stk.push({p.x - 1, p.y});
        stk.push({p.x,     p.y + 1});
        stk.push({p.x,     p.y - 1});
    }

    printf("Filled %d pixels\n", fillCount);

    SetRasterBottomLeft();
    glDrawPixels(w, h, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
    glFlush();
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
    glFlush();
}

void DrawSquare(int cx, int cy, int size)
{
    int lo = size / 2;
    int hi = (size + 1) / 2;
    DrawRectangle(cx - lo, cy - lo, cx + hi, cy + hi);
}

void DrawCircle(int cx, int cy, int r)
{
    int steps = (int)(2.0 * M_PI * r) + 4;
    glColor3ub(boundaryColor.r, boundaryColor.g, boundaryColor.b);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < steps; i++) {
        double angle = 2.0 * M_PI * i / steps;
        glVertex2f((float)(cx + r * cos(angle)), (float)(cy + r * sin(angle)));
    }
    glEnd();
    glFlush();
}

void DrawEllipse(int cx, int cy, int rx, int ry)
{
    int steps = (int)(2.0 * M_PI * ((rx + ry) / 2.0)) + 4;
    glColor3ub(boundaryColor.r, boundaryColor.g, boundaryColor.b);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < steps; i++) {
        double angle = 2.0 * M_PI * i / steps;
        glVertex2f((float)(cx + rx * cos(angle)), (float)(cy + ry * sin(angle)));
    }
    glEnd();
    glFlush();
}

/*
 * DrawEquilateralTriangle - All three sides equal, pointing upward.
 * Uses circumradius r; vertices placed at -90, 30, and 150 degrees.
 */
void DrawEquilateralTriangle(int cx, int cy, int r)
{
    glColor3ub(boundaryColor.r, boundaryColor.g, boundaryColor.b);
    glBegin(GL_LINE_LOOP);
        glVertex2f((float)(cx + r * cos(-M_PI / 2.0)),
                   (float)(cy + r * sin(-M_PI / 2.0)));
        glVertex2f((float)(cx + r * cos(-M_PI / 2.0 + 2.0 * M_PI / 3.0)),
                   (float)(cy + r * sin(-M_PI / 2.0 + 2.0 * M_PI / 3.0)));
        glVertex2f((float)(cx + r * cos(-M_PI / 2.0 + 4.0 * M_PI / 3.0)),
                   (float)(cy + r * sin(-M_PI / 2.0 + 4.0 * M_PI / 3.0)));
    glEnd();
    glFlush();
}

/*
 * DrawIsoscelesRightTriangle - Right angle at top, equal legs going
 * down-left and down-right (45-45-90 triangle).
 */
void DrawIsoscelesRightTriangle(int cx, int cy, int r)
{
    glColor3ub(boundaryColor.r, boundaryColor.g, boundaryColor.b);
    glBegin(GL_LINE_LOOP);
        glVertex2i(cx,     cy - r);   /* Top vertex (right angle) */
        glVertex2i(cx + r, cy + r);   /* Bottom-right */
        glVertex2i(cx - r, cy + r);   /* Bottom-left */
    glEnd();
    glFlush();
}

void DrawPolygon(int cx, int cy, int r, int n)
{
    glColor3ub(boundaryColor.r, boundaryColor.g, boundaryColor.b);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < n; i++) {
        double angle = (2.0 * M_PI * i / n) - M_PI / 2.0;
        glVertex2f((float)(cx + r * cos(angle)), (float)(cy + r * sin(angle)));
    }
    glEnd();
    glFlush();
}

void DrawArrow(int cx, int cy)
{
    glColor3ub(boundaryColor.r, boundaryColor.g, boundaryColor.b);
    glBegin(GL_LINE_LOOP);
        glVertex2i(cx,      cy - 80);
        glVertex2i(cx + 40, cy - 20);
        glVertex2i(cx + 20, cy - 20);
        glVertex2i(cx + 20, cy + 80);
        glVertex2i(cx - 20, cy + 80);
        glVertex2i(cx - 20, cy - 20);
        glVertex2i(cx - 40, cy - 20);
    glEnd();
    glFlush();
}

void DrawStar(int cx, int cy, int r)
{
    glColor3ub(boundaryColor.r, boundaryColor.g, boundaryColor.b);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 10; i++) {
        double angle = (2.0 * M_PI * i / 10) - M_PI / 2.0;
        int radius = (i % 2 == 0) ? r : r / 2;
        glVertex2f((float)(cx + radius * cos(angle)),
                   (float)(cy + radius * sin(angle)));
    }
    glEnd();
    glFlush();
}

void DrawPlus(int cx, int cy, int size)
{
    glColor3ub(boundaryColor.r, boundaryColor.g, boundaryColor.b);
    glLineWidth(3.0);
    glBegin(GL_LINES);
        glVertex2i(cx,        cy - size);
        glVertex2i(cx,        cy + size);
        glVertex2i(cx - size, cy);
        glVertex2i(cx + size, cy);
    glEnd();
    glLineWidth(1.0);
    glFlush();
}

void DrawMinus(int cx, int cy, int size)
{
    glColor3ub(boundaryColor.r, boundaryColor.g, boundaryColor.b);
    glLineWidth(3.0);
    glBegin(GL_LINES);
        glVertex2i(cx - size, cy);
        glVertex2i(cx + size, cy);
    glEnd();
    glLineWidth(1.0);
    glFlush();
}

void DrawMultiply(int cx, int cy, int size)
{
    glColor3ub(boundaryColor.r, boundaryColor.g, boundaryColor.b);
    glLineWidth(3.0);
    glBegin(GL_LINES);
        glVertex2i(cx - size, cy - size);
        glVertex2i(cx + size, cy + size);
        glVertex2i(cx - size, cy + size);
        glVertex2i(cx + size, cy - size);
    glEnd();
    glLineWidth(1.0);
    glFlush();
}

void DrawDivide(int cx, int cy, int size)
{
    glColor3ub(boundaryColor.r, boundaryColor.g, boundaryColor.b);
    glLineWidth(3.0);
    glBegin(GL_LINES);
        glVertex2i(cx - size, cy + size);
        glVertex2i(cx + size, cy - size);
    glEnd();
    glLineWidth(1.0);

    glPointSize(8.0);
    glBegin(GL_POINTS);
        glVertex2i(cx - 15, cy + 25);
        glVertex2i(cx + 15, cy - 25);
    glEnd();
    glPointSize(1.0);
    glFlush();
}

/*
 * DrawRightTriangle - right angle at bottom-left corner,
 * with one vertical leg and one horizontal leg (non-isosceles).
 */
void DrawRightTriangle(int cx, int cy, int r)
{
    glColor3ub(boundaryColor.r, boundaryColor.g, boundaryColor.b);
    glBegin(GL_LINE_LOOP);
        glVertex2i(cx - r, cy - r);   /* Top-left (apex) */
        glVertex2i(cx - r, cy + r);   /* Bottom-left (right angle) */
        glVertex2i(cx + r, cy + r);   /* Bottom-right */
    glEnd();
    glFlush();
}

/* ============================================================
 * GLUT Callbacks
 * ============================================================ */

void Display()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (currentShape > 0) {
        int cx = w / 2, cy = h / 2;
        switch (currentShape) {
            case 1:  DrawRectangle(cx - 150, cy - 100, cx + 150, cy + 100); break;
            case 2:  DrawSquare(cx, cy, 200); break;
            case 3:  DrawCircle(cx, cy, 120); break;
            case 4:  DrawEllipse(cx, cy, 180, 100); break;
            case 5:  DrawIsoscelesRightTriangle(cx, cy, 130); break;
            case 6:  DrawEquilateralTriangle(cx, cy, 130); break;
            case 7:  DrawPolygon(cx, cy, 130, 5); break;
            case 8:  DrawPolygon(cx, cy, 130, 6); break;
            case 9:  DrawArrow(cx, cy); break;
            case 10: DrawStar(cx, cy, 100); break;
            case 11: DrawPlus(cx, cy, 80); break;
            case 12: DrawMinus(cx, cy, 80); break;
            case 13: DrawMultiply(cx, cy, 80); break;
            case 14: DrawDivide(cx, cy, 80); break;
            case 15: DrawRightTriangle(cx, cy, 130); break;
            case 16: DrawRectangle(cx - 100, cy - 80, cx + 100, cy + 80); break;
            case 99: DrawMinus(cx, cy, 80); break;
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
    gluOrtho2D(0.0, w, h, 0.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glutPostRedisplay();
}

/*
 * XuLyMouse - Left-click seeds the boundary fill.
 * The filled framebuffer is written back via glDrawPixels so the
 * color persists after the mouse button is released.
 */
void XuLyMouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        printf("Mouse clicked at (%d, %d)\n", x, y);
        if (currentShape > 0) {
            BoundaryFill(x, y, fillColor, boundaryColor);

            /* Read back and redraw so the fill is visible and persists
             * after the mouse button is released. */
            std::vector<unsigned char> buffer(w * h * 3);
            glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
            SetRasterBottomLeft();
            glDrawPixels(w, h, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
            glFlush();
        }
    }
}

/* ============================================================
 * Menu Callbacks
 * ============================================================ */

void ColorMenu(int id)
{
    switch (id) {
        case 1: fillColor = RGBColor(0,   0,   255); printf("Color: Blue\n");    break;
        case 2: fillColor = RGBColor(255, 0,   0);   printf("Color: Red\n");     break;
        case 3: fillColor = RGBColor(255, 255, 0);   printf("Color: Yellow\n");  break;
        case 4: fillColor = RGBColor(0,   255, 255); printf("Color: Cyan\n");    break;
        case 5: fillColor = RGBColor(255, 0,   255); printf("Color: Magenta\n"); break;
        case 6: fillColor = RGBColor(255, 165, 0);   printf("Color: Orange\n");  break;
        case 7: fillColor = RGBColor(128, 0,   128); printf("Color: Purple\n");  break;
        case 8: fillColor = RGBColor(0,   128, 0);   printf("Color: Green\n");   break;
    }
}

void ShapeMenu(int id)
{
    currentShape = id;
    printf("Shape: %d\n", currentShape);
    glutPostRedisplay();
}

void MainMenu(int id)
{
    if (id == 0) exit(0);
    else if (id == 99) {
        currentShape = 99;
        printf("Drawing Line\n");
        glutPostRedisplay();
    }
}

void CreateContextMenu()
{
    int tamGiacMenuId = glutCreateMenu(ShapeMenu);
    glutAddMenuEntry("Vuong can", 5);
    glutAddMenuEntry("Deu",       6);

    int tuGiacMenuId = glutCreateMenu(ShapeMenu);
    glutAddMenuEntry("Hinh chu nhat", 1);
    glutAddMenuEntry("Hinh vuong",    2);

    int ovalMenuId = glutCreateMenu(ShapeMenu);
    glutAddMenuEntry("Hinh tron", 3);
    glutAddMenuEntry("Elip",      4);

    int daGiacMenuId = glutCreateMenu(ShapeMenu);
    glutAddMenuEntry("Ngu giac deu", 7);
    glutAddMenuEntry("Luc giac deu", 8);

    int hinhKhacMenuId = glutCreateMenu(ShapeMenu);
    glutAddMenuEntry("Mui ten",   9);
    glutAddMenuEntry("Ngoi sao", 10);

    int dauMenuId = glutCreateMenu(ShapeMenu);
    glutAddMenuEntry("Cong",  11);
    glutAddMenuEntry("Tru",   12);
    glutAddMenuEntry("Nhan",  13);
    glutAddMenuEntry("Chia",  14);

    int toMauMenuId = glutCreateMenu(ColorMenu);
    glutAddMenuEntry("Xanh (Blue)",       1);
    glutAddMenuEntry("Do (Red)",          2);
    glutAddMenuEntry("Vang (Yellow)",     3);
    glutAddMenuEntry("Cyan",              4);
    glutAddMenuEntry("Magenta",           5);
    glutAddMenuEntry("Cam (Orange)",      6);
    glutAddMenuEntry("Tim (Purple)",      7);
    glutAddMenuEntry("Xanh cay (Green)", 8);

    int chonHinhMenuId = glutCreateMenu(ShapeMenu);
    glutAddMenuEntry("Tam giac vuong", 15);
    glutAddMenuEntry("Tao hinh",       16);

    glutCreateMenu(MainMenu);
    glutAddMenuEntry("Duong thang",         99);
    glutAddSubMenu("Tam giac",     tamGiacMenuId);
    glutAddSubMenu("Tu giac",      tuGiacMenuId);
    glutAddSubMenu("Oval",         ovalMenuId);
    glutAddSubMenu("Da giac deu",  daGiacMenuId);
    glutAddSubMenu("Hinh khac",    hinhKhacMenuId);
    glutAddSubMenu("Dau",          dauMenuId);
    glutAddSubMenu("To mau",       toMauMenuId);
    glutAddSubMenu("Chon hinh",    chonHinhMenuId);
    glutAddMenuEntry("Thoat",       0);

    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(w, h);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Lab-02: 2D Object Coloring - Boundary Fill");

    glDisable(GL_DITHER);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, w, h, 0.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glutDisplayFunc(Display);
    glutReshapeFunc(Reshape);
    glutMouseFunc(XuLyMouse);

    CreateContextMenu();

    printf("=== Lab-02: 2D Object Coloring - Boundary Fill ===\n");
    printf("Right-click : open context menu\n");
    printf("Left-click  : fill the shape at the clicked seed point\n");
    printf("===================================================\n");

    glutMainLoop();
    return 0;
}