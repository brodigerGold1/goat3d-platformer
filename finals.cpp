#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <string>
#include "game_logic.h"
#include <GL/glew.h>
#include <GL/glut.h>
#include "winscreen.h"
#include "background.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// DECLARATIONS ////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

Game game;

// Rendering aliases keep the existing drawing functions easy to recognize.
int& currentLevel = game.currentLevel;
AppState& appState = game.state;
float& playerX = game.player.x;
float& playerY = game.player.y;
float& playerWidth = game.player.width;
float& playerHeight = game.player.height;
vector<GLfloat> trophyPolySlices;
const double PI = 3.141592653589793;
const double FIXED_STEP = 1.0 / 120.0;
int lastTickMs = 0;
double accumulator = 0;
int viewportX = 0, viewportY = 0, viewportW = 1024, viewportH = 768;

///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// TEXTURES /////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void setupProjection() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-10, 10, -5, 5);
    glMatrixMode(GL_MODELVIEW);
}

GLuint coinTexture = 0;

void createCoinTexture() {
    const int SIZE = 32;
    GLubyte image[SIZE][SIZE][4];
    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++) {
            float dx = (x - SIZE/2) / float(SIZE/2);
            float dy = (y - SIZE/2) / float(SIZE/2);
            float dist = sqrt(dx*dx + dy*dy);
            if (dist < 0.9f) { image[y][x][0] = 255; image[y][x][1] = 215; image[y][x][2] = 0; image[y][x][3] = 255; }
            else { image[y][x][0] = 0; image[y][x][1] = 0; image[y][x][2] = 0; image[y][x][3] = 0; }
        }
    }
    glGenTextures(1, &coinTexture);
    glBindTexture(GL_TEXTURE_2D, coinTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SIZE, SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// RENDER FUNCTIONS ////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

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
        float h = 0.19f + 0.04f * sinf(gx * 17.0f); // stable between frames
        glBegin(GL_TRIANGLES);
            glVertex2f(gx, y);
            glVertex2f(gx + 0.08f, y + h);
            glVertex2f(gx + 0.16f, y);
        glEnd();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// LAST LEVEL WIN ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void createPolygonSlice() {
    trophyPolySlices.clear();
    for (int i = 0; i <= 32; i++) {
        trophyPolySlices.push_back(cos(i * PI/16));
        trophyPolySlices.push_back(sin(i * PI/16));
    }
}

void trophyBase() {
    glColor3ub(150, 75, 0);   // brown base
    
    glBegin(GL_QUADS);        // bottom
        glVertex3f(3.0, -3.8, 2.0);
        glVertex3f(7.0, -3.8, 2.0);
        glVertex3f(7.0, -3.8, -2.0);
        glVertex3f(3.0, -3.8, -2.0);
    glEnd();
   
    glBegin(GL_QUADS);        // front
        glVertex3f(3.0, -3.8, 2.0);
        glVertex3f(7.0, -3.8, 2.0);
        glVertex3f(7.0, -3.0, 2.0);
        glVertex3f(3.0, -3.0, 2.0);
    glEnd();
    
    glBegin(GL_QUADS);        // left
        glVertex3f(3.0, -3.8, 2.0);
        glVertex3f(3.0, -3.0, 2.0);
        glVertex3f(3.0, -3.0, -2.0);
        glVertex3f(3.0, -3.8, -2.0);
    glEnd();
    
    glBegin(GL_QUADS);        // right
        glVertex3f(7.0, -3.8, 2.0);
        glVertex3f(7.0, -3.0, 2.0);
        glVertex3f(7.0, -3.0, -2.0);
        glVertex3f(7.0, -3.8, -2.0);
    glEnd();
    
    glBegin(GL_QUADS);        // top
        glVertex3f(3.0, -3.0, 2.0);
        glVertex3f(7.0, -3.0, 2.0);
        glVertex3f(7.0, -3.0, -2.0);
        glVertex3f(3.0, -3.0, -2.0);
    glEnd();
    
    glBegin(GL_QUADS);        // back
        glVertex3f(3.0, -3.8, -2.0);
        glVertex3f(7.0, -3.8, -2.0);
        glVertex3f(7.0, -3.0, -2.0);
        glVertex3f(3.0, -3.0, -2.0);
    glEnd();
}

