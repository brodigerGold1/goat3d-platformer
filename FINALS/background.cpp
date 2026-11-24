#include <GL/glew.h>
#include <GL/glut.h>
#include <cmath>
#include <cstdio>                          // printf, for the load error message

#define STB_IMAGE_IMPLEMENTATION           // keep this in this file only
#include "stb_image.h"

#include "background.h"

// x is 51.2 px/unit and y is 76.8 px/unit at 1024x768, so squash y to keep circles round
static const float ROUND = 2.0f / 3.0f;

// ---------- tiny helpers ----------
static void tri(float x1, float y1, float x2, float y2, float x3, float y3) {
    glBegin(GL_TRIANGLES);
    glVertex2f(x1, y1); 
    glVertex2f(x2, y2); 
    glVertex2f(x3, y3);
    glEnd();
}

static void ellipse(float cx, float cy, float rx, float ry) {
    glBegin(GL_TRIANGLE_FAN);

    glVertex2f(cx, cy);

    for (int i = 0; i <= 32; i++) {
        float a = i * 2.0f * 3.14159265f / 32;
        glVertex2f(cx + rx * cosf(a), cy + ry * sinf(a));
    }

    glEnd();
}

// ---------- image texture for the hills ----------
static GLuint hillTex = 0;
static const float TILE = 0.25f;           // the image repeats every 1/TILE = 4 world units

static GLuint loadTexture(const char* path) {
    int w, h, channels;
    unsigned char* data = stbi_load(path, &w, &h, &channels, 3);   // force 3 channels (RGB)
    if (!data) {
        printf("Could not load %s: %s\n", path, stbi_failure_reason());
        return 0;                                                  // 0 = no texture, hills fall back to flat green
    }
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);                         // image rows aren't always a multiple of 4 bytes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);  // tile the image instead of stretching it
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, w, h, GL_RGB, GL_UNSIGNED_BYTE, data);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);                         // back to the default
    stbi_image_free(data);
    return tex;
}

// an ellipse whose vertices carry texture coordinates (same shape as ellipse())
static void texEllipse(float cx, float cy, float rx, float ry, GLuint tex) {
    if (tex) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex);
        glColor3f(1, 1, 1);                // white = show the image's own colors
    } else {
        glColor3f(0.45f, 0.75f, 0.40f);    // fallback: the old flat green
    }

    glBegin(GL_TRIANGLE_FAN);
    glTexCoord2f(cx * TILE, cy * TILE / ROUND);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 32; i++) {
        float a = i * 2.0f * 3.14159265f / 32;
        float x = cx + rx * cosf(a), y = cy + ry * sinf(a);
        glTexCoord2f(x * TILE, y * TILE / ROUND);   // u, v follow the world position, so all hills tile evenly
        glVertex2f(x, y);
    }
    glEnd();

    glDisable(GL_TEXTURE_2D);              // so the rest of the scene (and the game's code) stays untextured
}

// ---------- switch 3D + lighting on/off around the 3D pieces ----------
static void begin3D() {
    // deeper view so wide 3D shapes (the mountains) aren't clipped.
    // Keep x/y the same as the game's gluOrtho2D(-10, 10, -5, 5).
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-10, 10, -5, 5, -10, 10);
    glMatrixMode(GL_MODELVIEW);

    glClear(GL_DEPTH_BUFFER_BIT);          // depth is only used inside begin3D/end3D
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);                // keeps normals correct after glScalef
    glEnable(GL_COLOR_MATERIAL);           // glColor3f now sets the material color
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    GLfloat ambient[] = { 0.60f, 0.60f, 0.65f, 1.0f };   // soft fill so shaded sides aren't black
    GLfloat diffuse[] = { 1.00f, 0.95f, 0.80f, 1.0f };   // warm sunlight
    GLfloat dir[]     = { -0.5f, 0.6f, 0.7f, 0.0f };     // w = 0: directional, from upper-left, toward the viewer
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

    glPushMatrix();
    glLoadIdentity();                      // light direction is set in screen space
    glLightfv(GL_LIGHT0, GL_POSITION, dir);
    glPopMatrix();
}

static void end3D() {                      // restores the plain 2D state the game's code expects
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_NORMALIZE);
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);           // back to the game's own projection
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// ---------- VBO: one unit pine tree (base at y=0) ----------
static GLuint treeVBO;
static const int TRUNK_VERTS = 6, LEAF_VERTS = 9;

