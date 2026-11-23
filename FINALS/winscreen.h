// winscreen.h — "LEVEL COMPLETE" win screen, legacy fixed-function OpenGL style.
// 960x504 virtual coords, y down, origin top-left (glOrtho(0,960,504,0)).
// Stars are real 3D meshes drawn in a perspective pass after the 2D UI.
//
// Wire-up: call win::show() when the player reaches the exit. Set
// win::onNextLevel / win::onReplay / onMenu callbacks once at startup.
#pragma once
#include <cmath>
#include <cstring>
#include <vector>
#include <functional>
#include <GL/glew.h>
#include <GL/glut.h>
#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#endif

namespace win {

struct C { float r, g, b; };
static C hex3(int h) { return { ((h >> 16) & 255) / 255.f, ((h >> 8) & 255) / 255.f, (h & 255) / 255.f }; }

// ======================= tunables =======================
const int   VW = 960, VH = 504;                       // virtual coordinate space
const int   CONF_COUNT = 80;
const float CONF_COIN_PCT = 0.14f;
const C     PANEL = hex3(0x173A2B), PANEL_DARK = hex3(0x0E2A1C);
const C     GOLD = hex3(0xFFD200), BTN_GREEN = hex3(0x6FBF63), BTN_GOLD = hex3(0xFFD200);
const C     WHITE = hex3(0xFFFFFF), GREY = hex3(0x8A8A8A), TEXT_SH = hex3(0x0E2A1C);
const C     CONF_COLS[4] = { hex3(0xFFD200), hex3(0xFFFFFF), hex3(0xE23A3A), hex3(0x9ADCF0) };
const int   PANEL_X = (VW - 480) / 2, PANEL_Y = (VH - 320) / 2, PANEL_W = 480, PANEL_H = 320;
const float STAR_CY = PANEL_Y + 122, STAR_R = 32;     // star row center, outer radius px
const float STAR_DELAY = 0.35f, STAR_POP_T = 0.25f;   // stagger + pop duration
const float STAR_SPIN = 120.f, STAR_TILT = 10.f;      // deg/sec, deg
const float STAR_BOUNCE_H = 0.25f;                    // in star-heights
const float COUNT_T = 1.5f;                           // stat count-up duration
const int   BTN_W = 190, BTN_H = 40, BTN_Y = PANEL_Y + 244;
const int   BTN_NEXT_X = 262, BTN_REPLAY_X = 508;

// ======================= data to wire later =======================
int coins = 0;                 // <-- plug real coin count
int time_seconds = 0;          // <-- plug real elapsed time
int score = 0;                 // <-- plug real score
int stars_earned = 3;          // <-- plug real star count (0..3)

std::function<void()> onNextLevel, onReplay, onMenu;

// ======================= state =======================
static float winT = 0;         // time since shown
static bool  built = false;
static int   hoverBtn = -1;    // -1 none, 0 next, 1 replay

struct Conf  { float x, y, vy, sway, phase; int type; C col; };
static std::vector<Conf> confetti;

// ======================= tiny pixel font (5x7) =======================
static const unsigned char GLYPHS[][7] = {
    /* '0' */ {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E},
    /* '1' */ {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E},
    /* '2' */ {0x0E,0x11,0x01,0x02,0x04,0x08,0x1F},
    /* '3' */ {0x1F,0x02,0x04,0x02,0x01,0x11,0x0E},
    /* '4' */ {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02},
    /* '5' */ {0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E},
    /* '6' */ {0x06,0x08,0x10,0x1E,0x11,0x11,0x0E},
    /* '7' */ {0x1F,0x01,0x02,0x04,0x08,0x08,0x08},
    /* '8' */ {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E},
    /* '9' */ {0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C},
    /* 'A' */ {0x0E,0x11,0x11,0x1F,0x11,0x11,0x11},
    /* 'C' */ {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E},
    /* 'E' */ {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F},
    /* 'F' */ {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10},
    /* 'I' */ {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E},
    /* 'L' */ {0x10,0x10,0x10,0x10,0x10,0x10,0x1F},
    /* 'M' */ {0x11,0x1B,0x15,0x15,0x11,0x11,0x11},
    /* 'N' */ {0x11,0x19,0x15,0x13,0x11,0x11,0x11},
    /* 'O' */ {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E},
    /* 'P' */ {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10},
    /* 'R' */ {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11},
    /* 'S' */ {0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E},
    /* 'T' */ {0x1F,0x04,0x04,0x04,0x04,0x04,0x04},
    /* 'V' */ {0x11,0x11,0x11,0x11,0x11,0x0A,0x04},
    /* 'X' */ {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11},
    /* 'Y' */ {0x11,0x11,0x0A,0x04,0x04,0x04,0x04},
    /* ':' */ {0x00,0x0C,0x0C,0x00,0x0C,0x0C,0x00},
    /* '!' */ {0x04,0x04,0x04,0x04,0x04,0x00,0x04},
    /* '-' */ {0x00,0x00,0x00,0x1F,0x00,0x00,0x00},
    /* ' ' */ {0x00,0x00,0x00,0x00,0x00,0x00,0x00},
};
static const char* GLYPH_ORDER = "0123456789ACEFILMNOPRSTVXY:!- ";

static const unsigned char* glyph(char c) {
    const char* p = strchr(GLYPH_ORDER, c);
    return p ? GLYPHS[p - GLYPH_ORDER] : GLYPHS[sizeof(GLYPH_ORDER) - 2]; // space
}
static int textW(const char* s, int scale) { return (int)strlen(s) * 6 * scale - scale; }
static void drawText(float x, float y, const char* s, int scale, C col) {
    glColor3f(col.r, col.g, col.b);
    glBegin(GL_QUADS);
    for (const char* p = s; *p; p++) {
        const unsigned char* g = glyph(*p);
        for (int row = 0; row < 7; row++)
            for (int cx = 0; cx < 5; cx++)
                if (g[row] & (1 << (4 - cx))) {
                    float px = x + (p - s) * 6 * scale + cx * scale;
                    float py = y + row * scale;
                    glVertex2f(px, py); glVertex2f(px + scale, py);
                    glVertex2f(px + scale, py + scale); glVertex2f(px, py + scale);
                }
    }
    glEnd();
}
static void drawTextShadow(float x, float y, const char* s, int scale, C col, C sh) {
    drawText(x + 2, y + 2, s, scale, sh);
    drawText(x, y, s, scale, col);
}

// ======================= 2D shape helpers =======================
static void rect(float x, float y, float w, float h, C c) {
    glColor3f(c.r, c.g, c.b);
    glBegin(GL_QUADS);
        glVertex2f(x, y); glVertex2f(x + w, y);
        glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
}
static void dashedLine(float x0, float x1, float y, C c) {
    for (float x = x0; x < x1; x += 10) rect(x, y, 5, 2, c);
}
static void button(float x, float y, float w, float h, const char* label, C fill, bool hot) {
    rect(x + 4, y + 4, w, h, PANEL_DARK);                       // drop shadow
    rect(x, y, w, h, hot ? C{fill.r * 1.15f, fill.g * 1.15f, fill.b * 1.15f} : fill);
    rect(x, y + h - 4, w, 4, C{ fill.r * 0.55f, fill.g * 0.55f, fill.b * 0.55f }); // bevel bottom
    rect(x + w - 4, y, 4, h, C{ fill.r * 0.55f, fill.g * 0.55f, fill.b * 0.55f }); // bevel right
    rect(x, y, w, 3, C{ fill.r * 1.25f > 1 ? 1 : fill.r * 1.25f,
                        fill.g * 1.25f > 1 ? 1 : fill.g * 1.25f,
                        fill.b * 1.25f > 1 ? 1 : fill.b * 1.25f });
    if (hot) { rect(x - 2, y - 2, w + 4, 2, GOLD); rect(x - 2, y + h, w + 4, 2, GOLD);
               rect(x - 2, y, 2, h, GOLD); rect(x + w, y, 2, h, GOLD); }
    int sc = 3;
    drawTextShadow(x + (w - textW(label, sc)) / 2.f, y + (h - 7 * sc) / 2.f, label, sc,
                   hot ? WHITE : PANEL, TEXT_SH);
}

// ======================= 3D star mesh =======================
static std::vector<float> sv, sn, sc;                          // verts, normals, colors
static int starVerts = 0;

static void addTri(const float* a, const float* b, const float* c,
                   const float* n, C col) {
    for (const float* v : {a, b, c}) { sv.insert(sv.end(), v, v + 3); }
    for (int i = 0; i < 3; i++) sn.insert(sn.end(), n, n + 3);
    for (int i = 0; i < 3; i++) sc.insert(sc.end(), &col.r, &col.r + 3);
    starVerts += 3;
}
static void buildStar() {
    const float Ro = 1.0f, Ri = 0.45f, D = 0.35f, hz = D / 2;
    float ov[10][3], iv[10][3];                                // outline verts, front z
    for (int k = 0; k < 10; k++) {
        float a = (90.f + k * 36.f) * 3.14159265f / 180.f;
        float r = (k % 2 == 0) ? Ro : Ri;
        ov[k][0] = cosf(a) * r; ov[k][1] = sinf(a) * r; ov[k][2] = hz;
        iv[k][0] = ov[k][0];    iv[k][1] = ov[k][1];    iv[k][2] = -hz;
    }
    C front = hex3(0xFFF175), back = hex3(0xFFE030), side = hex3(0xFFC400);
    float nf[3] = {0, 0, 1}, nb[3] = {0, 0, -1}, ctr[3] = {0, 0, hz}, cb[3] = {0, 0, -hz};
    for (int k = 0; k < 10; k++) {
        int k1 = (k + 1) % 10;
        addTri(ctr, ov[k], ov[k1], nf, front);                 // front fan (CCW -> +z)
        addTri(cb, iv[k1], iv[k], nb, back);                   // back fan (CW from front)
        float dx = ov[k1][0] - ov[k][0], dy = ov[k1][1] - ov[k][1];
        float len = sqrtf(dx * dx + dy * dy);
        float nw[3] = { dy / len, -dx / len, 0 };              // outward normal of CCW edge
        addTri(ov[k],  iv[k],  iv[k1], nw, side);              // side wall quad (2 tris)
        addTri(ov[k],  iv[k1], ov[k1], nw, side);
    }
}
static void drawStarMesh() {
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, sv.data());
    glNormalPointer(GL_FLOAT, 0, sn.data());
    glColorPointer(3, GL_FLOAT, 0, sc.data());
    glDrawArrays(GL_TRIANGLES, 0, starVerts);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}
