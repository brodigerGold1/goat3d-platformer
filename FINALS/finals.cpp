#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <GL/glew.h>
#include <GL/glut.h>
#include "winscreen.h"
#include "background.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// DECLARATIONS ////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

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
    {-10, -5, 5, 1,         0.2f, 0.4f, 0.2f},
    {-2, -5, 3, 1,          0.2f, 0.4f, 0.2f},
    {4, -5, 6, 1,           0.2f, 0.4f, 0.2f},
    {-8, -2.5, 2.5, 0.3,    0.5f, 0.3f, 0.7f},
    {-4, -1.5, 2.5, 0.3,    0.5f, 0.3f, 0.7f},
    {0, -0.5, 2.5, 0.3,     0.5f, 0.3f, 0.7f},
    {-7, 1, 0.5, 0.3,       0.5f, 0.3f, 0.7f},
    {-2, 2, 2.5, 0.3,       0.5f, 0.3f, 0.7f},
    {-9, 3.5, 0.5, 0.3,     0.5f, 0.3f, 0.7f},
    {6, 2, 4, 0.3,          0.5f, 0.3f, 0.7f},
    {6, 0, 3, 0.3,          0.5f, 0.3f, 0.7f},
    {-10, -5, 0.5, 10,      0.5f, 0.3f, 0.2f},
    {10, -5, 0.5, 10,       0.5f, 0.3f, 0.2f},
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
vector<GLfloat> trophyPolySlices;
const double PI = 3.141592653589793;

///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// LAST LEVEL WIN ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void createPolygonSlice() {
    for (int i = 0; i <= 32; i++) {
        trophyPolySlices.push_back(cos(i * PI/16));
        trophyPolySlices.push_back(sin(i * PI/16));
    }
}

void trophyBase() {
    glColor3ub(150, 75, 0); //brown base
    glBegin(GL_QUADS); //bottom
        glVertex3f(3.0, -5.0, 2.0);
        glVertex3f(7.0, -5.0, 2.0);
        glVertex3f(7.0, -5.0, -2.0);
        glVertex3f(3.0, -5.0, -2.0);
    glEnd();
    glBegin(GL_QUADS); //front
        glVertex3f(3.0, -5.0, 2.0);
        glVertex3f(7.0, -5.0, 2.0);
        glVertex3f(7.0, -4.0, 2.0);
        glVertex3f(3.0, -4.0, 2.0);
    glEnd();
    glBegin(GL_QUADS); // left
        glVertex3f(3.0, -5.0, 2.0);
        glVertex3f(3.0, -4.0, 2.0);
        glVertex3f(3.0, -4.0, -2.0);
        glVertex3f(3.0, -5.0, -2.0);
    glEnd();
    glBegin(GL_QUADS); //right
        glVertex3f(7.0, -5.0, 2.0);
        glVertex3f(7.0, -4.0, 2.0);
        glVertex3f(7.0, -4.0, -2.0);
        glVertex3f(7.0, -5.0, -2.0);
    glEnd();
    glBegin(GL_QUADS); //top
        glVertex3f(3.0, -4.0, 2.0);
        glVertex3f(7.0, -4.0, 2.0);
        glVertex3f(7.0, -4.0, -2.0);
        glVertex3f(3.0, -4.0, -2.0);
    glEnd();
    glBegin(GL_QUADS); //back
        glVertex3f(3.0, -5.0, -2.0);
        glVertex3f(7.0, -5.0, -2.0);
        glVertex3f(7.0, -4.0, -2.0);
        glVertex3f(3.0, -4.0, -2.0);
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

    GLfloat lightPosition[] = { 0.0f, 9.0f, 0.0f, 1.0f };
    glTranslatef(-5.0f, 0.0f, 0.0f);
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
    glClear(GL_COLOR_BUFFER_BIT);
    trophyBase();
    for (int x = -4; x <= 4; x++) {
        trophyLayer(x, sqrt((x+7)/3), x+1, sqrt((x+8)/3));
    }
    drawD20();
    glFlush();
}

//////////////////////////// PLAYER //////////////////////////////////////////////////////////

// Player rectangle (for collision detection)
float playerX = 0.0f;
float playerY = 1.0f;
float playerWidth  = 0.8f;
float playerHeight = 1.2f;

// Player physics
bool moveLeft  = false;
bool moveRight = false;

float horizontalSpeed = 0.08f;
float verticalSpeed = 0.0f;

const float gravity = -0.015f;
const float jumpForce = 0.30f;

bool isOnGround = false;

// Ground position
const float groundY = 0.0f;



///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// TEXTURES /////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

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
        float h = 0.15f + (rand() % 10) * 0.01f;
        glBegin(GL_TRIANGLES);
            glVertex2f(gx, y);
            glVertex2f(gx + 0.08f, y + h);
            glVertex2f(gx + 0.16f, y);
        glEnd();
    }
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