int initBackground() {            
    GLenum err = glewInit();      
    if (err != GLEW_OK) return -1; 

    float v[(TRUNK_VERTS + LEAF_VERTS) * 2] = {
        // trunk (2 triangles)
        -0.05f, 0.00f,   0.05f, 0.00f,   0.05f, 0.25f,
        -0.05f, 0.00f,   0.05f, 0.25f,  -0.05f, 0.25f
    };

    int n = TRUNK_VERTS * 2;
    for (int i = 0; i < 3; i++) {  // 3 stacked foliage layers
        float y = 0.20f + i * 0.28f, w = 0.40f - i * 0.08f;
        float t[6] = { -w, y,  w, y,  0, y + 0.50f };
        for (int k = 0; k < 6; k++) v[n++] = t[k];
    }

    glGenBuffers(1, &treeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, treeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    hillTex = loadTexture("grass.jpg");    // if this fails, the hills just stay flat green

    return 0;                      
}

static void drawTreeVBO(float r, float g, float b) {
    glBindBuffer(GL_ARRAY_BUFFER, treeVBO);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, 0);
    glColor3f(0.40f, 0.26f, 0.13f);
    glDrawArrays(GL_TRIANGLES, 0, TRUNK_VERTS); // trunk
    glColor3f(r, g, b);
    glDrawArrays(GL_TRIANGLES, TRUNK_VERTS, LEAF_VERTS); // foliage
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// the same pine as the VBO one, but built from 3D solids (call between begin3D/end3D)
static void pine3D(float r, float g, float b) {
    glColor3f(0.40f, 0.26f, 0.13f);
    glPushMatrix();                                  // trunk: a thin box
        glTranslatef(0, 0.125f, 0);
        glScalef(0.10f, 0.25f, 0.10f);
        glutSolidCube(1.0);
    glPopMatrix();

    glColor3f(r, g, b);
    for (int i = 0; i < 3; i++) {                    // 3 stacked cones, same sizes as the VBO tree
        float y = 0.20f + i * 0.28f, w = 0.40f - i * 0.08f;
        glPushMatrix();
            glTranslatef(0, y, 0);
            glRotatef(-90, 1, 0, 0);                 // a cone points along +z; turn it to point up (+y)
            glutSolidCone(w, 0.50f, 16, 1);
        glPopMatrix();
    }
}

// ---------- scene pieces ----------
static void sky() {
    glBegin(GL_QUADS);
    glColor3f(0.35f, 0.65f, 0.95f); glVertex2f(-10, 5);  glVertex2f(10, 5);
    glColor3f(0.80f, 0.93f, 1.00f); glVertex2f(10, -5);  glVertex2f(-10, -5);
    glEnd();
}

static void sun(float x, float y, float t) {
    float p = 1.0f + 0.05f * sinf(t * 2.0f);
    glPushMatrix();
        glTranslatef(x, y, 0); // TRANSLATE
        glScalef(p, p * ROUND, p); // SCALE (pulse; ROUND keeps it round on screen)
        glColor3f(1.0f, 0.97f, 0.70f);
        for (int i = 0; i < 8; i++) { // ROTATE (spinning rays)
            glPushMatrix();
            glRotatef(t * 10.0f + i * 45.0f, 0, 0, 1);
            tri(-0.12f, 0.95f, 0.12f, 0.95f, 0, 1.6f);
            glPopMatrix();
        }
        glColor3f(1.0f, 0.97f, 0.75f); ellipse(0, 0, 0.90f, 0.90f);   // 2D glow

        // the core is a lit 3D sphere
        GLfloat glow[] = { 0.35f, 0.30f, 0.10f, 1.0f };
        GLfloat none[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        begin3D();
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, glow);   // makes it look like it gives off light
        glColor3f(1.0f, 0.93f, 0.45f);
        glutSolidSphere(0.60, 32, 32);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, none);   // reset so everything else doesn't glow too
        end3D();
    glPopMatrix();
}

// one cloud puff = a sphere squashed into an ellipsoid (call between begin3D/end3D)
static void puff(float x, float y, float rx, float ry) {
    glPushMatrix();
        glTranslatef(x, y, 0);
        glScalef(rx, ry, ry);              // depth = ry, so each puff is as thick as it is tall
        glutSolidSphere(1.0, 24, 24);
    glPopMatrix();
}

