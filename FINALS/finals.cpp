#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <GL/glew.h>
#include <GL/glut.h>
#include "winscreen.h"

using namespace std;

struct Platform {
    float x, y, w, h;
    float r, g, b;
};

struct Collectible {
    float x, y, w, h;
    float r, g, b;
    float bobOffset = 0;
};

struct Light {
    float x, y, w, h;
    float r, g, b;
    float pulse = 0;
};

int currentLevel = 1;

enum AppState { STATE_LEVEL, STATE_WIN };
AppState appState = STATE_LEVEL;

vector<Platform> level1Platforms = {
    {-10, -5, 20, 1,   0.3f, 0.5f, 0.3f},
    {-7, -2, 3, 0.3,   0.4f, 0.4f, 0.6f},
    {-2, -1, 3, 0.3,   0.4f, 0.6f, 0.4f},
    {3, 0, 1.5, 0.3,   0.6f, 0.4f, 0.4f},
    {-8, 1.5, 3, 0.3,  0.5f, 0.5f, 0.7f},
    {1, 2, 1, 0.3,     0.7f, 0.5f, 0.5f},
    {6, 3, 5, 0.3,     0.3f, 0.7f, 0.7f},
    {-10, -5, 0.5, 10, 0.5f, 0.3f, 0.2f},
    {10, -5, 0.5, 10,  0.5f, 0.3f, 0.2f},
};

vector<Platform> level2Platforms = {
    {-10, -5, 5, 1,    0.2f, 0.4f, 0.2f},
    {-2, -5, 3, 1,     0.2f, 0.4f, 0.2f},
    {4, -5, 6, 1,      0.2f, 0.4f, 0.2f},
    {-8, -2.5, 2.5, 0.3,  0.5f, 0.3f, 0.7f},
    {-4, -1.5, 2.5, 0.3,  0.5f, 0.3f, 0.7f},
    {0, -0.5, 2.5, 0.3,   0.5f, 0.3f, 0.7f},
    {-7, 1, 0.5, 0.3,     0.5f, 0.3f, 0.7f},
    {-2, 2, 2.5, 0.3,     0.5f, 0.3f, 0.7f},
    {-9, 3.5, 0.5, 0.3,   0.5f, 0.3f, 0.7f},
    {6, 2, 4, 0.3,        0.5f, 0.3f, 0.7f},
    {6, 0, 3, 0.3,        0.5f, 0.3f, 0.7f},
    {-10, -5, 0.5, 10,    0.5f, 0.3f, 0.2f},
    {10, -5, 0.5, 10,     0.5f, 0.3f, 0.2f},
};

vector<Collectible> level1Coins = {
    {-8 + 1.25f - 0.15f, 1.5 + 0.3 + 0.05f, 0.3f, 0.3f, 1.0f, 1.0f, 0.0f},
};

vector<Collectible> level2Coins = {
    {-9 + 0.1f, 3.5 + 0.3 + 0.05f, 0.3f, 0.3f, 1.0f, 1.0f, 0.0f},
};

vector<Light> level1Lights = {
    {6 + 5 - 0.3f, 3 + 0.3 + 0.05f, 0.4f, 0.6f, 1.0f, 1.0f, 0.5f},
};

vector<Light> level2Lights = {
    {9.2f, 2 + 0.3 + 0.05f, 0.4f, 0.6f, 1.0f, 1.0f, 0.5f},
};

vector<Platform>* platforms = &level1Platforms;
vector<Collectible>* coins = &level1Coins;
vector<Light>* lights = &level1Lights;

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
    for (char c : text) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
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
    glColor4f(r, g, b, 0.25f + 0.2f * sin(pulse));
    glBegin(GL_QUADS);
        glVertex2f(x - 0.3f, y - 0.1f);
        glVertex2f(x + w + 0.3f, y - 0.1f);
        glVertex2f(x + w + 0.3f, y + h + 0.3f);
        glVertex2f(x - 0.3f, y + h + 0.3f);
    glEnd();
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x+w, y);
        glVertex2f(x+w, y+h);
        glVertex2f(x, y+h);
    glEnd();
    glColor3f(1, 1, 0.9f);
    glBegin(GL_QUADS);
        glVertex2f(x + 0.05f, y + 0.05f);
        glVertex2f(x+w - 0.05f, y + 0.05f);
        glVertex2f(x+w - 0.05f, y+h - 0.05f);
        glVertex2f(x + 0.05f, y+h - 0.05f);
    glEnd();
}

