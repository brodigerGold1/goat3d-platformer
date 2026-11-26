#include <GL/glew.h>
#include <GL/glut.h>
#include <cmath>
#include <cstdio>

// Define the image loader implementation in this source file only
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "background.h"

// Aspect correction for round shapes at the original 4:3 viewport
static const float ROUND = 2.0f / 3.0f;

// ---------- Shared 2D shape helpers ----------
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

// ---------- Hills: texture loading and drawing ----------
static GLuint hillTex = 0;
static const float TILE = 0.25f;

static GLuint loadTexture(const char* path) {
    int w, h, channels;
    unsigned char* data = stbi_load(path, &w, &h, &channels, 3);

    if (!data) {
        printf("Could not load %s: %s\n", path, stbi_failure_reason());
        return 0;
    }

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    // RGB image rows may not be aligned to four-byte boundaries
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, w, h, GL_RGB, GL_UNSIGNED_BYTE, data);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    stbi_image_free(data);
    return tex;
}

static void texEllipse(float cx, float cy, float rx, float ry, GLuint tex) {
    if (tex) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex);
        glColor3f(1, 1, 1);
    } else {
        // Use a solid color if the texture could not be loaded
        glColor3f(0.45f, 0.75f, 0.40f);
    }

    glBegin(GL_TRIANGLE_FAN);
    glTexCoord2f(cx * TILE, cy * TILE / ROUND);
    glVertex2f(cx, cy);

    for (int i = 0; i <= 32; i++) {
        float a = i * 2.0f * 3.14159265f / 32;
        float x = cx + rx * cosf(a), y = cy + ry * sinf(a);

        // Use world coordinates to keep the texture scale consistent across hills.
        glTexCoord2f(x * TILE, y * TILE / ROUND);
        glVertex2f(x, y);
    }

    glEnd();
    glDisable(GL_TEXTURE_2D);
}

// ---------- 3D projection and lighting ----------
// Each pass clears depth so background layers follow their drawing order
static void begin3D() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-10, 10, -5, 5, -10, 10);
    glMatrixMode(GL_MODELVIEW);

    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    GLfloat ambient[] = { 0.60f, 0.60f, 0.65f, 1.0f };
    GLfloat diffuse[] = { 1.00f, 0.95f, 0.80f, 1.0f };
    GLfloat dir[]     = { -0.5f, 0.6f, 0.7f, 0.0f };

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

    // Set the light direction independently of object transformations
    glPushMatrix();
    glLoadIdentity();
    glLightfv(GL_LIGHT0, GL_POSITION, dir);
    glPopMatrix();
}

// Disable 3D rendering and restore the saved projection
static void end3D() {
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_NORMALIZE);
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// ---------- Trees: buffer data and initialization ----------
static GLuint treeVBO;
static const int TRUNK_VERTS = 6, LEAF_VERTS = 9;

int initBackground() {
    // Two triangles form the trunk; three more form the foliage
    float v[(TRUNK_VERTS + LEAF_VERTS) * 2] = {
        -0.05f, 0.00f,   0.05f, 0.00f,   0.05f, 0.25f,
        -0.05f, 0.00f,   0.05f, 0.25f,  -0.05f, 0.25f
    };

    int n = TRUNK_VERTS * 2;

    for (int i = 0; i < 3; i++) {
        float y = 0.20f + i * 0.28f, w = 0.40f - i * 0.08f;
        float t[6] = { -w, y,  w, y,  0, y + 0.50f };

        for (int k = 0; k < 6; k++) v[n++] = t[k];
    }

    glGenBuffers(1, &treeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, treeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    hillTex = loadTexture("grass.jpg");

    return 0;
}

// ---------- Trees: flat VBO model ----------
static void drawTreeVBO(float r, float g, float b) {
    glBindBuffer(GL_ARRAY_BUFFER, treeVBO);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, 0);

    glColor3f(0.40f, 0.26f, 0.13f);
    glDrawArrays(GL_TRIANGLES, 0, TRUNK_VERTS);

    glColor3f(r, g, b);
    glDrawArrays(GL_TRIANGLES, TRUNK_VERTS, LEAF_VERTS);

    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// ---------- Trees: 3D trunk and foliage ----------
static void pine3D(float r, float g, float b) {
    glColor3f(0.40f, 0.26f, 0.13f);

    glPushMatrix();
        glTranslatef(0, 0.125f, 0);
        glScalef(0.10f, 0.25f, 0.10f);
        glutSolidCube(1.0);
    glPopMatrix();

    glColor3f(r, g, b);

    for (int i = 0; i < 3; i++) {
        float y = 0.20f + i * 0.28f, w = 0.40f - i * 0.08f;

        glPushMatrix();
            glTranslatef(0, y, 0);

            // GLUT cones point along +Z; rotate them upward along +Y
            glRotatef(-90, 1, 0, 0);
            glutSolidCone(w, 0.50f, 16, 1);
        glPopMatrix();
    }
}

// ---------- Sky: color gradient ----------
static void sky() {
    glBegin(GL_QUADS);
    glColor3f(0.35f, 0.65f, 0.95f); glVertex2f(-10, 5);  glVertex2f(10, 5);
    glColor3f(0.80f, 0.93f, 1.00f); glVertex2f(10, -5);  glVertex2f(-10, -5);
    glEnd();
}

// ---------- Sun: rotating rays, pulsing glow, and 3D core ----------
static void sun(float x, float y, float t) {
    float p = 1.0f + 0.05f * sinf(t * 2.0f);

    glPushMatrix();
        glTranslatef(x, y, 0);
        glScalef(p, p * ROUND, p);
        glColor3f(1.0f, 0.97f, 0.70f);

        for (int i = 0; i < 8; i++) {
            glPushMatrix();
            glRotatef(t * 10.0f + i * 45.0f, 0, 0, 1);
            tri(-0.12f, 0.95f, 0.12f, 0.95f, 0, 1.6f);
            glPopMatrix();
        }

        glColor3f(1.0f, 0.97f, 0.75f);
        ellipse(0, 0, 0.90f, 0.90f);

        // Give the sphere an emissive material, then reset it after drawing.
        GLfloat glow[] = { 0.35f, 0.30f, 0.10f, 1.0f };
        GLfloat none[] = { 0.0f, 0.0f, 0.0f, 1.0f };

        begin3D();
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, glow);
        glColor3f(1.0f, 0.93f, 0.45f);
        glutSolidSphere(0.60, 32, 32);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, none);
        end3D();
    glPopMatrix();
}

