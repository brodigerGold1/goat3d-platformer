#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

// Gameplay data and rules without OpenGL dependencies

// ---------- Platforms, collectibles, and exit lights ----------
struct Platform {
    float x, y, w, h, r, g, b;
};

struct Collectible {
    float x, y, w, h, r, g, b;
    float bobOffset = 0;
};

struct Light {
    float x, y, w, h, r, g, b;
    float pulse = 0;
};

// ---------- Game screens ----------
enum AppState {
    STATE_LEVEL,
    STATE_WIN,
    STATE_COMPLETE
};

// ---------- Level layout and spawn position ----------
struct LevelData {
    std::vector<Platform> platforms;
    std::vector<Collectible> coins;
    std::vector<Light> lights;
    float spawnX, spawnY;
};

// ---------- Level definitions ----------
// Object values follow x, y, width, height, red, green, blue
inline std::vector<LevelData> makeLevels() {
    return {
        // Level 1
        {
            // Platforms and boundary walls
            {
                {-10, -5, 20, 1, .3f, .5f, .3f},
                {-7, -2, 3, .3f, .4f, .4f, .6f},
                {-2, -1, 3, .3f, .4f, .6f, .4f},
                {3, 0, 1.5f, .3f, .6f, .4f, .4f},
                {-8, 1.5f, 3, .3f, .5f, .5f, .7f},
                {1, 2, 1, .3f, .7f, .5f, .5f},
                {6, 3, 4, .3f, .3f, .7f, .7f},
                {-10, -5, .5f, 10, .5f, .3f, .2f},
                {10, -5, .5f, 10, .5f, .3f, .2f},

                // Connecting platforms
                {-4, .5f, 2, .3f, .4f, .6f, .4f},
                {-1, 1.2f, 2, .3f, .4f, .6f, .4f},
                {3.5f, 1.7f, 2, .3f, .4f, .6f, .4f}
            },

            // Coins
            {
                {-6.9f, 1.85f, .3f, .3f, 1, 1, 0}
            },

            // Exit lights
            {
                {9.1f, 3.35f, .4f, .6f, 1, 1, .5f}
            },

            // Spawn position
            -8.5f, -4
        },

        // Level 2
        {
            // Platforms and boundary walls
            {
                {-10, -5, 5, 1, .2f, .4f, .2f},
                {-2, -5, 3, 1, .2f, .4f, .2f},
                {4, -5, 6, 1, .2f, .4f, .2f},
                {-8, -2.5f, 2.5f, .3f, .5f, .3f, .7f},
                {-4, -1.5f, 2.5f, .3f, .5f, .3f, .7f},
                {0, -.5f, 2.5f, .3f, .5f, .3f, .7f},
                {-7, 1, .5f, .3f, .5f, .3f, .7f},
                {-2, 2, 2.5f, .3f, .5f, .3f, .7f},
                {-9, 3.5f, .5f, .3f, .5f, .3f, .7f},
                {6, 2, 4, .3f, .5f, .3f, .7f},
                {6, 0, 3, .3f, .5f, .3f, .7f},
                {-10, -5, .5f, 10, .5f, .3f, .2f},
                {10, -5, .5f, 10, .5f, .3f, .2f},

                // Connecting platforms for the coin and exit routes
                {-5.2f, -1.2f, 1.2f, .3f, .5f, .3f, .7f},
                {-4.5f, 1.4f, 1.3f, .3f, .5f, .3f, .7f},
                {2.5f, 1, 2, .3f, .5f, .3f, .7f}
            },

            // Coins
            {
                {-8.9f, 3.85f, .3f, .3f, 1, 1, 0}
            },

            // Exit lights
            {
                {9.2f, 2.35f, .4f, .6f, 1, 1, .5f}
            },

            // Spawn position
            -8.5f, -4
        }
    };
}

// ---------- Player position, dimensions, and movement state ----------
// X marks the horizontal center and Y marks the feet
struct Player {
    float x = 0, y = 0, vy = 0;
    float width = .6f, height = .8f;
    bool grounded = false;
};

// ---------- Held movement and jump keys ----------
struct InputState {
    bool left = false, right = false, jumpHeld = false;
};

// ---------- Gameplay controller ----------
class Game {
public:
    // Level definitions and active level objects
    std::vector<LevelData> levels = makeLevels();
    std::vector<Platform> platforms;
    std::vector<Collectible> coins;
    std::vector<Light> lights;

    Player player;
    InputState input;
    AppState state = STATE_LEVEL;

    // Level progress and timers
    int currentLevel = 1, collected = 0, deaths = 0;
    double elapsed = 0, animationTime = 0;
    float jumpBuffer = 0, coyoteTime = 0;

    Game() {
        loadLevel(1);
    }

    // Check whether two rectangles overlap
    static bool overlaps(float ax, float ay, float aw, float ah,
                         float bx, float by, float bw, float bh) {
        return ax < bx + bw && ax + aw > bx &&
               ay < by + bh && ay + ah > by;
    }

    // Clear held keys and any buffered jump
    void clearInput() {
        input = {};
        jumpBuffer = 0;
    }