static float easeOutBack(float t) {
    const float c1 = 1.70158f, c3 = c1 + 1.f;
    return 1 + c3 * powf(t - 1, 3) + c1 * powf(t - 1, 2);
}
static float clamp01(float v) { return v < 0 ? 0 : (v > 1 ? 1 : v); }

// ======================= win jingle (win_jingle.wav, trailing silence trimmed, looped) =======================
const char* WIN_WAV = "win_jingle.wav";     // drop-in asset next to the exe
const float WIN_VOLUME = 0.01f;             // volume scale applied to samples at load
const int   TRIM_THRESHOLD = 900;           // 16-bit amplitude treated as silence (~-31dB)
const float TRIM_PAD_MS = 40.f;             // tail kept after last audible frame

static std::vector<unsigned char> wav;
static bool wavReady = false;
static void loadWinSound() {
#ifdef _WIN32
    FILE* f = fopen(WIN_WAV, "rb");
    if (!f) { printf("win sound: %s not found\n", WIN_WAV); return; }
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    wav.resize(sz);
    fread(wav.data(), 1, sz, f); fclose(f);
    if (sz < 44 || memcmp(wav.data(), "RIFF", 4) || memcmp(&wav[8], "WAVE", 4)) {
        printf("win sound: not a wav\n"); wav.clear(); return;
    }
    // walk chunks -> locate fmt + data
    size_t pos = 12; int fmtOff = -1, dataOff = -1, dataLen = 0;
    int ch = 0, bits = 0, rate = 0;
    while (pos + 8 <= (size_t)sz) {
        int len = *(int*)&wav[pos + 4];
        if (!memcmp(&wav[pos], "fmt ", 4)) {
            fmtOff = (int)pos + 8;
            ch = *(short*)&wav[fmtOff + 2]; rate = *(int*)&wav[fmtOff + 4];
            bits = *(short*)&wav[fmtOff + 14];
        }
        if (!memcmp(&wav[pos], "data", 4)) { dataOff = (int)pos + 8; dataLen = len; break; }
        pos += 8 + len + (len & 1);
    }
    if (bits != 16 || ch < 1 || dataOff < 0 || rate <= 0) {
        printf("win sound: unsupported wav (needs 16-bit PCM)\n"); wav.clear(); return;
    }
    // find last audible frame (any channel above threshold), keep TRIM_PAD_MS tail
    int frameB = ch * 2;
    int frames = dataLen / frameB;
    short* smp = (short*)&wav[dataOff];
    int last = frames - 1;
    while (last > 0) {
        bool audible = false;
        for (int c = 0; c < ch; c++)
            if (abs(smp[last * ch + c]) > TRIM_THRESHOLD) { audible = true; break; }
        if (audible) break;
        last--;
    }
    int pad = (int)(rate * TRIM_PAD_MS / 1000.f);
    int newFrames = last + 1 + pad < frames ? last + 1 + pad : frames;
    int newLen = newFrames * frameB;
    // attenuate volume
    for (int i = 0; i < newFrames * ch; i++) {
        int v = (int)(smp[i] * WIN_VOLUME);
        smp[i] = (short)(v > 32767 ? 32767 : (v < -32768 ? -32768 : v));
    }
    *(int*)&wav[dataOff - 4] = newLen;                          // patch data chunk size
    *(int*)&wav[4] = 36 + newLen;                               // patch RIFF size
    wav.resize(dataOff + newLen);
    printf("win sound: trimmed %dms of trailing silence (%.2fs -> %.2fs)\n",
           (frames - newFrames) * 1000 / rate, frames / (float)rate, newFrames / (float)rate);
    wavReady = true;
#endif
}
static void playWinSound() {
#ifdef _WIN32
    if (!wavReady) loadWinSound();
    if (wavReady)
        PlaySoundA((LPCSTR)wav.data(), NULL, SND_MEMORY | SND_ASYNC | SND_LOOP | SND_NODEFAULT);
#endif
}
static void stopWinSound() {
#ifdef _WIN32
    PlaySoundA(NULL, NULL, SND_ASYNC | SND_NODEFAULT);
#endif
}

