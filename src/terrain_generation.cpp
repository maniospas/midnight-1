#pragma once
#include <raylib.h>
#include <raymath.h>
#include <cmath>
#include <vector>
#include "terrain.cpp"

static const int GRID_SIZE = 196;
int NOISE_SEED = 0;
#define WATER_SPEED 0.2f

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
    int NUM_ROADS = 40 * GRID_SIZE * GRID_SIZE / 512 / 512;
    if (GetRandomValue(0,2)) NUM_ROADS /= 2;
    if (GetRandomValue(0,2)) NUM_ROADS /= 2;
    int MAX_LEN = 600;

    // 8 directions in clockwise order: 0=E, 1=SE, 2=S, 3=SW, 4=W, 5=NW, 6=N, 7=NE
    int dirX[8] = {  1,  1,  0, -1, -1, -1,  0,  1 };
    int dirY[8] = {  0,  1,  1,  1,  0, -1, -1, -1 };

    auto PaintWater = [&](int wx, int wy, float spd) {
        if (wx <= 2 || wy <= 2 || wx >= GRID_SIZE-3 || wy >= GRID_SIZE-3) return;
        Terrain &T = terrainGrid[wy][wx];
        if (!IsDesert(T.texture) && T.texture != &tex::tree) {
            T.texture     = &tex::water;
            T.speed       = spd;
            T.extra_sight = -0.7f;
        }
    };

    // Paint a 3-tile-wide strip centred on (x,y) moving in direction dir.
    // For diagonals we widen perpendicular to the direction of travel.
    auto PaintStrip = [&](int x, int y, int dir, float spd) {
        PaintWater(x, y, spd);
        int dx = dirX[dir], dy = dirY[dir];
        if (dy == 0) {
            // Pure horizontal (E/W) — widen vertically
            PaintWater(x, y - 1, spd);
            PaintWater(x, y + 1, spd);
        } else if (dx == 0) {
            // Pure vertical (N/S) — widen horizontally
            PaintWater(x - 1, y, spd);
            PaintWater(x + 1, y, spd);
        } else {
            // Diagonal — widen along both perpendicular axes
            // Perpendicular to (dx,dy) is (-dy, dx) and (dy, -dx)
            PaintWater(x - dy, y + dx, spd);
            PaintWater(x + dy, y - dx, spd);
        }
    };

    for (int r = 0; r < NUM_ROADS; r++) {
        int x = 0, y = 0, dir = 0;
        switch (GetRandomValue(0, 7)) {
            case 0: x = GetRandomValue(0, GRID_SIZE-1); y = 3;           dir = 2; break; // top    → S
            case 1: x = GetRandomValue(0, GRID_SIZE-1); y = GRID_SIZE-4; dir = 6; break; // bottom → N
            case 2: y = GetRandomValue(0, GRID_SIZE-1); x = 3;           dir = 0; break; // left   → E
            case 3: y = GetRandomValue(0, GRID_SIZE-1); x = GRID_SIZE-4; dir = 4; break; // right  → W
            case 4: x = 3;           y = 3;           dir = 1; break; // TL → SE
            case 5: x = GRID_SIZE-4; y = 3;           dir = 3; break; // TR → SW
            case 6: x = 3;           y = GRID_SIZE-4; dir = 7; break; // BL → NE
            case 7: x = GRID_SIZE-4; y = GRID_SIZE-4; dir = 5; break; // BR → NW
        }

        float drift      = 0.0f;
        float driftSpeed = (GetRandomValue(0,100) / 100.0f) * 0.06f + 0.02f;
        int   driftDir   = GetRandomValue(0,1) ? 1 : -1;

        for (int i = 0; i < MAX_LEN; i++) {
            if (x <= 2 || y <= 2 || x >= GRID_SIZE-3 || y >= GRID_SIZE-3) break;

            PaintStrip(x, y, dir, WATER_SPEED);

            // Meander drift — perpendicular to direction of travel
            drift += driftSpeed * driftDir;
            if (drift > 1.0f || drift < -1.0f) {
                int sdx = 0, sdy = 0;
                int dx = dirX[dir], dy = dirY[dir];
                if (dy == 0) {
                    // Horizontal: drift N/S
                    sdy = (drift > 0) ? 1 : -1;
                } else if (dx == 0) {
                    // Vertical: drift E/W
                    sdx = (drift > 0) ? 1 : -1;
                } else {
                    // Diagonal: drift along perpendicular (-dy, dx) or (dy, -dx)
                    if (drift > 0) { sdx = -dy; sdy =  dx; }
                    else           { sdx =  dy; sdy = -dx; }
                }
                x += sdx;
                y += sdy;
                drift -= (drift > 0) ? 1.0f : -1.0f;
                if (GetRandomValue(0,100) < 25) driftDir = -driftDir;
            }

            // Rare hard turn — pick any of the 8 directions except current
            if (GetRandomValue(0,100) < 4) {
                int turn = GetRandomValue(0,1) ? 1 : -1;
                dir = (dir + turn + 8) % 8;
                driftDir = GetRandomValue(0,1) ? 1 : -1;
                drift    = 0.0f;
            }

            // Rare oxbow widening
            if (GetRandomValue(0,100) < 2) {
                int dx = dirX[dir], dy = dirY[dir];
                if (dy == 0) {
                    PaintWater(x, y - 2, WATER_SPEED);
                    PaintWater(x, y + 2, WATER_SPEED);
                } else if (dx == 0) {
                    PaintWater(x - 2, y, WATER_SPEED);
                    PaintWater(x + 2, y, WATER_SPEED);
                } else {
                    PaintWater(x - 2*dy, y + 2*dx, WATER_SPEED);
                    PaintWater(x + 2*dy, y - 2*dx, WATER_SPEED);
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
            T.speed       = WATER_SPEED;
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
        int numIslands = GetRandomValue(0, 3);
        for (int isle = 0; isle < numIslands; isle++) {
            int ix = GetRandomValue(cx - seaRadius/4, cx + seaRadius/4);
            int iy = GetRandomValue(cy - seaRadius/4, cy + seaRadius/4);
            if (ix < 6 || iy < 6 || ix >= GRID_SIZE - 6 || iy >= GRID_SIZE - 6) continue;
            if (terrainGrid[iy][ix].texture != &tex::water) continue;
            int ir = GetRandomValue(3,6);
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
                    terrainGrid[gy][gx] = {
                        tex,
                        0.7f,
                        0.5f
                    };
                }
            }
            terrainGrid[iy][ix] = { &tex::treasure, 1.f, 0.f };
            ir += 2;
            for (int dy = -ir; dy <= ir; dy++) {
                for (int dx = -ir; dx <= ir; dx++) {
                    if(dx*dx+dy*dy>ir*ir) continue;
                    int gx = ix + dx;
                    int gy = iy + dy;
                    if(terrainGrid[gy][gx].texture!=&tex::water) continue;
                    terrainGrid[gy][gx] = { &tex::grass, 1.0 };
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

            if (hillValue > 0.65f) {
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