    // Load a level and reset its progress
    bool loadLevel(int level) {
        if (level < 1 || level > static_cast<int>(levels.size()))
            return false;

        currentLevel = level;
        const auto& data = levels[level - 1];

        platforms = data.platforms;
        coins = data.coins;
        lights = data.lights;

        collected = deaths = 0;
        elapsed = animationTime = 0;
        state = STATE_LEVEL;

        respawn();
        return true;
    }

    // Reset the player while preserving coins, elapsed time, and fall count
    void respawn() {
        player = {};
        player.x = levels[currentLevel - 1].spawnX;
        player.y = levels[currentLevel - 1].spawnY;
        coyoteTime = 0;
        clearInput();
    }

    // Buffer a new jump press for a short time before landing
    void pressJump() {
        if (state == STATE_LEVEL && !input.jumpHeld)
            jumpBuffer = .12f;

        input.jumpHeld = true;
    }

    void releaseJump() {
        input.jumpHeld = false;
    }

    // Advance after a win or open the final completion screen
    void nextLevel() {
        if (state != STATE_WIN)
            return;

        if (currentLevel < static_cast<int>(levels.size()))
            loadLevel(currentLevel + 1);
        else {
            state = STATE_COMPLETE;
            clearInput();
        }
    }

    // Award coin and time points, subtract fall penalties, and prevent negative scores
    int score() const {
        return std::max(
            0,
            collected * 100 +
            std::max(0, 120 - static_cast<int>(elapsed)) * 10 -
            deaths * 25
        );
    }

    // Completion star plus bonuses for no falls and finishing within 75 seconds
    int stars() const {
        return 1 + (deaths == 0 ? 1 : 0) + (elapsed <= 75 ? 1 : 0);
    }

    // ---------- Gameplay updates ----------
    void update(float dt) {
        if (state != STATE_LEVEL || dt <= 0)
            return;

        elapsed += dt;
        animationTime += dt;

        // Allow buffered jumps and a short grace period after leaving a platform
        jumpBuffer = std::max(0.f, jumpBuffer - dt);
        coyoteTime = player.grounded ? .10f : std::max(0.f, coyoteTime - dt);

        if (jumpBuffer > 0 && coyoteTime > 0) {
            player.vy = 18.f;
            player.grounded = false;
            jumpBuffer = coyoteTime = 0;
        }

        // Move horizontally, apply gravity, and resolve vertical movement
        const float dx =
            (static_cast<int>(input.right) - static_cast<int>(input.left)) * 4.8f * dt;

        moveHorizontal(dx);
        player.vy = std::max(-35.f, player.vy - 54.f * dt);
        moveVertical(player.vy * dt);

        // Count the fall and respawn without resetting level progress
        if (player.y < -7) {
            ++deaths;
            respawn();
            return;
        }

        // Collect and remove coins that overlap the player
        const float left = player.x - player.width / 2;

        coins.erase(
            std::remove_if(coins.begin(), coins.end(), [&](const Collectible& c) {
                if (!overlaps(
                    left, player.y, player.width, player.height,
                    c.x, c.y, c.w, c.h
                ))
                    return false;

                ++collected;
                return true;
            }),
            coins.end()
        );

        // Animate the remaining coins and exit lights
        for (auto& c : coins)
            c.bobOffset = std::sin(static_cast<float>(animationTime) * 2 + c.x) * .05f;

        for (auto& l : lights)
            l.pulse = static_cast<float>(animationTime) * 4;

        // Unlock the exit after all coins have been collected
        if (coins.empty()) {
            for (const auto& l : lights) {
                if (overlaps(
                    left, player.y, player.width, player.height,
                    l.x, l.y, l.w, l.h
                )) {
                    state = STATE_WIN;
                    clearInput();
                    return;
                }
            }
        }
    }

private:
    // ---------- Horizontal collision handling ----------
    void moveHorizontal(float dx) {
        const float half = player.width / 2;
        const float oldLeft = player.x - half, oldRight = player.x + half;
        float target = player.x + dx;

        for (const auto& p : platforms) {
            if (player.y + player.height <= p.y || player.y >= p.y + p.h)
                continue;

            if (dx > 0 && oldRight <= p.x && target + half >= p.x)
                target = std::min(target, p.x - half);
            else if (dx < 0 && oldLeft >= p.x + p.w && target - half <= p.x + p.w)
                target = std::max(target, p.x + p.w + half);
        }

        // Keep the player's full width within the scene
        player.x = std::max(-10.f + half, std::min(10.f - half, target));
    }

    // ---------- Vertical collision handling ----------
    void moveVertical(float dy) {
        const float oldBottom = player.y, oldTop = player.y + player.height;
        const float left = player.x - player.width / 2,
                    right = player.x + player.width / 2;

        float target = player.y + dy;
        bool hit = false;
        player.grounded = false;

        for (const auto& p : platforms) {
            if (right <= p.x || left >= p.x + p.w)
                continue;

            // Land on a platform while falling
            if (dy <= 0 && oldBottom >= p.y + p.h && target <= p.y + p.h) {
                target = std::max(target, p.y + p.h);
                hit = true;
                player.grounded = true;
            }
            // Stop upward movement when hitting a platform's underside
            else if (dy > 0 && oldTop <= p.y && target + player.height >= p.y) {
                target = std::min(target, p.y - player.height);
                hit = true;
            }
        }

        player.y = target;

        if (hit)
            player.vy = 0;
    }
};