void trophyLayer(float lowerY, float lowerR, float upperY, float upperR) {
    glColor3ub(255, 210, 0); // gold "body"
    glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= 32; i++) {
            glVertex3f(lowerR * trophyPolySlices[2*i], lowerY, lowerR * trophyPolySlices[2*i + 1]);
            glVertex3f(upperR * trophyPolySlices[2*i], upperY, upperR * trophyPolySlices[2*i + 1]);
        }
    glEnd();
}

void drawD20() {
    glPushMatrix();

    GLfloat lightPosition[] = { 0.0f, 9.0f, 9.0f, 1.0f };
    glTranslatef(-5.0f, 0.0f, 9.0f);
    GLfloat negativeGold[] = { 0.0f, 0.82f, 1.0f, 1.0f };
    GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat white[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    glMaterialfv(GL_FRONT, GL_DIFFUSE, negativeGold);
    glMaterialfv(GL_FRONT, GL_AMBIENT, black);
    glMaterialfv(GL_FRONT, GL_SPECULAR, white);
    glMaterialfv(GL_FRONT, GL_EMISSION, black);
    glMaterialf(GL_FRONT, GL_SHININESS, 100);

    glutSolidIcosahedron();

    glPopMatrix();
}

void displayComplete() {
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-10, 10, -5, 5, -30, 30);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Dim the scenery behind the completion panel
    glColor4f(0.02f, 0.06f, 0.04f, 0.70f);

    glBegin(GL_QUADS);
        glVertex2f(-10, -5);
        glVertex2f( 10, -5);
        glVertex2f( 10,  5);
        glVertex2f(-10,  5);
    glEnd();

    // Gold border and forest-green panel
    drawRect(-8.45f, -4.45f, 16.9f, 8.9f,
             0.95f, 0.77f, 0.30f);

    drawRect(-8.38f, -4.38f, 16.76f, 8.76f,
             0.07f, 0.19f, 0.14f);

    // Divider above the controls
    drawRect(-7.3f, -2.9f, 14.6f, 0.025f,
             0.30f, 0.43f, 0.33f);

    // Center text using its actual pixel width
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    const int viewWidth = viewport[2] > 0 ? viewport[2] : 1;

    auto centeredText = [&](float y, const char* message,
                            void* font, float r, float g, float b) {
        int pixelWidth = 0;

        for (const char* p = message; *p; ++p)
            pixelWidth += glutBitmapWidth(font, *p);

        const float worldWidth = pixelWidth * 20.0f / viewWidth;

        glColor3f(r, g, b);
        glRasterPos2f(-worldWidth / 2.0f, y);

        for (const char* p = message; *p; ++p)
            glutBitmapCharacter(font, *p);
    };

    centeredText(
        3.65f, "YOU ARE THE GOAT3D WINNER!",
        GLUT_BITMAP_TIMES_ROMAN_24,
        1.0f, 0.83f, 0.35f
    );

    centeredText(
        3.1f, "Both levels complete. Well played!",
        GLUT_BITMAP_HELVETICA_18,
        0.90f, 0.95f, 0.88f
    );

    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // Trophy
    glPushMatrix();
        glTranslatef(3.0f, -0.05f, 0.0f);
        glScalef(0.48f, 0.48f, 0.48f);
        glTranslatef(-5.0f, -0.6f, 0.0f);

        trophyBase();

        glPushMatrix();
            glTranslatef(5.0f, 0.0f, 0.0f);

            for (int x = -3; x <= 4; x++) {
                // Keep the original radius calculations.
                trophyLayer(
                    x, sqrt((x + 7) / 3),
                    x + 1, sqrt((x + 8) / 3)
                );
            }
        glPopMatrix();
    glPopMatrix();

    // Diamond lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_NORMALIZE);

    const GLfloat mainLight[] = {
        0.90f, 0.95f, 1.00f, 1.0f
    };

    const GLfloat fillLight[] = {
        0.30f, 0.35f, 0.40f, 1.0f
    };

    const GLfloat fillPosition[] = {
        3.0f, -2.0f, 5.0f, 0.0f
    };

    glLightfv(GL_LIGHT0, GL_DIFFUSE, mainLight);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, fillLight);
    glLightfv(GL_LIGHT1, GL_POSITION, fillPosition);

    // Original diamond, positioned on the left
    glPushMatrix();
        glTranslatef(-3.0f, -0.05f, 0.0f);
        glScalef(1.35f, 1.35f, 1.35f);

        glTranslatef(5.0f, 0.0f, -9.0f);

        drawD20();
    glPopMatrix();

    glDisable(GL_NORMALIZE);
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHT1);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    centeredText(
        -3.55f, "R: Restart game     ESC: Quit",
        GLUT_BITMAP_HELVETICA_18,
        1.0f, 0.83f, 0.35f
    );

    centeredText(
        -4.05f, "Thanks for playing!",
        GLUT_BITMAP_HELVETICA_18,
        0.78f, 0.87f, 0.78f
    );

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glPopAttrib();
}