// Player repositioning with gravity and ground collision
void updatePlayer() {
    
    // Previous position
    float previousX = playerX;
    float previousY = playerY;

    // Horizontal movement
    if (moveLeft)   playerX -= horizontalSpeed;
    if (moveRight)  playerX += horizontalSpeed;

    // Player surfaces
    float playerLeft = playerX - playerWidth / 2;
    float playerRight = playerX + playerWidth / 2;
    float playerBottom = playerY;
    float playerTop = playerY + playerHeight;

    // Horizontal collision with platform sides
    for (const Platform& p : *platforms) {
        // Determine overlap of player and platform surfaces
        bool verticalOverlap =
            playerTop > p.y &&          // Player top below platform top AND&&
            playerBottom < p.y + p.h;   // Player bottom above platform bottom
        bool horizontalOverlap =
            playerRight > p.x &&        // Player right to the right of platform left AND
            playerLeft < p.x + p.w;     // Player left to the left of platform right

        // If true, no more moving that way
        if (verticalOverlap && horizontalOverlap) {
            if (moveRight)
                playerX = p.x - playerWidth / 2.0f;
            else if (moveLeft)
                playerX = p.x + p.w + playerWidth / 2.0f;

            playerLeft  = playerX - playerWidth / 2.0f;
            playerRight = playerX + playerWidth / 2.0f;
        }
    }

    // Vertical movement + gravity
    verticalSpeed += gravity;
    playerY += verticalSpeed;

    // Reset player surfaces after potential horizontal collision adjustments
    playerBottom = playerY;
    playerTop = playerY + playerHeight;
    isOnGround = false;

    // Vertical collision with platform tops and bottoms
    for (const Platform& p : *platforms)
    {
        bool horizontalOverlap =
            playerRight > p.x &&
            playerLeft < p.x + p.w;

        float platformBottom = p.y;
        float platformTop = p.y + p.h;

        // Landing on top while falling
        bool onTop =
            verticalSpeed <= 0.0f &&
            previousY >= platformTop &&
            playerBottom <= platformTop;
        if (horizontalOverlap && onTop) {
            // Modify playerY directly to avoid overlap
            playerY = platformTop;
            verticalSpeed = 0.0f;
            isOnGround = true;
        }

        // Hitting the underside while jumping
        bool owMyHead =
            verticalSpeed > 0.0f &&
            previousY + playerHeight <= platformBottom &&
            playerTop >= platformBottom;
        if (horizontalOverlap && owMyHead) {
            // Modify playerY directly to avoid overlap
            playerY = platformBottom - playerHeight;
            verticalSpeed = 0.0f;
        }
    }

    glutPostRedisplay();
}

// Timer for consistent movement across frames
void timer(int value) {
    updatePlayer();
    glutTimerFunc(16, timer, 0); // Approximately 60 FPS
}

///////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////// DISPLAY FUNCTION //////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

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
    drawBackground();
    
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

    // Player
    drawPlayer();
    
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



///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// CONTROLS ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void keyboard(unsigned char key, int x, int y) {
    if (appState == STATE_WIN) { win::key(key); return; }

    // Menu
    if (key == 27) exit(0);
    if (key == '1') switchLevel(1);
    if (key == '2') switchLevel(2);
    if ((key == 'c' || key == 'C') && currentLevel == 1) { appState = STATE_WIN; win::show(); } // demo: wire to exit collision
    if ((key == 'c' || key == 'C') && currentLevel == 2) { displayComplete(); } // demo: wire to exit collision
    
    // Player movement
    if (key == 'a') moveLeft = true;
    if (key == 'd') moveRight = true;
    if (key == ' ' && isOnGround)
    {
        verticalSpeed = jumpForce;
        isOnGround = false;
    }
}

// Keys that need holding down return false on release.
void keyboardRelease(unsigned char key, int x, int y) {
    if (key == 'a') moveLeft = false;
    if (key == 'd') moveRight = false;
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


///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// INITIALIZATION ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void init() {
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    srand(12345);
    createCheckerTexture();
    createCoinTexture();
    createPolygonSlice();
    win::loadWinSound();  // load win jingle at startup
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
    win::onNextLevel = [](){ win::stopWinSound(); switchLevel(2); appState = STATE_LEVEL; };
    win::onReplay    = [](){ win::show(); };
    win::onMenu      = [](){ win::stopWinSound(); appState = STATE_LEVEL; };
    glutDisplayFunc(display);

    glutTimerFunc(16, timer, 0); // Start off timer for physics updates

    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardRelease);
    glutMouseFunc(mouse);

    glutPassiveMotionFunc(passiveMotion);
    glutIdleFunc(display);
    glutMainLoop();
    return 0;
}