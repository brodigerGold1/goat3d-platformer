#include <iostream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <GL/glew.h>
#include <GL/glut.h>

using namespace std;

struct Platform {
    float x, y, w, h;
    float r, g, b;
    const char* name;
};

struct Collectible {
    float x, y, w, h;
    float r, g, b;
    const char* name;
    bool collected = false;
};

struct Light {
    float x, y, w, h;
    float r, g, b;
    const char* name;
    float pulse = 0;
};

vector<Platform> platforms = {
    // LEVEL 2 GROUND - Gap sections
    {-10, -5, 5, 1,    0.2f, 0.4f, 0.2f,  "L2_GROUND_A"},
    {-2, -5, 3, 1,     0.2f, 0.4f, 0.2f,  "L2_GROUND_B"},
    {4, -5, 6, 1,      0.2f, 0.4f, 0.2f,  "L2_GROUND_C"},

    // LEVEL 2 - Platform path
    {-8, -2.5, 2.5, 0.3,  0.5f, 0.3f, 0.7f,  "L2_P1"},
    {-4, -1.5, 2.5, 0.3,  0.5f, 0.3f, 0.7f,  "L2_P2"},
    {0, -0.5, 2.5, 0.3,   0.5f, 0.3f, 0.7f,  "L2_P3"},
    {-7, 1, 0.5, 0.3,     0.5f, 0.3f, 0.7f,  "L2_P4"},
    {-2, 1.5, 1.5, 0.3,     0.5f, 0.3f, 0.7f,  "L2_P5"},
    {-9, 3, 0.5, 0.3,        0.5f, 0.3f, 0.7f,  "L2_P6"},
    {6, 2, 5, 0.3,        0.5f, 0.3f, 0.7f,  "L2_P7"},
    {6, 0, 3, 0.3,        0.5f, 0.3f, 0.7f,  "L2_P8"},

    // WALLS
    {-10, -5, 0.5, 10,    0.5f, 0.3f, 0.2f,  "LEFT_WALL"},
    {10, -5, 0.5, 10,     0.5f, 0.3f, 0.2f,  "RIGHT_WALL"},
};

vector<Collectible> coins = {
    {-9 + 0.1f, 3 + 0.3 + 0.05f, 0.3f, 0.3f, 1.0f, 1.0f, 0.0f, "L2_COIN"},   // On L2_P6
};

vector<Light> lights = {
    {6 + 3 - 0.3f, 2 + 0.3 + 0.05f, 0.4f, 0.6f, 1.0f, 1.0f, 0.5f, "L2_EXIT"},   // Top right of L2_P7
};

void setupProjection() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-10, 10, -5, 5);
    glMatrixMode(GL_MODELVIEW);
}

GLuint checkerTexture = 0;
GLuint coinTexture = 0;

void createCheckerTexture() {
    const int SIZE = 64;
    GLubyte image[SIZE][SIZE][3];
    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++) {
            int checker = ((x / 8) + (y / 8)) % 2;
            if (checker == 0) { image[y][x][0] = 100; image[y][x][1] = 100; image[y][x][2] = 100; }
            else { image[y][x][0] = 50; image[y][x][1] = 50; image[y][x][2] = 50; }
        }
    }
    glGenTextures(1, &checkerTexture);
    glBindTexture(GL_TEXTURE_2D, checkerTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SIZE, SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
}

void createCoinTexture() {
    const int SIZE = 32;
    GLubyte image[SIZE][SIZE][3];
    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++) {
            float dx = (x - SIZE/2) / float(SIZE/2);
            float dy = (y - SIZE/2) / float(SIZE/2);
            float dist = sqrt(dx*dx + dy*dy);
            if (dist < 0.9f) { image[y][x][0] = 255; image[y][x][1] = 215; image[y][x][2] = 0; }
            else { image[y][x][0] = 0; image[y][x][1] = 0; image[y][x][2] = 0; }
        }
    }
    glGenTextures(1, &coinTexture);
    glBindTexture(GL_TEXTURE_2D, coinTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SIZE, SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
}

void drawText(float x, float y, const string& text, float r=1, float g=1, float b=1) {
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    for (char c : text) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
}

void drawRect(float x, float y, float w, float h, float r, float g, float b) {
    glDisable(GL_TEXTURE_2D);
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x+w, y);
        glVertex2f(x+w, y+h);
        glVertex2f(x, y+h);
    glEnd();
}