// built from 3D puffs, so call it between begin3D/end3D
static void cloud(float baseX, float y, float s, float speed, float t) {
    float x = fmodf(baseX + 12.0f + t * speed, 24.0f) - 12.0f; // drifts right, wraps around
    glPushMatrix();
        glTranslatef(x, y, 0); // TRANSLATE
        glScalef(s, s, s); // SCALE (z too, so the puffs keep their shape)
        glColor3f(1, 1, 1);
        puff( 0.00f, 0.00f, 0.70f, 0.30f);
        puff( 0.50f, 0.12f, 0.50f, 0.30f);
        puff(-0.50f, 0.05f, 0.45f, 0.25f);
    glPopMatrix();
}

// a lit 3D cone, with a smaller white cone on top as the snow cap
static void mountain(float cx, float base, float w, float h,
                     float r, float g, float b) {
    begin3D();                                   // own depth pass, so mountains still stack far-to-near
    glPushMatrix();
        glTranslatef(cx, base, 0);
        glRotatef(-90, 1, 0, 0);                 // a cone points along +z; turn it to point up (+y)
        glColor3f(r * 0.8f, g * 0.8f, b * 0.8f); // a bit darker, since the light adds brightness back
        glutSolidCone(w / 2, h, 32, 1);

        glTranslatef(0, 0, h * 0.7f);            // local +z is "up" now: move to 70% of the height
        glColor3f(1, 1, 1);                      // snow cap = top 30%
        glutSolidCone(w / 2 * 0.3f * 1.02f, h * 0.3f, 32, 1);   // 1.02 stops the two surfaces flickering
    glPopMatrix();
    end3D();
}

static void treeRow(float step, float offset, float size,
                    float r, float g, float b, float t, bool solid) {
    int i = 0;
    for (float x = -10 + offset; x < 10.5f; x += step, i++) {
        float s = size * (0.8f + 0.3f * sinf(i * 2.3f));
        glPushMatrix();
        glTranslatef(x, -4.1f, 0); // TRANSLATE
        glRotatef(2.0f * sinf(t * 1.5f + i), 0, 0, 1); // ROTATE (wind sway)
        glScalef(s, s, s); // SCALE (z too, so the 3D trees stay in proportion)
        if (solid) pine3D(r, g, b); // lit 3D tree
        else       drawTreeVBO(r, g, b); // VBO
        glPopMatrix();
    }
}

// ---------- call FIRST in display(), right after glClear and glLoadIdentity ----------
void drawBackground() {
    float t = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    sky();
    sun(-5.0f, 3.6f, t);

    begin3D();                                                  // lit 3D clouds
    cloud(-1.5f, 3.9f, 1.0f, 0.25f, t);
    cloud( 3.0f, 3.0f, 0.8f, 0.15f, t);
    cloud(-8.0f, 2.2f, 0.7f, 0.20f, t);
    end3D();

    // lit 3D mountains (each one sets up its own 3D pass)
    mountain(-6.0f, -4.0f, 7.0f, 5.0f, 0.60f, 0.68f, 0.85f);
    mountain( 1.0f, -4.0f, 9.0f, 6.0f, 0.55f, 0.64f, 0.82f);
    mountain( 7.5f, -4.0f, 7.0f, 4.5f, 0.60f, 0.68f, 0.85f);
    mountain(-2.5f, -4.0f, 6.0f, 3.6f, 0.42f, 0.60f, 0.58f);
    mountain( 4.5f, -4.0f, 6.5f, 3.2f, 0.42f, 0.60f, 0.58f);

    // textured hills, tall enough to rise above the tree line where you can see them
    texEllipse(-6.0f, -4.3f, 5.0f, 2.5f, hillTex);
    texEllipse( 3.0f, -4.4f, 6.0f, 2.2f, hillTex);
    texEllipse( 9.0f, -4.3f, 4.0f, 2.6f, hillTex);

    treeRow(0.70f, 0.00f, 0.9f, 0.25f, 0.60f, 0.35f, t, false); // back row: flat 2D VBO trees
    begin3D();
    treeRow(0.95f, 0.30f, 1.3f, 0.12f, 0.45f, 0.22f, t, true);  // front row: lit 3D trees
    end3D();
}