// ---------- Clouds: 3D puffs and horizontal movement ----------
static void puff(float x, float y, float rx, float ry) {
    glPushMatrix();
        glTranslatef(x, y, 0);
        glScalef(rx, ry, ry);
        glutSolidSphere(1.0, 24, 24);
    glPopMatrix();
}

static void cloud(float baseX, float y, float s, float speed, float t) {
    // Wrap clouds back to the left after they move beyond the scene
    float x = fmodf(baseX + 12.0f + t * speed, 24.0f) - 12.0f;

    glPushMatrix();
        glTranslatef(x, y, 0);
        glScalef(s, s, s);
        glColor3f(1, 1, 1);
        puff( 0.00f, 0.00f, 0.70f, 0.30f);
        puff( 0.50f, 0.12f, 0.50f, 0.30f);
        puff(-0.50f, 0.05f, 0.45f, 0.25f);
    glPopMatrix();
}

// ---------- Mountains: 3D cones and snowcaps ----------
static void mountain(float cx, float base, float w, float h,
                     float r, float g, float b) {
    begin3D();

    glPushMatrix();
        glTranslatef(cx, base, 0);
        glRotatef(-90, 1, 0, 0);
        glColor3f(r * 0.8f, g * 0.8f, b * 0.8f);
        glutSolidCone(w / 2, h, 32, 1);

        // The snowcap covers the top 30%; its extra width reduces surface flicker
        glTranslatef(0, 0, h * 0.7f);
        glColor3f(1, 1, 1);
        glutSolidCone(w / 2 * 0.3f * 1.02f, h * 0.3f, 32, 1);
    glPopMatrix();

    end3D();
}

// ---------- Trees: row placement, size variation, and wind sway ----------
static void treeRow(float step, float offset, float size,
                    float r, float g, float b, float t, bool solid) {
    int i = 0;

    for (float x = -10 + offset; x < 10.5f; x += step, i++) {
        float s = size * (0.8f + 0.3f * sinf(i * 2.3f));

        glPushMatrix();
        glTranslatef(x, -4.1f, 0);
        glRotatef(2.0f * sinf(t * 1.5f + i), 0, 0, 1);
        glScalef(s, s, s);

        if (solid) pine3D(r, g, b);
        else       drawTreeVBO(r, g, b);

        glPopMatrix();
    }
}

// ---------- Complete background: drawing order ----------
// Call after clearing the frame and resetting the modelview matrix
void drawBackground() {
    // Preserve the caller's render state and matrices
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glMatrixMode(GL_PROJECTION); glPushMatrix();
    glMatrixMode(GL_MODELVIEW); glPushMatrix();

    float t = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    // Sky and sun
    sky();
    sun(-5.0f, 3.6f, t);

    // Clouds
    begin3D();
    cloud(-1.5f, 3.9f, 1.0f, 0.25f, t);
    cloud( 3.0f, 3.0f, 0.8f, 0.15f, t);
    cloud(-8.0f, 2.2f, 0.7f, 0.20f, t);
    end3D();

    // Mountains
    mountain(-6.0f, -4.0f, 7.0f, 5.0f, 0.60f, 0.68f, 0.85f);
    mountain( 1.0f, -4.0f, 9.0f, 6.0f, 0.55f, 0.64f, 0.82f);
    mountain( 7.5f, -4.0f, 7.0f, 4.5f, 0.60f, 0.68f, 0.85f);
    mountain(-2.5f, -4.0f, 6.0f, 3.6f, 0.42f, 0.60f, 0.58f);
    mountain( 4.5f, -4.0f, 6.5f, 3.2f, 0.42f, 0.60f, 0.58f);

    // Grass-textured hills
    texEllipse(-6.0f, -4.3f, 5.0f, 2.5f, hillTex);
    texEllipse( 3.0f, -4.4f, 6.0f, 2.2f, hillTex);
    texEllipse( 9.0f, -4.3f, 4.0f, 2.6f, hillTex);

    // Back row: flat trees. Front row: lit 3D trees
    treeRow(0.70f, 0.00f, 0.9f, 0.25f, 0.60f, 0.35f, t, false);

    begin3D();
    treeRow(0.95f, 0.30f, 1.3f, 0.12f, 0.45f, 0.22f, t, true);
    end3D();

    // Restore the caller's matrices and render state.
    glMatrixMode(GL_MODELVIEW); glPopMatrix();
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glPopClientAttrib();
    glPopAttrib();
}