void drawRectTex(float x, float y, float w, float h, GLuint tex) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    glColor3f(1,1,1);
    glBegin(GL_QUADS);
        glTexCoord2f(0,0); glVertex2f(x, y);
        glTexCoord2f(1,0); glVertex2f(x+w, y);
        glTexCoord2f(1,1); glVertex2f(x+w, y+h);
        glTexCoord2f(0,1); glVertex2f(x, y+h);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void drawGrass(float x, float y, float w) {
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.2f, 0.7f, 0.2f);
    for (float gx = x + 0.1f; gx < x + w - 0.1f; gx += 0.25f) {
        float h = 0.15f + (rand() % 10) * 0.01f;
        glBegin(GL_TRIANGLES);
            glVertex2f(gx, y);
            glVertex2f(gx + 0.08f, y + h);
            glVertex2f(gx + 0.16f, y);
        glEnd();
    }
}

void drawLight(float x, float y, float w, float h, float r, float g, float b, float pulse) {
    glDisable(GL_TEXTURE_2D);
    glColor4f(r, g, b, 0.3f + 0.2f * sin(pulse));
    glBegin(GL_QUADS);
        glVertex2f(x - 0.2f, y);
        glVertex2f(x + w + 0.2f, y);
        glVertex2f(x + w + 0.2f, y + h + 0.2f);
        glVertex2f(x - 0.2f, y + h + 0.2f);
    glEnd();
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x+w, y);
        glVertex2f(x+w, y+h);
        glVertex2f(x, y+h);
    glEnd();
    glColor3f(1, 1, 0.8f);
    glBegin(GL_QUADS);
        glVertex2f(x + 0.05f, y + 0.05f);
        glVertex2f(x+w - 0.05f, y + 0.05f);
        glVertex2f(x+w - 0.05f, y+h - 0.05f);
        glVertex2f(x + 0.05f, y+h - 0.05f);
    glEnd();
}

float frameTime = 0;

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    
    frameTime += 0.016f;
    
    // Background
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, checkerTexture);
    glColor3f(1,1,1);
    glBegin(GL_QUADS);
        glTexCoord2f(0,0); glVertex2f(-10, -5);
        glTexCoord2f(1,0); glVertex2f(10, -5);
        glTexCoord2f(1,1); glVertex2f(10, 5);
        glTexCoord2f(0,1); glVertex2f(-10, 5);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    
    // Platforms
    for (auto& p : platforms) {
        drawRect(p.x, p.y, p.w, p.h, p.r, p.g, p.b);
        if (strstr(p.name, "GROUND") || strstr(p.name, "L2_GROUND")) {
            drawGrass(p.x, p.y + p.h, p.w);
        }
        drawText(p.x, p.y + p.h + 0.1, p.name, 1, 1, 1);
        drawText(p.x, p.y + p.h + 0.25, "(" + to_string((int)p.x) + "," + to_string((int)p.y) + ") " + to_string((int)p.w) + "x" + to_string((int)(p.h*10)/10.0f), 0.7, 0.7, 0.7);
    }
    
    // Coins
    for (auto& c : coins) {
        if (!c.collected) {
            drawRectTex(c.x, c.y, c.w, c.h, coinTexture);
            drawText(c.x, c.y + c.h + 0.1, c.name, 1, 1, 0);
        }
    }
    
    // Exit lights
    for (auto& l : lights) {
        l.pulse = frameTime * 3;
        drawLight(l.x, l.y, l.w, l.h, l.r, l.g, l.b, l.pulse);
        drawText(l.x - 0.5f, l.y + l.h + 0.1, l.name, 1, 1, 0.5f);
    }
    
    // Legend
    drawText(-9.5, 4.5, "LEVEL 2 - YELLOW=Coin  BRIGHT_YELLOW=Exit Light  DARK_GREEN=Ground  PURPLE=Platforms", 1, 1, 0);
    drawText(-9.5, -4.8, "ESC to exit", 0.5, 0.5, 0.5);
    
    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 27) exit(0);
}

void init() {
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    srand(12345);
    createCheckerTexture();
    createCoinTexture();
    setupProjection();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1024, 768);
    glutInitWindowPosition(100, 50);
    glutCreateWindow("CS0045 Final - Level 2");
    
    GLenum err = glewInit();
    if (err != GLEW_OK) return -1;
    
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(display);
    glutMainLoop();
    return 0;
}