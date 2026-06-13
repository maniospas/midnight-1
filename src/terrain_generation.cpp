#pragma once
#include <raylib.h>
#include <raymath.h>
#include <cmath>
#include "terrain.cpp"

static const int GRID_SIZE = 196;
int NOISE_SEED = 0;

static void DrawConnector(float x1, float y1, float x2, float y2, bool active) {
    if(!active) return;
    Color c = active ? Fade(GREEN, 0.8f) : Fade(GRAY, 0.4f);
    DrawLineBezier({ x1, y1 }, { x2, y2 }, 3.0f, c);
}


// Draw dashed line between two world-space points
static void DrawDashedLine(float x1, float y1, float x2, float y2, Color c) {
    const float dash = 10.0f;
    const float gap  = 6.0f;
    float dx = x2 - x1;
    float dy = y2 - y1;
    float len = sqrtf(dx*dx + dy*dy);
    if (len < 1.0f) return;
    float vx = dx / len;
    float vy = dy / len;
    float pos = 0.0f;
    while (pos < len) {
        float sx = x1 + vx * pos;
        float sy = y1 + vy * pos;
        float ex = x1 + vx * fminf(pos + dash, len);
        float ey = y1 + vy * fminf(pos + dash, len);
        DrawLineEx({sx, sy}, {ex, ey}, 6.0f, c);
        pos += dash + gap;
    }
}

static float HashNoise2D(int x, int y) {
    unsigned int n = (unsigned int)(x * 374761393u + y * 668265263u + NOISE_SEED * 374761393u);
    n = (n ^ (n >> 13)) * 1274126177u;
    return (float)(n & 0x00FFFFFF) / (float)0x01000000; // 0.0 - 1.0
}

static float SmoothNoise(float x, float y, float scale) {
    float fx = x / scale;
    float fy = y / scale;
    int ix = (int)floorf(fx);
    int iy = (int)floorf(fy);
    float tx = fx - ix;
    float ty = fy - iy;
    float v00 = HashNoise2D(ix, iy);
    float v10 = HashNoise2D(ix+1, iy);
    float v01 = HashNoise2D(ix, iy+1);
    float v11 = HashNoise2D(ix+1, iy+1);
    float vx0 = v00*(1-tx) + v10*tx;
    float vx1 = v01*(1-tx) + v11*tx;
    return vx0*(1-ty) + vx1*ty;
}

static float FractalNoise(float x, float y, float baseScale) {
    float v  = SmoothNoise(x, y, baseScale)       * 0.55f;
    v       += SmoothNoise(x, y, baseScale * 0.5) * 0.28f;
    v       += SmoothNoise(x, y, baseScale * 0.25)* 0.14f;
    return v;
}

static float ForestNoise(int x, int y) {
    float n1 = FractalNoise(x + 12000, y - 9000, 150.0f);
    float n2 = FractalNoise(x + 5000,  y + 7000, 80.0f);
    float n3 = FractalNoise(x + 8000,  y + 13000, 100.0f);
    float n4 = FractalNoise(x + 3000,  y + 2000, 100.0f);
    return n1 * 0.6f + n2 * 0.4f + n3 * 0.4f - n4*0.3f;
}

// Adjust curve for widening or sharpening noise clusters
static float BiasCurve(float v, float bias) {
    // bias < 1 makes it fatter, bias > 1 makes it sharper
    return powf(v, bias);
}

static void GenerateGrass(Terrain** terrainGrid) {
    for (int y = 0; y < GRID_SIZE; y++)
        for (int x = 0; x < GRID_SIZE; x++)
            if (GetRandomValue(1, 100) <= 90)
                terrainGrid[y][x] = { &tex::grass, 1.0 };
    else {
        int alt = GetRandomValue(2, 4); // 2,3,4
        switch (alt) {
            case 2: terrainGrid[y][x] = { &tex::grass2, 1.0 }; break;
            case 3: terrainGrid[y][x] = { &tex::grass3, 1.0 }; break;
            case 4: terrainGrid[y][x] = { &tex::grass4, 1.0 }; break;
        }
    }
}