// ======================= init / show =======================
static void build() {
    confetti.clear();
    for (int i = 0; i < CONF_COUNT; i++) {
        bool coin = (rand() % 100) < (int)(CONF_COIN_PCT * 100);
        confetti.push_back({ (float)(rand() % VW), (float)(rand() % VH),
                             (float)(60 + rand() % 90), (float)(20 + rand() % 30),
                             (float)(rand() % 628) / 100.f, coin ? 1 : 0,
                             coin ? GOLD : CONF_COLS[i % 4] });
    }
    buildStar();
    built = true;
}
static void show() {
    if (!built) build();
    confetti.clear();                                          // re-roll confetti
    for (int i = 0; i < CONF_COUNT; i++) {
        bool coin = (rand() % 100) < (int)(CONF_COIN_PCT * 100);
        confetti.push_back({ (float)(rand() % VW), (float)(-rand() % VH),
                             (float)(60 + rand() % 90), (float)(20 + rand() % 30),
                             (float)(rand() % 628) / 100.f, coin ? 1 : 0,
                             coin ? GOLD : CONF_COLS[i % 4] });
    }
    winT = 0;
    hoverBtn = -1;
    playWinSound();
}

// ======================= input =======================
static void key(unsigned char k) {
    if (k == '2' || k == 13) { if (onNextLevel) onNextLevel(); }
    else if (k == 'r' || k == 'R') { if (onReplay) onReplay(); else show(); }
    else if (k == 27) { if (onMenu) onMenu(); }
}
static void motion(int vx, int vy) {
    hoverBtn = -1;
    if (vy >= BTN_Y && vy <= BTN_Y + BTN_H) {
        if (vx >= BTN_NEXT_X && vx <= BTN_NEXT_X + BTN_W) hoverBtn = 0;
        else if (vx >= BTN_REPLAY_X && vx <= BTN_REPLAY_X + BTN_W) hoverBtn = 1;
    }
}
static void click(int vx, int vy) {
    motion(vx, vy);
    if (hoverBtn == 0 && onNextLevel) onNextLevel();
    else if (hoverBtn == 1) { if (onReplay) onReplay(); else show(); }
}