void switchLevel(int level) {
    currentLevel = level;
    if (level == 1) {
        platforms = &level1Platforms;
        coins = &level1Coins;
        lights = &level1Lights;
        glutSetWindowTitle("CS0045 Final - Level 1 Preview");
    } else {
        platforms = &level2Platforms;
        coins = &level2Coins;
        lights = &level2Lights;
        glutSetWindowTitle("CS0045 Final - Level 2 Preview");
    }
}

float frameTime = 0;

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    static float lastT = 0;
    float now = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float dt = now - lastT; lastT = now;
    frameTime = now;

    for (auto& c : *coins) c.bobOffset = sin(frameTime * 2 + c.x) * 0.05f;
    for (auto& l : *lights) l.pulse = frameTime * 4;
    
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
    for (auto& p : *platforms) {
        drawRect(p.x, p.y, p.w, p.h, p.r, p.g, p.b);
        if (p.y == -5.0f) drawGrass(p.x, p.y + p.h, p.w);
    }
    
    // Coins
    for (auto& c : *coins) {
        drawRectTex(c.x, c.y + c.bobOffset, c.w, c.h, coinTexture);
    }
    
    // Exit lights
    for (auto& l : *lights) {
        drawLight(l.x, l.y, l.w, l.h, l.r, l.g, l.b, l.pulse);
    }
    
    // UI
    if (currentLevel == 1) {
        drawText(-9.8f, 4.2f, "LEVEL 1 PREVIEW  (Press 2 for Level 2)", 1, 1, 0);
        drawText(-9.8f, 3.8f, "Blue platforms | Green ground | Gold coin | Yellow exit", 0.8f, 0.8f, 0.8f);
    } else {
        drawText(-9.8f, 4.2f, "LEVEL 2 PREVIEW  (Press 1 for Level 1)", 1, 1, 0);
        drawText(-9.8f, 3.8f, "Purple platforms | Dark green ground sections | Gold coin | Yellow exit", 0.8f, 0.8f, 0.8f);
    }
    drawText(-9.8f, -4.8f, "ESC to exit", 0.5f, 0.5f, 0.5f);

    if (appState == STATE_WIN) { win::draw(dt); setupProjection(); }   // popup overlay on top of the level

    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    if (appState == STATE_WIN) { win::key(key); return; }
    if (key == 27) exit(0);
    if (key == '1') switchLevel(1);
    if (key == '2') switchLevel(2);
    if ((key == 'c' || key == 'C') && currentLevel == 1) { appState = STATE_WIN; win::show(); } // demo: wire to exit collision
}

void mouse(int btn, int state, int x, int y) {
    if (appState != STATE_WIN || btn != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;
    int w = glutGet(GLUT_WINDOW_WIDTH), h = glutGet(GLUT_WINDOW_HEIGHT);
    win::click(x * win::VW / w, y * win::VH / h);
}

void passiveMotion(int x, int y) {
    if (appState != STATE_WIN) return;
    int w = glutGet(GLUT_WINDOW_WIDTH), h = glutGet(GLUT_WINDOW_HEIGHT);
    win::motion(x * win::VW / w, y * win::VH / h);
}

void init() {
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    srand(12345);
    createCheckerTexture();
    createCoinTexture();

    win::loadWinSound();  // load win jingle at startup
    setupProjection();
    switchLevel(1);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1024, 768);
    glutInitWindowPosition(100, 50);
    glutCreateWindow("CS0045 Final - Level Preview");
    
    GLenum err = glewInit();
    if (err != GLEW_OK) return -1;
    
    init();
    win::onNextLevel = [](){ win::stopWinSound(); switchLevel(2); appState = STATE_LEVEL; };
    win::onReplay    = [](){ win::show(); };
    win::onMenu      = [](){ win::stopWinSound(); appState = STATE_LEVEL; };
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutPassiveMotionFunc(passiveMotion);
    glutIdleFunc(display);
    glutMainLoop();
    return 0;
}