static void GenerateRivers(Terrain** terrainGrid) {
    int NUM_ROADS = 80 * GRID_SIZE * GRID_SIZE / 512 / 512;
    if(GetRandomValue(0,2)) NUM_ROADS /= 2;
    if(GetRandomValue(0,2)) NUM_ROADS /= 2;
    int MAX_LEN   = 600;

    int dirX[4] = { 1, -1, 0, 0 };
    int dirY[4] = { 0, 0, 1, -1 };

    for (int r = 0; r < NUM_ROADS; r++) {
        int x = 0, y = 0, dir = 0;
        switch (GetRandomValue(0,3)) {
            case 0: x = GetRandomValue(0, GRID_SIZE-1); y = 3;             dir = 2; break;
            case 1: x = GetRandomValue(0, GRID_SIZE-1); y = GRID_SIZE-4;   dir = 3; break;
            case 2: y = GetRandomValue(0, GRID_SIZE-1); x = 3;             dir = 0; break;
            case 3: y = GetRandomValue(0, GRID_SIZE-1); x = GRID_SIZE-4;   dir = 1; break;
        }

        for (int i = 0; i < MAX_LEN; i++) {
            if (x <= 2 || y <= 2 || x >= GRID_SIZE-3 || y >= GRID_SIZE-3)
                break;

            // --- MAIN TILE ---
            Terrain &T = terrainGrid[y][x];
            if (!IsDesert(T.texture)) {
                T.texture = &tex::water;
                T.speed = 0.2f;
                T.extra_sight = -0.7f;
            }

            // --- SECOND TILE (perpendicular, width = 2) ---
            int px = 0, py = 0;
            if (dirX[dir] != 0) py = 1;  // horizontal → widen vertically
            else  px = 1;  // vertical → widen horizontally

            int wx = x + px;
            int wy = y + py;

            if (wx > 2 && wy > 2 && wx < GRID_SIZE-3 && wy < GRID_SIZE-3) {
                Terrain &W = terrainGrid[wy][wx];
                if (!IsDesert(W.texture)) {
                    W.texture = &tex::water;
                    W.speed = 0.2f;
                    W.extra_sight = -0.7f;
                }
            }

            // --- RARE TURN ---
            if (GetRandomValue(0,100) < 6) {
                if (dir<2) dir = GetRandomValue(0,1)?2:3;
                else dir = GetRandomValue(0,1)?0:1;
            }

            // --- RARE CROSSROAD (also 2 tiles wide) ---
            if (GetRandomValue(0,100) < 2) {
                int cd = GetRandomValue(0,3);
                int cx = x + dirX[cd];
                int cy = y + dirY[cd];

                if (cx > 2 && cy > 2 && cx < GRID_SIZE-3 && cy < GRID_SIZE-3) {
                    Terrain &C = terrainGrid[cy][cx];
                    if (!IsDesert(C.texture)) {
                        C.texture = &tex::water;
                        C.speed = 0.3f;
                        C.extra_sight = -0.7f;
                    }
                    int bpx = 0, bpy = 0;
                    if(dirX[cd] != 0) bpy = 1;
                    else bpx = 1;
                    if (GetRandomValue(0,1)) { bpx = -bpx; bpy = -bpy; }
                    int bx = cx + bpx;
                    int by = cy + bpy;

                    if (bx > 2 && by > 2 && bx < GRID_SIZE-3 && by < GRID_SIZE-3) {
                        Terrain &B = terrainGrid[by][bx];
                        if (!IsDesert(B.texture)) {
                            B.texture = &tex::water;
                            B.speed = 0.3f;
                            B.extra_sight = -0.7f;
                        }
                    }
                }
            }
            x += dirX[dir];
            y += dirY[dir];
        }
    }
}