/////////////////////////////// LIGHTS CAMERA ACTION ////////////////////////////////////////

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
    if (!game.loadLevel(level)) return;
    win::stopWinSound();
    const string title = "GOAT3D - Level " + to_string(currentLevel);
    glutSetWindowTitle(title.c_str());
    accumulator = 0;
}

void showResults() {
    win::coins = game.collected;
    win::time_seconds = static_cast<int>(game.elapsed);
    win::score = game.score();
    win::stars_earned = game.stars();
    win::level_number = currentLevel;
    win::last_level = currentLevel == static_cast<int>(game.levels.size());
    win::show();
}

///////////////////////////// PLAYER APPEARANCE ///////////////////////////////////////////////
void drawPlayer() {
    glPushMatrix();
    glTranslatef(playerX, playerY, 0.0f);

    glColor3f(0.2f, 0.0f, 1.0f);
    glBegin(GL_QUADS);
        glVertex2f(-playerWidth / 2, 0.0f);
        glVertex2f( playerWidth / 2, 0.0f);
        glVertex2f( playerWidth / 2, playerHeight);
        glVertex2f(-playerWidth / 2, playerHeight);
    glEnd();

    glPopMatrix();
}

///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// PHYSICS ////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

// GLUT schedules wakeups; elapsed time determines the number of physics steps
void timer(int) {
    const int now = glutGet(GLUT_ELAPSED_TIME);
    const double elapsed = std::max(0.0, std::min(0.25, (now - lastTickMs) / 1000.0));
    lastTickMs = now;
    if (appState == STATE_LEVEL) {
        accumulator += elapsed;
        while (accumulator >= FIXED_STEP) {
            game.update(static_cast<float>(FIXED_STEP));
            accumulator -= FIXED_STEP;
            if (appState == STATE_WIN) {
                showResults();
                accumulator = 0;
                break;
            }
        }
    } else accumulator = 0;
    glutPostRedisplay(); // Animate all screens, including the completion screen.
    glutTimerFunc(8, timer, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////// DISPLAY FUNCTION //////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    setupProjection();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    static float lastT = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float now = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float dt = std::max(0.f, std::min(0.1f, now - lastT));
    lastT = now;

    // Background
    drawBackground();
    if (appState == STATE_COMPLETE) {
        displayComplete();
        glutSwapBuffers();
        return;
    }

    // Platforms
    for (auto& p : game.platforms) {
        drawRect(p.x, p.y, p.w, p.h, p.r, p.g, p.b);
        if (p.y == -5.0f) drawGrass(p.x, p.y + p.h, p.w);
    }

    // Coins
    for (auto& c : game.coins) {
        drawRectTex(c.x, c.y + c.bobOffset, c.w, c.h, coinTexture);
    }

    // Exit lights
    for (auto& l : game.lights) {
        drawLight(l.x, l.y, l.w, l.h, l.r, l.g, l.b, l.pulse);
    }

    // Player
    drawPlayer();
    
    // HUD reflects actual gameplay, including the exit's locked state.
    drawText(-9.2f, 4.5f, "LEVEL " + to_string(currentLevel) +
             "   COINS " + to_string(game.collected) + "/" +
             to_string(game.levels[currentLevel - 1].coins.size()) +
             "   TIME " + to_string(static_cast<int>(game.elapsed)) +
             "s   FALLS " + to_string(game.deaths), 1, 1, 0);
    drawText(-9.2f, 4.1f, game.coins.empty() ? "Exit unlocked! Reach the yellow light."
             : "Collect all coins to unlock the yellow exit.", 1, 1, 1);
    drawText(-9.2f, -4.45f,
         "A/D: move   SPACE: jump   R: restart level   ESC: quit",
         1, 1, 1);

    if (appState == STATE_WIN) { 
        win::draw(dt); setupProjection(); 
    }   // popup overlay on top of the level

    glutSwapBuffers();
}

///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// CONTROLS ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void keyboard(unsigned char key, int, int) {
    if (appState == STATE_WIN) { win::key(key); return; }
    if (key == 27) { win::stopWinSound(); std::exit(0); }
    if (appState == STATE_COMPLETE) {
        if (key == 'r' || key == 'R') switchLevel(1);
        return;
    }
    if (key == 'r' || key == 'R') { switchLevel(currentLevel); return; }
    if (key == 'a' || key == 'A') game.input.left = true;
    if (key == 'd' || key == 'D') game.input.right = true;
    if (key == ' ') game.pressJump();
}