// ======================= draw =======================
static void draw(float dt) {
    if (!built) build();
    winT += dt;

    // ---- ortho 960x504, y down ----
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glOrtho(0, VW, VH, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    glDisable(GL_TEXTURE_2D); glDisable(GL_DEPTH_TEST); glDisable(GL_LIGHTING);

    // confetti (falls over the level view behind the panel)
    for (auto& c : confetti) {
        c.y += c.vy * dt;
        c.x += sinf(winT * 2.f + c.phase) * c.sway * dt;
        if (c.y > VH + 8) { c.y = -8; c.x = (float)(rand() % VW); }
        if (c.type == 1) {                                       // spinning coin
            float w = 7 * fabsf(cosf(winT * 4.f + c.phase)) + 1;
            rect(c.x - w / 2, c.y - 4, w, 8, GOLD);
        } else rect(c.x - 3, c.y - 3, 6, 6, c.col);
    }

    // ---- panel ----
    rect(PANEL_X + 7, PANEL_Y + 7, PANEL_W, PANEL_H, PANEL_DARK); // shadow
    rect(PANEL_X - 3, PANEL_Y - 3, PANEL_W + 6, PANEL_H + 6, PANEL_DARK);
    rect(PANEL_X, PANEL_Y, PANEL_W, PANEL_H, GOLD);               // 3px gold border
    rect(PANEL_X + 3, PANEL_Y + 3, PANEL_W - 6, PANEL_H - 6, PANEL);
    drawTextShadow(PANEL_X + (PANEL_W - textW("LEVEL 1", 5)) / 2.f, PANEL_Y + 16, "LEVEL 1", 5, GOLD, TEXT_SH);
    drawTextShadow(PANEL_X + (PANEL_W - textW("COMPLETE!", 5)) / 2.f, PANEL_Y + 54, "COMPLETE!", 5, GOLD, TEXT_SH);

    // ---- stats (count up over COUNT_T, ease-out) ----
    float cf = 1 - (1 - clamp01(winT / COUNT_T)) * (1 - clamp01(winT / COUNT_T));
    int dCoins = (int)(coins * cf), dScore = (int)(score * cf);
    int dTime = (int)(time_seconds * cf);
    char buf[32];
    const int sc = 3, rowY[3] = { PANEL_Y + 168, PANEL_Y + 192, PANEL_Y + 216 };
    const char* labels[3] = { "COINS", "TIME", "SCORE" };
    snprintf(buf, sizeof(buf), "%d", dCoins);
    char timeBuf[16]; snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", dTime / 60, dTime % 60);
    char scoreBuf[16]; snprintf(scoreBuf, sizeof(scoreBuf), "%06d", dScore);
    const char* vals[3] = { buf, timeBuf, scoreBuf };
    for (int i = 0; i < 3; i++) {
        float lx = PANEL_X + 55, rx = PANEL_X + PANEL_W - 55;
        drawText(lx, rowY[i], labels[i], sc, WHITE);
        drawText(rx - textW(vals[i], sc), rowY[i], vals[i], sc, GOLD);
        if (i < 2) dashedLine(lx, rx, rowY[i] + 26, PANEL_DARK);
    }

    // ---- buttons ----
    button(BTN_NEXT_X, BTN_Y, BTN_W, BTN_H, "NEXT LEVEL", BTN_GOLD, hoverBtn == 0);
    button(BTN_REPLAY_X, BTN_Y, BTN_W, BTN_H, "REPLAY", BTN_GREEN, hoverBtn == 1);

    // ---- hint (PRESS 2 blinks) ----
    const char* h1 = "PRESS 2", *h2 = " FOR LEVEL 2 - R TO REPLAY";
    int hs = 2, hx = PANEL_X + (PANEL_W - (textW(h1, hs) + textW(h2, hs))) / 2;
    C blink = (sinf(winT * 5.f) > 0) ? GOLD : C{ GOLD.r * 0.45f, GOLD.g * 0.45f, GOLD.b * 0.45f };
    drawText(hx, PANEL_Y + PANEL_H - 24, h1, hs, blink);
    drawText(hx + textW(h1, hs), PANEL_Y + PANEL_H - 24, h2, hs, WHITE);

    // ======================= 3D star pass =======================
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0); glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    float amb[4] = { 0.72f, 0.66f, 0.44f, 1 }, dif[4] = { 1.f, 0.95f, 0.72f, 1 };
    float lspec[4] = { 1.f, 1.f, 0.9f, 1 };                      // shiny specular streak
    float lpos[4] = { -0.45f, 0.8f, 1.0f, 0 };                   // directional, upper-left
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
    glLightfv(GL_LIGHT0, GL_POSITION, lpos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lspec);
    float mspec[4] = { 0.9f, 0.9f, 0.55f, 1 }, memis[4] = { 0.10f, 0.08f, 0.f, 1 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mspec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 64.f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, memis);         // slight self-glow

    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    // fov s.t. tan(fov/2)*504 = 252 -> 1 world unit = 1 virtual px at z=0
    gluPerspective(2 * atan(252.0 / 504.0) * 180.0 / 3.14159265, VW / (double)VH, 50, 2000);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    glTranslatef(0, 0, -504);                                  // camera distance -> z0 plane = virtual coords

    for (int i = 0; i < 3; i++) {
        float sx = PANEL_X + PANEL_W / 2 + (i - 1) * 120.f;      // 360 / 480 / 600
        float sy = STAR_CY;
        bool earned = i < stars_earned;
        float lt = winT - STAR_DELAY * i;
        if (!earned) {                                         // grey, static silhouette
            glDisable(GL_LIGHTING);
            glLoadIdentity(); glTranslatef(0, 0, -504);
            glTranslatef(sx - VW / 2, -(sy - VH / 2), 0);
            glScalef(STAR_R, STAR_R, STAR_R);
            glColor3f(GREY.r, GREY.g, GREY.b);
            drawStarMesh();
            glEnable(GL_LIGHTING);
            continue;
        }
        if (lt <= 0) continue;                                 // hidden until its stagger
        float pop = easeOutBack(clamp01(lt / STAR_POP_T));
        float phase = winT * 3.f + i * 0.6f;
        float bounce = fabsf(sinf(phase));
        float squash = 0.9f + 0.1f * bounce;
        glLoadIdentity(); glTranslatef(0, 0, -504);
        glTranslatef(sx - VW / 2, -(sy - VH / 2) - bounce * STAR_BOUNCE_H * STAR_R, 0);
        glScalef(STAR_R * pop, STAR_R * pop * squash, STAR_R * pop);
        glRotatef(fmodf(winT * STAR_SPIN, 360.f), 0, 1, 0);
        glRotatef(STAR_TILT, 1, 0, 0);
        glColor3f(1, 1, 1);
        drawStarMesh();
    }

    // ---- restore 2D state ----
    glDisable(GL_LIGHTING); glDisable(GL_LIGHT0);
    glDisable(GL_COLOR_MATERIAL); glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glOrtho(0, VW, VH, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
}

} // namespace win