static void GenerateSeas(Terrain** terrainGrid) {
    int NUM_SEAS = 3 * GRID_SIZE * GRID_SIZE / 512 / 512;
    if (NUM_SEAS < 1) NUM_SEAS = 1;

    for (int s = 0; s < NUM_SEAS; s++) {
        // Pick a random center for the sea, away from edges
        int cx = GetRandomValue(GRID_SIZE / 6, GRID_SIZE * 5 / 6);
        int cy = GetRandomValue(GRID_SIZE / 6, GRID_SIZE * 5 / 6);

        // Sea is an irregular blob grown via random walk flood-fill
        int seaRadius   = GetRandomValue(GRID_SIZE / 3, GRID_SIZE *2/3);
        int targetCells = seaRadius * seaRadius * 3; // approximate area

        // BFS-style expansion with randomness to get organic shapes
        std::vector<std::pair<int,int>> frontier;
        frontier.push_back({cx, cy});

        int filled = 0;
        while (!frontier.empty() && filled < targetCells) {
            // Pick a random cell from the frontier
            int idx = GetRandomValue(0, (int)frontier.size() - 1);
            auto [x, y] = frontier[idx];
            frontier.erase(frontier.begin() + idx);
            if (x <= 2 || y <= 2 || x >= GRID_SIZE - 3 || y >= GRID_SIZE - 3)
                continue;
            Terrain &T = terrainGrid[y][x];
            if (T.texture == &tex::water) continue;
            if (IsMountain(T.texture)) continue;
            if (IsHill(T.texture)) continue;
            if (IsDesert(T.texture))      continue;
            T.texture     = &tex::water;
            T.speed       = 0.15f;
            T.extra_sight = -0.8f;
            filled++;
            // Add neighbours with a distance-based probability so the
            // blob stays roughly circular but has ragged edges
            int dirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
            for (auto& d : dirs) {
                int nx = x + d[0];
                int ny = y + d[1];
                int dx = nx - cx, dy = ny - cy;
                float dist = sqrtf((float)(dx*dx + dy*dy));
                float prob = 1.0f - (dist / (float)(seaRadius + 1));
                if (prob < 0.2f) prob = 0.2f;
                if (GetRandomValue(0, 100) < (int)(prob * 85))
                    frontier.push_back({nx, ny});
            }
        }

        // scatter islands in places where the seas hes left back stuff
        int numIslands = GetRandomValue(6, 12);
        for (int isle = 0; isle < numIslands; isle++) {
            int ix = GetRandomValue(cx - seaRadius/2, cx + seaRadius/2);
            int iy = GetRandomValue(cy - seaRadius/2, cy + seaRadius/2);
            if (ix < 3 || iy < 3 || ix >= GRID_SIZE - 3 || iy >= GRID_SIZE - 3) continue;
            if (terrainGrid[iy][ix].texture != &tex::water) continue;
            int ir = GetRandomValue(2,8);
            for (int dy = -ir; dy <= ir; dy++) {
                for (int dx = -ir; dx <= ir; dx++) {
                    if(dx*dx+dy*dy>ir*ir) continue;
                    int gx = ix + dx;
                    int gy = iy + dy;
                    if (gx <= 2 || gy <= 2 || gx >= GRID_SIZE-3 || gy >= GRID_SIZE-3) continue;
                    int h = HashNoise2D(gx, gy) * 100;
                    Texture2D* tex = &tex::hill;
                    if (h == 1) tex = &tex::hill2;
                    if (h == 2) tex = &tex::hill3;
                    if (h == 3) tex = &tex::hill4;
                    terrainGrid[iy][ix] = {
                        tex,
                        0.7f,
                        0.5f
                    };
                }
            }
        }
    }
}

static void GenerateHillsAndDesert(Terrain** terrainGrid) {
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {

            float h1 = FractalNoise((float)x, (float)y, 140.0f);
            float h2 = FractalNoise((float)x, (float)y, 60.0f);
            float hillValue = h1 * 0.65f + h2 * 0.35f;
            hillValue = BiasCurve(hillValue, 1.0f);

            float d1 = FractalNoise(x + 8000, y + 8000, 180.0f);
            float d2 = FractalNoise(x + 8000, y + 8000, 90.0f);
            float desertValue = d1 * 0.7f + d2 * 0.3f;
            desertValue = BiasCurve(desertValue, 0.8f);

            if (hillValue > 0.67f) {
                terrainGrid[y][x] = {
                    &tex::mountain,
                    0.4f,
                    1.0f
                };
                continue;
            }

            if (hillValue > 0.62f) {
                terrainGrid[y][x] = {
                    &tex::mountain,
                    0.4f,
                    1.0f
                };
                continue;
            }

            if (hillValue > 0.57f) {
                int h = HashNoise2D(x, y) * 100;
                Texture2D* tex = &tex::hill;
                if (h == 1) tex = &tex::hill2;
                if (h == 2) tex = &tex::hill3;
                if (h == 3) tex = &tex::hill4;

                terrainGrid[y][x] = {
                    tex,
                    0.7f,
                    0.5f
                };
                continue;
            }

            if (desertValue > 0.65f) {
                terrainGrid[y][x] = {
                    &tex::desert,
                    0.8f,
                    -0.7f
                };
                continue;
            }
        }
    }
}