void keyboardRelease(unsigned char key, int, int) {
    // Always process releases so an overlay cannot leave movement stuck on.
    if (key == 'a' || key == 'A') game.input.left = false;
    if (key == 'd' || key == 'D') game.input.right = false;
    if (key == ' ') game.releaseJump();
}

// Preserve the original 4:3 view and coordinate mapping when resized.
void reshape(int w, int h) {
    w = std::max(1, w); h = std::max(1, h);
    if (w * 3LL > h * 4LL) {
        viewportH = h;
        viewportW = std::max(1, h * 4 / 3);
    } else {
        viewportW = w;
        viewportH = std::max(1, w * 3 / 4);
    }
    viewportX = (w - viewportW) / 2;
    viewportY = (h - viewportH) / 2;
    glViewport(viewportX, viewportY, viewportW, viewportH);
    setupProjection();
    glutPostRedisplay();
}

bool virtualMouse(int x, int y, int& vx, int& vy) {
    const int top = glutGet(GLUT_WINDOW_HEIGHT) - viewportY - viewportH;
    if (x < viewportX || x >= viewportX + viewportW || y < top || y >= top + viewportH)
        return false;
    vx = (x - viewportX) * win::VW / viewportW;
    vy = (y - top) * win::VH / viewportH;
    return true;
}
void mouse(int btn, int state, int x, int y) {
    if (appState != STATE_WIN || btn != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;
    int vx, vy;
    if (virtualMouse(x, y, vx, vy)) win::click(vx, vy);
}
void passiveMotion(int x, int y) {
    if (appState != STATE_WIN) return;
    int vx, vy;
    if (virtualMouse(x, y, vx, vy)) win::motion(vx, vy);
    else win::motion(-1, -1);
}
void visibility(int state) {
    if (state != GLUT_VISIBLE) game.clearInput();
    lastTickMs = glutGet(GLUT_ELAPSED_TIME);
    accumulator = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// INITIALIZATION ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void init() {
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    srand(12345);
    createCoinTexture();
    createPolygonSlice();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    setupProjection();
    switchLevel(1);
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// MAIN FUNCTION ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1024, 768);
    glutInitWindowPosition(100, 50);
    glutCreateWindow("CS0045 Final - Level Preview");

    GLenum err = glewInit();
    if (err != GLEW_OK) return -1;

    if (initBackground() != 0) {
        return -1;
    }

    init();
    win::onNextLevel = [](){
        win::stopWinSound();
        if (currentLevel < static_cast<int>(game.levels.size())) switchLevel(currentLevel + 1);
        else game.nextLevel();
    };
    win::onReplay = [](){ switchLevel(currentLevel); };
    win::onMenu = [](){ switchLevel(currentLevel); }; // ESC on results replays the level
    glutDisplayFunc(display);

    lastTickMs = glutGet(GLUT_ELAPSED_TIME);
    glutTimerFunc(8, timer, 0);
    glutIgnoreKeyRepeat(1);
    glutReshapeFunc(reshape);
    glutVisibilityFunc(visibility);

    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardRelease);
    glutMouseFunc(mouse);

    glutPassiveMotionFunc(passiveMotion);
    glutMainLoop();
    return 0;
}