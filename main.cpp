#include <raylib.h>
#include <raymath.h>
#include <cmath>
#include <iostream>
#include <bitset>
#include <cstring>
#include <vector>
#include "src/texture.cpp"
#include "src/ranks.cpp"
#include "src/sound.cpp"
#include "src/font.cpp"
#include "src/tech.cpp"
#include "src/terrain.cpp"
#include "src/faction.cpp"
#include "src/units.cpp"
#include "src/terrain_generation.cpp"

static const int MAX_DECORATORS = 1000000;
static const int TILE_SIZE = 128;
static const float CAMERA_ZOOM = 0.8f;
static const float movement_speed_multiplier = 0.5f;
static const int SHOW_MINIMAP = 1;
static const float DESC_FONT_SIZE = 28;

// difficulty controls
static const float CAMP_SPAWN_RATE = 2.f; //
static const float ANIMAL_SPAWN_RATE = 0.1f; // expected spawns per second
static const float CAPTURE_RATE = 1.0f; // also heals espers and partial captures
static const float OVER_CAP_REGEN_RATE = 0.3f; // 1.f is normal restoration from units being captured, this is lower if we have over-saturated industry

enum class MovementMode {Tight,Scattered,Explore};

#define is_mecha(u) (u.texture==&tex::tank || u.texture==&tex::van || u.texture==&tex::railgun || u.texture==&tex::roomba || u.texture==&tex::hovercraft)

void DrawUnitStatCircle(Unit* unit, int px, int py) {
    const float RADIUS    = 55.0f;
    const float INNER     = RADIUS * 0.08f;
    const int   FONT_SIZE = 24;
    const int   RINGS     = 4;
    px += 40;
    float cx = px + RADIUS;
    float cy = py + RADIUS;
    float angles[4] = {
        -45.0f * DEG2RAD,
        45.0f * DEG2RAD,
        135.0f * DEG2RAD,
        225.0f * DEG2RAD,
    };
    struct { const char* label; float val; float max; } stats[4] = {
        { "speed",  unit->speed,                      15.0f  },
        { "combat", unit->damage * unit->attack_rate, 14.0f  },
        { "health", (float)unit->max_health,          50.0f  },
        { "sight",  unit->range / 2.0f,                8.0f  },
    };
    for (int ring = 1; ring <= RINGS; ring++) {
        float r = RADIUS * ((float)ring / RINGS);
        for (int i = 0; i < 4; i++) {
            int j = (i + 1) % 4;
            Vector2 p1 = { cx + cosf(angles[i]) * r, cy + sinf(angles[i]) * r };
            Vector2 p2 = { cx + cosf(angles[j]) * r, cy + sinf(angles[j]) * r };
            DrawLineV(p1, p2, Fade(WHITE, ring == RINGS ? 0.30f : 0.18f));
        }
    }
    for (int i = 0; i < 4; i++) {
        Vector2 tip = { cx + cosf(angles[i]) * RADIUS, cy + sinf(angles[i]) * RADIUS };
        DrawLineV({ cx, cy }, tip, Fade(WHITE, 0.80f));
    }
    Vector2 pts[4];
    for (int i = 0; i < 4; i++) {
        float t = fminf(stats[i].val / stats[i].max, 1.0f);
        float r = INNER + t * (RADIUS - INNER);
        pts[i] = { cx + cosf(angles[i]) * r, cy + sinf(angles[i]) * r };
    }
    auto faction_color = unit->faction?unit->faction->color:LIGHTGRAY;
    faction_color = ColorBrightness(faction_color, -0.5f);
    DrawTriangle(pts[0], pts[3], pts[1], Fade(faction_color, 0.65f));
    DrawTriangle(pts[1], pts[3], pts[2], Fade(faction_color, 0.65f));
    for (int i = 0; i < 4; i++) DrawLineV(pts[i], pts[(i + 1) % 4], ColorAlpha(faction_color, 0.90f));
    for (int i = 0; i < 4; i++) DrawCircleV(pts[i], 3.0f, WHITE);
    for (int i = 0; i < 4; i++) {
        float lx = cx + cosf(angles[i]) * (RADIUS + 10.0f);
        float ly = cy + sinf(angles[i]) * (RADIUS - 8.0f);
        int tw = MeasureText(stats[i].label, FONT_SIZE);
        int drawX = (int)(lx - tw / 2.f + copysignf(tw / 2.f, cosf(angles[i])));
        DrawTextSmallOutlined(stats[i].label, drawX, (int)(ly - 7.f), FONT_SIZE, Fade(WHITE, 1.f));
    }
}



static const char* veteran_name = "Veteran";
static const char* hero_name = "Hero";

struct Decorator {
    Texture2D* texture;
    float x,y;
    float size;
};

const int GAME_W = 2560;
const int GAME_H = 1600;


int main() {
    PrefMask unlocked_preferences = 0;
    const double AI_ELO = 1500;
    double player_rating = 1200;
    {
        FILE* f = fopen("midnight-save.dat", "rb");
        if(f) {
            if(fread(&unlocked_preferences, sizeof(unlocked_preferences), 1, f)!=1) unlocked_preferences = 0;
            if(fread(&player_rating, sizeof(player_rating), 1, f)!=1) player_rating = 0;
            fclose(f);
        }
    }

    SetTraceLogLevel(LOG_NONE); // disable raylib logs
    int w = GetMonitorWidth(0);
    int h = GetMonitorHeight(0);

    #ifdef _WIN32
        SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    #else
        SetConfigFlags(FLAG_FULLSCREEN_MODE);
    #endif
    InitWindow(w, h, "MIDNIGHT - next morn");
    MaximizeWindow();
    InitAudioDevice();
    SetRandomSeed((unsigned)time(NULL));
    load_fonts();
    load();
    sound::bg.looping = true;
    PlayMusicStream(sound::bg);
    SetTargetFPS(60);

    int num_units = 0;
    int max_factions = 7; // can never be less than 3 if we include the player, unclaimed, and wild - can also not include the last Wild faction
    static const float GAME_DURATION = 17.0f * 60.0f * GRID_SIZE/196; // 17 minutes for 196-sized grid
    float game_time = 0.f;

    // load shaders
    Shader unitShader = LoadShader(0, "data/unit_tint.fs");
    int alphaThresholdLoc = GetShaderLocation(unitShader, "alphaThreshold");
    float alphaThreshold = 0.5f;
    SetShaderValue(unitShader, alphaThresholdLoc, &alphaThreshold, SHADER_UNIFORM_FLOAT);
    int factionColorLoc = GetShaderLocation(unitShader, "factionColor");
    Shader waterShader = LoadShader(0, "data/water.fs");
    int waterTimeLoc = GetShaderLocation(waterShader, "time");

    // prepare buttons
    MovementMode currentMovementMode = MovementMode::Tight;
    bool showTechTree = false;
    bool showHelp = false;
    Rectangle techBtn = {GetScreenWidth() - 260.0f, GetScreenHeight() - 95.0f, 240.0f, 90.0f};
    Rectangle helpBtn = {GetScreenWidth() - 260.0f,GetScreenHeight() - 95.0f-65.f,240.0f,60.0f};
    Rectangle fortBtn = {GetScreenWidth() - 260.0f, GetScreenHeight() - 95.0f-65.f-95.f, 240.0f, 90.0f};
    Rectangle trenchBtn = {GetScreenWidth() - 260.0f, GetScreenHeight() - 95.0f-65.f-95.f-65.f, 240.0f, 60.0f};
    Rectangle turtleBtn = {GetScreenWidth() - 260.0f, GetScreenHeight() - 95.0f-65.f-95.f-65.f-65.f, 240.0f, 60.0f};
    Rectangle optionsButton = {GetScreenWidth() - 260.0f, 10.f, 240.0f, 60.0f};

    static float fort_creation_px = 0;
    static float fort_creation_py = 0;
    static int fort_creation_num = 0;
    static int trench_creation_num = 0;
    static int turtle_creation_num = 0;
    static float fort_creation_total_health = 0;
    static bool fort_creation_has_nearby = false;

    // preallocate stuff
    Terrain* terrainBlock = (Terrain*)malloc(GRID_SIZE * GRID_SIZE * sizeof(Terrain));
    Terrain** terrainGrid = (Terrain**)malloc(GRID_SIZE * sizeof(Terrain*));
    for (int y = 0; y < GRID_SIZE; y++)
        terrainGrid[y] = &terrainBlock[y * GRID_SIZE];
    static Unit units[MAX_UNITS];
    static Decorator decorators[MAX_DECORATORS];
    static Faction factions[11] = {
        { ColorBrightness(BLUE, 0.25), "Player", 0 },
        { GRAY, "Unclaimed", 0},
        { WHITE, "Wild", 0},
        { RED,       "AI", 0 },
        { GREEN,     "AI", 0 },
        { YELLOW,    "AI", 0 },
        { PURPLE,    "AI", 0 },
        { ORANGE,    "AI", 0 },
        { PINK,      "AI", 0 },
        { BEIGE,     "AI", 0 },
    };
    Faction* ANIMAL_FACTION = &factions[2];
    int player_preferred_start[2] = {
        PREFERENCE_RAILGUN,
        PREFERENCE_RAILGUN
    };
    auto RevealUnitToAllFactions = [&](int unitIndex) {
        for(int fi = 0; fi < max_factions; fi++)
            factions[fi].visible_knowledge.set(unitIndex);
    };

    // main loop
    int main_menu_transition_mode = 0; // don't animate first entry (-1 from next menu, -2 from end game)
    int new_game_transition_mode = -1;
    int game_over_transition_mode = 0;
    int reward_transition_mode = -1;
    float main_menu_progress = 0;
    float new_game_progress = 0;
    float game_over_progress = 0;
    float reward_progress = 0;

    MAIN_MENU:
    while (true) {
        UpdateMusicStream(sound::bg);
        if(main_menu_transition_mode)
            main_menu_progress += GetFrameTime()*5.f;
        if(main_menu_progress>1.f) {
            main_menu_progress = 0.f;
            if (main_menu_transition_mode==-2) {main_menu_transition_mode=0;}
            else if (main_menu_transition_mode==-1) {main_menu_transition_mode=0;}
            else if (main_menu_transition_mode==1) {
                new_game_transition_mode = -1;
                main_menu_transition_mode = -1;
                showTechTree = false;
                showHelp = false;
                goto NEW_GAME;
            }
            else if (main_menu_transition_mode==2) {
                main_menu_transition_mode = -1;
                free(terrainBlock);
                free(terrainGrid);
                unload();
                return 0;
            }
        }

        float baseY = (float)GetScreenHeight()/2 - 500;
        float baseX = (float)GetScreenWidth()/2;
        if(main_menu_transition_mode==-2) baseX += (1.0f-main_menu_progress)*(1.0f-main_menu_progress)*GetScreenWidth();
        else if(main_menu_transition_mode==-1) baseX -= (1.0f-main_menu_progress)*(1.0f-main_menu_progress)*GetScreenWidth();
        else if(main_menu_transition_mode==2) baseX -= main_menu_progress*main_menu_progress*GetScreenWidth();
        else baseX -= main_menu_progress*main_menu_progress*GetScreenWidth();
        const Rectangle btnStart = {GetScreenWidth()-400.f, GetScreenHeight()-100.f,400, 80};
        const Rectangle btnQuit = {20, GetScreenHeight()-100.f,300, 80};
        BeginDrawing();
        ClearBackground(BLACK);

        DrawTexturePro(
            tex::sun,
            Rectangle{0,0,(float)tex::sun.width,(float)tex::sun.height},
            Rectangle{baseX-256, baseY+100, 512, 256},
            {0,0}, 0, WHITE);
        DrawText("MIDNIGHT", baseX - 300, baseY+400, 128, WHITE);
        DrawText("next", baseX +260, baseY+400, 64, WHITE);
        DrawText("morn", baseX +260, baseY+450, 64, WHITE);
        Vector2 mouse = GetMousePosition();
        bool hoverStart = !main_menu_transition_mode && CheckCollisionPointRec(mouse, btnStart);
        DrawText("New expedition", btnStart.x, btnStart.y + 8, 58, hoverStart ? WHITE : GRAY);
        {
            int rank_index = 0;
            double rank_progress = 0;
            if(player_rating<1200) player_rating = 1200;
            if(player_rating>2200) player_rating = 2200;
            for (int i=0;i<4;i++)
                if (player_rating>=ranks[i].min && player_rating<ranks[i].max) {
                    rank_index = i;
                    break;
                }
            rank_progress = (player_rating - ranks[rank_index].min) / (ranks[rank_index].max - ranks[rank_index].min);
            if (rank_progress < 0) rank_progress = 0;
            if (rank_progress > 1) rank_progress = 1;
            float size = 64.f;
            float radius = size*0.65f;
            float end_angle = (float)(rank_progress * 360.0) - 90;
            float cx = btnStart.x-64;//baseX - 250 + 32;
            float cy = btnStart.y+32;//baseY + 650;
            DrawCircle(cx, cy, radius, BLACK);
            DrawRing({cx, cy}, radius - 8, radius, -90, end_angle, 64, ranks[rank_index].color);
            DrawRing({cx, cy}, radius - 8, radius, end_angle, 360-90, 64, Fade(ranks[rank_index].color, 0.4f));
            DrawTexturePro(
                *ranks[rank_index].tex,
                {0,0,(float)ranks[rank_index].tex->width,(float)ranks[rank_index].tex->height}, {cx - size/2, cy - size/2, size, size}, {0,0}, 0, ranks[rank_index].color
            );
            Rectangle circleRect = { cx - radius, cy - radius, radius * 2, radius * 2 };
            if (CheckCollisionPointRec(mouse, circleRect)) {
                const char* rankName = ranks[rank_index].name;
                int fontSize = 24;
                int textWidth = MeasureText(rankName, fontSize);
                DrawText(rankName, cx - textWidth/2, cy - radius - 30, fontSize, ranks[rank_index].color);
            }
        }
        bool hoverQuit = !main_menu_transition_mode && CheckCollisionPointRec(mouse, btnQuit);
        DrawText("Quit", btnQuit.x, btnQuit.y + 8, 58, hoverQuit ? RED : GRAY);
        EndDrawing();
        if (hoverStart && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            main_menu_transition_mode = 1;
            PlaySound(sound::select);
        }
        if ((hoverQuit && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_ESCAPE)) {
            if(main_menu_transition_mode && main_menu_transition_mode!=2) {
                main_menu_transition_mode = -1;
                main_menu_progress = 1.f-main_menu_progress;
            }
            else main_menu_transition_mode = 2;
            PlaySound(sound::select);
        }
    }

    NEW_GAME:
    max_factions = GetRandomValue(6,11);
    NOISE_SEED = GetRandomValue(1, 1'000'000);
    GenerateGrass(terrainGrid);
    GenerateHillsAndDesert(terrainGrid);
    GenerateRivers(terrainGrid);
    GenerateSeas(terrainGrid);

    // minimap (used only for new game)
    static const int MINIMAP_SCALE = 2;
    static RenderTexture2D minimap = LoadRenderTexture(GRID_SIZE * MINIMAP_SCALE, GRID_SIZE * MINIMAP_SCALE);
    BeginTextureMode(minimap);
    ClearBackground(BLACK);
    for (int y = 0; y < GRID_SIZE; y++)
        for (int x = 0; x < GRID_SIZE; x++)
            DrawRectangle(x * MINIMAP_SCALE, y * MINIMAP_SCALE, MINIMAP_SCALE, MINIMAP_SCALE, ColorForTile(terrainGrid[y][x].texture));
    EndTextureMode(); // minimap

    while (true) {
        UpdateMusicStream(sound::bg);
        if(new_game_transition_mode)
            new_game_progress += GetFrameTime()*5.f;
        if(new_game_progress>1.f) {
            new_game_progress = 0.f;
            if (new_game_transition_mode==-1) {
                new_game_transition_mode=0;
            }
            else if (new_game_transition_mode==1) {
                new_game_transition_mode = -1;
                showTechTree = false;
                showHelp = false;
                goto START_GAME;
            }
            else if (new_game_transition_mode==2) {
                new_game_transition_mode = -1;
                goto MAIN_MENU;
            }
        }
        float baseY = (float)GetScreenHeight()/2 - 600;
        float baseX = (float)GetScreenWidth()/2;
        if(new_game_transition_mode==-1) baseX += (1.0f-new_game_progress)*(1.0f-new_game_progress)*GetScreenWidth();
        else if(new_game_transition_mode==2) baseX += new_game_progress*new_game_progress*GetScreenWidth();
        else baseX -= new_game_progress*new_game_progress*GetScreenWidth();
        const Rectangle btnStart = {GetScreenWidth()-400.f, GetScreenHeight()-100.f,400, 80};
        const Rectangle btnQuit = {20, GetScreenHeight()-100.f,300, 80};
        const Rectangle btnReroll = {baseX - 270, baseY+700,600, 80};
        Rectangle prefBox[2] = {
            { baseX+220-260, baseY + 720-70-10-70-30, 310, 70 },
            { baseX+220-260, baseY + 720-70-30, 310, 70 }
        };
        BeginDrawing();
        ClearBackground(BLACK);

        

        DrawTexturePro(
            tex::earth,
            Rectangle{0,0,(float)tex::earth.width,(float)tex::earth.height},
            Rectangle{baseX-256, baseY+200, 512, 256},
            {0,0}, 0, Fade(WHITE, 0.85f));
        //DrawText(TextFormat("%d opponents", max_factions-3), baseX - 320, baseY+400, 128, WHITE);

        DrawTexturePro(
            minimap.texture,
            Rectangle{0,0,(float)GRID_SIZE*MINIMAP_SCALE,-(float)GRID_SIZE*MINIMAP_SCALE},
            Rectangle{baseX-270, baseY+545, 148, 148},
            {0,0}, 0, WHITE);
        Vector2 mouse = GetMousePosition();
        // preferences
        if(unlocked_preferences)
            for (int i=0; i<2; ++i) {
                bool hover = false;//!new_game_transition_mode && CheckCollisionPointRec(mouse, prefBox[i]);
                DrawRectangleRec(prefBox[i], hover ? Fade(ORANGE,0.34f) : BLACK);
                DrawRectangleLinesEx(prefBox[i], 2, hover?GRAY:BLACK);
                DrawTextSmall(i == 0 ? "Start perk A" : "Start perk B",prefBox[i].x + 10,prefBox[i].y + 6,30,hover?GRAY:DARKGRAY);
                int pref = player_preferred_start[i];
                DrawTextSmall(preference_desc[pref], prefBox[i].x + 10, prefBox[i].y + 32, 38, hover?WHITE:GRAY);

                float rot = 0;//GetTime() * 120.0f;
                Rectangle src = {0, 0, (float)preference_icon[pref]->width, (float)preference_icon[pref]->height};
                Rectangle dst = {prefBox[i].x + prefBox[i].width - 70,prefBox[i].y + 5, 60, 60};
                Vector2 origin = { dst.width / 2.0f, dst.height / 2.0f };
                dst.x += origin.x;
                dst.y += origin.y;
                DrawTexturePro(*preference_icon[pref], src, dst, origin, rot, WHITE);
                if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    PlaySound(sound::select);
                    int p = player_preferred_start[i];
                    do {
                        p = (p + 1) % PREFERENCE_COUNT;
                    } while (!(unlocked_preferences & (1ULL << p)) && p!=PREFERENCE_RAILGUN);
                    player_preferred_start[i] = p;
                }

                Rectangle leftArrow  = { prefBox[i].x - 28-7, prefBox[i].y + 15, 24, 40 };
                Rectangle rightArrow = { prefBox[i].x + prefBox[i].width + 4, prefBox[i].y + 15, 24, 40 };

                bool hoverL = !new_game_transition_mode && CheckCollisionPointRec(mouse, leftArrow);
                bool hoverR = !new_game_transition_mode && CheckCollisionPointRec(mouse, rightArrow);
                DrawTriangle(
                    Vector2{ leftArrow.x + leftArrow.width, leftArrow.y },
                             Vector2{ leftArrow.x, leftArrow.y + leftArrow.height/2 },
                             Vector2{ leftArrow.x + leftArrow.width, leftArrow.y + leftArrow.height },
                             hoverL ? ORANGE : DARKGRAY);
                DrawTriangle(
                    Vector2{ rightArrow.x, rightArrow.y },
                             Vector2{ rightArrow.x, rightArrow.y + rightArrow.height },
                             Vector2{ rightArrow.x + rightArrow.width, rightArrow.y + rightArrow.height/2 },
                             hoverR ? ORANGE : DARKGRAY);

                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    PlaySound(sound::select);
                    int p = player_preferred_start[i];
                    if (hoverR) {
                        do {p = (p + 1) % PREFERENCE_COUNT;} while (!(unlocked_preferences & (1ULL << p)) && p != PREFERENCE_RAILGUN);
                        player_preferred_start[i] = p;
                    }
                    if (hoverL) {
                        do {p = (p - 1 + PREFERENCE_COUNT) % PREFERENCE_COUNT;} while (!(unlocked_preferences & (1ULL << p)) && p != PREFERENCE_RAILGUN);
                        player_preferred_start[i] = p;
                    }
                }

            }
        // Start button
        bool hoverStart = !new_game_transition_mode && CheckCollisionPointRec(mouse, btnStart);
        {
            int rank_index = 0;
            double rank_progress = 0;
            if(player_rating<1200) player_rating = 1200;
            if(player_rating>2200) player_rating = 2200;
            for (int i=0;i<4;i++)
                if (player_rating>=ranks[i].min && player_rating<ranks[i].max) {
                    rank_index = i;
                    break;
                }
            rank_progress = (player_rating - ranks[rank_index].min) / (ranks[rank_index].max - ranks[rank_index].min);
            if (rank_progress < 0) rank_progress = 0;
            if (rank_progress > 1) rank_progress = 1;
            float size = 64.f;
            float radius = size*0.65f;
            float end_angle = (float)(rank_progress * 360.0) - 90;
            float cx = btnStart.x-64;//baseX - 250 + 32;
            float cy = btnStart.y+32;//baseY + 650;
            DrawCircle(cx, cy, radius, BLACK);
            DrawRing({cx, cy}, radius - 8, radius, -90, end_angle, 64, ranks[rank_index].color);
            DrawRing({cx, cy}, radius - 8, radius, end_angle, 360-90, 64, Fade(ranks[rank_index].color, 0.4f));
            DrawTexturePro(
                *ranks[rank_index].tex,
                {0,0,(float)ranks[rank_index].tex->width,(float)ranks[rank_index].tex->height}, {cx - size/2, cy - size/2, size, size}, {0,0}, 0, ranks[rank_index].color
            );
            Rectangle circleRect = { cx - radius, cy - radius, radius * 2, radius * 2 };
            if (CheckCollisionPointRec(mouse, circleRect)) {
                const char* rankName = ranks[rank_index].name;
                int fontSize = 24;
                int textWidth = MeasureText(rankName, fontSize);
                DrawText(rankName, cx - textWidth/2, cy - radius - 30, fontSize, ranks[rank_index].color);
            }
        }
        DrawText("Embark now", btnStart.x, btnStart.y + 8, 58, hoverStart ? WHITE : GRAY);
        // quit button
        bool hoverReroll = !new_game_transition_mode && CheckCollisionPointRec(mouse, btnReroll);
        DrawText("Elsewhere", btnReroll.x, btnReroll.y + 8, 42, hoverReroll ? WHITE : GRAY);


        bool hoverQuit = !new_game_transition_mode && CheckCollisionPointRec(mouse, btnQuit);
        // DrawRectangleRec(btnQuit, hoverQuit ? DARKGRAY : BLACK);
        // DrawRectangleLinesEx(btnQuit, 2, DARKGRAY);
        DrawText("Back", btnQuit.x, btnQuit.y + 8, 58, hoverQuit ? WHITE : GRAY);
        EndDrawing();
        if (hoverStart && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            PlaySound(sound::select);
            new_game_transition_mode = 1;
        }
        if (hoverReroll && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            // new_game_transition_mode = -1;
            // main_menu_transition_mode = -1;
            showTechTree = false;
            showHelp = false;
            goto NEW_GAME;
        }
        if ((hoverQuit && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_ESCAPE)) {
            new_game_transition_mode = 2;
            PlaySound(sound::select);
        }
    }

    GAME_OVER:
    {
        int player_points = factions[0].count_members?factions[0].victory_points:0;
        int best_ai_points = 0;
        Faction* best_faction = factions;
        for (int fi = 3; fi < max_factions; fi++)
            if (factions[fi].victory_points >= best_ai_points) { // >= important
                best_ai_points = factions[fi].victory_points;
                best_faction = &factions[fi];
            }

        bool victory = player_points > best_ai_points;
        if(player_points<0) player_points = -player_points;

        const char* badTechNames[8];
        const Texture* badTechTextures[8];
        int badTechCount = 0;
        auto player_techs = factions[0].technology;
        if (player_techs & TECHNOLOGY_BIOWEAPON) {badTechTextures[badTechCount]=&tex::bioweapon;badTechNames[badTechCount++] = "BIOWEAPON";}
        if (player_techs & TECHNOLOGY_PROPAGANDA) {badTechTextures[badTechCount]=&tex::propaganda;badTechNames[badTechCount++] = "PROPAGANDA";}
        if (player_techs & TECHNOLOGY_SUPERIORITY) {badTechTextures[badTechCount]=&tex::utopia;badTechNames[badTechCount++] = "SUPERIORITY";}
        if (player_techs & TECHNOLOGY_ARTIFICIAL) {badTechTextures[badTechCount]=&tex::mind;badTechNames[badTechCount++] = "HIVEMENIND";}
        if (player_techs & TECHNOLOGY_AIFARM) {badTechTextures[badTechCount]=&tex::lab;badTechNames[badTechCount++] = "AI FARMS";}
        int unlocking_perk = 0; // zero = railgun = always unlocked = used to signify that we unlocked nothing
        if(victory && !badTechCount) {
            unlocking_perk = GetRandomValue(1, PREFERENCE_COUNT - 1);
            if (unlocked_preferences & (1ULL << unlocking_perk)) unlocking_perk = 0;
            else unlocked_preferences |= (1ULL << unlocking_perk);
        }

        int rank_index = 0;
        double rank_progress = 0;
        {
            double K = (game_time < GAME_DURATION || victory)?48.0:24.0; // if we lose but ran until the end, we get smaller penalty
            double E = 1.0 / (1.0 + std::pow(10.0, (AI_ELO - player_rating) / 400.0));
            if(victory) player_rating += K * (1.0 - E);
            else player_rating -= K * E;
            if(player_rating<1200) player_rating = 1200;
            if(player_rating>2200) player_rating = 2200;
            for (int i=0;i<4;i++)
                if (player_rating>=ranks[i].min && player_rating<ranks[i].max) {
                    rank_index = i;
                    break;
                }
            rank_progress = (player_rating - ranks[rank_index].min) / (ranks[rank_index].max - ranks[rank_index].min);
            if (rank_progress < 0) rank_progress = 0;
            if (rank_progress > 1) rank_progress = 1;
        }
        {
            FILE* f = fopen("midnight-save.dat", "wb");
            if(f) {
                fwrite(&unlocked_preferences, sizeof(unlocked_preferences), 1, f);
                fwrite(&player_rating, sizeof(player_rating), 1, f);
                fclose(f);
            }
        }
        while (true) {
            UpdateMusicStream(sound::bg);
            if(game_over_transition_mode)
                game_over_progress += GetFrameTime()*5.f;
            if(game_over_progress>1.f) {
                game_over_progress = 0.f;
                game_over_transition_mode = 0;
                if(unlocking_perk) {
                    reward_transition_mode = -1;
                    reward_progress = 0;
                    goto CLAIM_REWARDS;
                }
                main_menu_transition_mode = -2;
                goto MAIN_MENU;
            }
            float baseY = (float)GetScreenHeight()/2 - 450;
            float baseX = (float)GetScreenWidth()/2;
            if(game_over_transition_mode) baseX -= game_over_progress*game_over_progress*GetScreenWidth();
            const Rectangle btnStart = {GetScreenWidth()-400.f, GetScreenHeight()-100.f,400, 80};
            const Rectangle btnQuit = {20, GetScreenHeight()-100.f,300, 80};

            BeginDrawing();
            ClearBackground(BLACK);
            if(victory) {
                DrawTexturePro(
                    tex::victory,
                    Rectangle{0,0,(float)tex::sun.width,(float)tex::sun.height},
                    Rectangle{baseX-256, baseY+100, 512, 256},
                    {0,0}, 0, WHITE);
                //DrawText("BEST UTOPIA", baseX - MeasureText("BEST UTOPIA", 96)/2+70, baseY+420, 96, GREEN);
            }
            else if (game_time >= GAME_DURATION) {
                DrawTexturePro(
                    tex::defeat,
                    Rectangle{0,0,(float)tex::sun.width,(float)tex::sun.height},
                    Rectangle{baseX-256, baseY+100, 512, 256},
                    {0,0}, 0, WHITE);
                //DrawText("SURVIVED", baseX - MeasureText("SURVIVED", 96)/2+50, baseY+420, 96, ORANGE);
            }
            else {
                DrawTexturePro(
                    tex::defeat,
                    Rectangle{0,0,(float)tex::sun.width,(float)tex::sun.height},
                    Rectangle{baseX-256, baseY+100, 512, 256},
                    {0,0}, 0, WHITE);
                //DrawText("ELIMINATED", baseX - MeasureText("ELIMINATED", 96)/2+50, baseY+420, 96, RED);
            }
            char score[128];
            snprintf(score, sizeof(score), "Your utopia: %d   |   Best AI: %d", player_points, best_ai_points);
            if (game_time >= GAME_DURATION) DrawText(score, baseX - MeasureText(score, 42)/2+40, baseY+440, 42, WHITE);
            if (badTechCount) {
                float offset = MeasureText("Was it really worth it?", 28)/2;
                DrawTextSmall("Was it really worth it?", baseX - offset,baseY+510,28,ORANGE);
                for (int i = 0; i < badTechCount; i++) {
                    DrawTextureEx(*badTechTextures[i], {baseX - offset, baseY+550 + i * 28}, 0, 28 / (float)badTechTextures[i]->width, WHITE);
                    DrawTextSmall(TextFormat("%s", badTechNames[i]), baseX - offset+40, baseY+550 + i * 32,24, DARKGRAY);
                }
            }

            if (showTechTree) {
                DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.85f));
                DrawTechs(*best_faction);
            }
            bool techHover = CheckCollisionPointRec(GetMousePosition(), btnStart);
            if (techHover) {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    best_faction->technology_progress = 0;
                    showTechTree = !showTechTree;
                    PlaySound(sound::select2);
                }
            }
            const char* tech_msg = showTechTree?"Close techs":victory?"Your techs":"Winning techs";
            DrawText(tech_msg, btnStart.x+btnStart.width-MeasureText(tech_msg, 58), btnStart.y + 8, 58, techHover ? WHITE : GRAY);

            Vector2 mouse = GetMousePosition();
            bool hoverOk = !showTechTree && !game_over_transition_mode && CheckCollisionPointRec(mouse, btnQuit);
            if(!showTechTree) {
                DrawText(victory?"Rewards":"Retreat", btnQuit.x, btnQuit.y + 8, 58, hoverOk ? WHITE : GRAY);
            }
            EndDrawing();
            if ((!showTechTree && hoverOk && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_ESCAPE)) {
                PlaySound(sound::select);
                game_over_transition_mode = 1;
            }
        }

        CLAIM_REWARDS:
        while (true) {
            UpdateMusicStream(sound::bg);
            if(reward_transition_mode)
                reward_progress += GetFrameTime()*5.f;
            if(reward_progress>1.f) {
                reward_progress = 0.f;
                if (reward_transition_mode==-1) {reward_transition_mode=0;}
                else if (reward_transition_mode==1) {
                    reward_progress = 0;
                    reward_transition_mode = -1;
                    main_menu_transition_mode = -2;
                    goto MAIN_MENU;
                }
            }
            float baseY = (float)GetScreenHeight()/2 - 600;
            float baseX = (float)GetScreenWidth()/2;
            if(reward_transition_mode==-1) baseX += (1.0f-reward_progress)*(1.0f-reward_progress)*GetScreenWidth();
            else baseX -= reward_progress*reward_progress*GetScreenWidth();
            Rectangle btnOk = {baseX - 400, baseY+820,800, 80};

            BeginDrawing();
            ClearBackground(BLACK);
            if(unlocking_perk) {
                DrawTexturePro(
                    tex::reward,
                    Rectangle{0,0,(float)tex::reward.width,(float)tex::reward.height},
                    Rectangle{baseX-256, baseY+100, 512, 256},
                    {0,0}, 0, WHITE);
            }
            else {
                DrawTexturePro(
                    tex::sun,
                    Rectangle{0,0,(float)tex::sun.width,(float)tex::sun.height},
                    Rectangle{baseX-256, baseY+100, 512, 256},
                    {0,0}, 0, WHITE);
            }
            // float size = 128.f;
            // float radius = size*0.55f;
            // float end_angle = (float)(rank_progress * 360.0) - 90;
            // float cx = baseX - MeasureText(ranks[rank_index].name, 96)/2+100-70;
            // float cy = baseY+420+45;
            // DrawRing({cx, cy}, radius - 14, radius, -90, end_angle, 64, ranks[rank_index].color);
            // DrawRing({cx, cy}, radius - 14, radius, end_angle, 360-90, 64, Fade(ranks[rank_index].color, 0.4f));
            // DrawTexturePro(
            //     *ranks[rank_index].tex,
            //     {0,0,(float)ranks[rank_index].tex->width,(float)ranks[rank_index].tex->height},
            //     {cx - size/2, cy - size/2, size, size},
            //     {0,0}, 0, ranks[rank_index].color
            // );
            //
            // DrawText(ranks[rank_index].name, baseX - MeasureText(ranks[rank_index].name, 96)/2+100, baseY+420, 96, ranks[rank_index].color);

            // --- Ethical tech disclosure ---
            if(unlocking_perk) {
                DrawText("New start perk!", baseX - MeasureText("New start perk!", 96)/2+75, baseY+420, 96, GREEN);

                DrawTextSmall("This may appear only if you avoided iffy techs.", baseX - MeasureText("This may appear only if you avoided iffy techs.", 32)/2+75,
                baseY+550,32,GRAY);
            }
            else if(badTechCount){
                DrawTextSmall("No rewards after using iffy techs.", baseX - MeasureText("No rewards after using iffy techs.", 28)/2,baseY+610,28,ORANGE);
            }
            else {
                DrawTextSmall("You avoided iffy techs. Good job!",
                              baseX - MeasureText("You avoided iffy techs. Good job!", 32)/2+60,
                              baseY+610,32,Fade(ORANGE, 0.8f));
            }
            // --- OK button ---
            Vector2 mouse = GetMousePosition();
            bool hoverOk = !reward_transition_mode && CheckCollisionPointRec(mouse, btnOk);
            DrawRectangleRec(btnOk, hoverOk ? DARKGRAY : BLACK);
            DrawRectangleLinesEx(btnOk, 2, GRAY);
            DrawText("OK", btnOk.x + btnOk.width/2 - MeasureText("OK", 48)/2, btnOk.y + 14, 48,hoverOk ? WHITE : GRAY);
            EndDrawing();
            if ((hoverOk && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_ESCAPE)) {
                reward_transition_mode = 1;
                PlaySound(sound::select);
            }
        }
    }




    START_GAME:;
    num_units = 0;
    for(int i=0;i<max_factions;i++) {
        factions[i].visible_knowledge.reset();
        factions[i].technology = 0;
        factions[i].technology_progress = 0.f;
    }

    // minimap
    // RenderTexture2D minimap = LoadRenderTexture(GRID_SIZE * MINIMAP_SCALE, GRID_SIZE * MINIMAP_SCALE);
    if(SHOW_MINIMAP) {
        const int MINIMAP_SCALE = 2;
        BeginTextureMode(minimap);
        ClearBackground(BLACK);
        for (int y = 0; y < GRID_SIZE; y++)
            for (int x = 0; x < GRID_SIZE; x++)
                DrawRectangle(x * MINIMAP_SCALE, y * MINIMAP_SCALE, MINIMAP_SCALE, MINIMAP_SCALE, Color{ 20,20,20,255 });
        EndTextureMode(); // minimap
    }

    int num_decorators = 0;   // track trees
    for (int y = 1; y < GRID_SIZE-1; y++)
        for (int x = 1; x < GRID_SIZE-1; x++) {
            Terrain &T = terrainGrid[y][x];
            bool grass =
                (T.texture == &tex::grass  ||
                T.texture == &tex::grass2 ||
                T.texture == &tex::grass3 ||
                T.texture == &tex::grass4);
            bool hill = IsHill(T.texture);
            bool mountain = IsMountain(T.texture);
            if (!grass && !hill && !mountain) continue;
            // if ((float)GetRandomValue(0, 1000000) / 1000000.0f < 0.02f) continue;
            if (hill && (float)GetRandomValue(0, 1000000) / 1000000.0f < 0.2f) continue;
            if (mountain && (float)GetRandomValue(0, 1000000) / 1000000.0f < 0.95f) continue;
            float f = ForestNoise(x, y);
            if((float)GetRandomValue(0, 1000000) / 1000000.0f<f*0.1f) continue;
            if (f > 0.62f && num_decorators<MAX_DECORATORS-1) {
                float ox = ((float)GetRandomValue(-5000, 5000) / 5000.0f) * 0.25f;  // ±0.25 tile
                float oy = ((float)GetRandomValue(-5000, 5000) / 5000.0f) * 0.25f;  // ±0.25 tile
                decorators[num_decorators++] = {&tex::tree,(float)x+ox-0.5f,(float)y+oy-0.5f,1.0f};
                T.extra_sight -= 0.7f;
                if(T.extra_sight<-0.7f) T.extra_sight = -0.7f;
                T.speed = 0.4f;
            }
        }


    // declare camera
    Camera2D camera = { 0 };
    float target_zoom = CAMERA_ZOOM;
    camera.target = { GRID_SIZE * TILE_SIZE / 2.0f, GRID_SIZE * TILE_SIZE / 2.0f }; // will overwrite this
    camera.offset = { (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 };
    camera.zoom = 0.05;

    // spawn starting units
    // -----------------------------------------------------------------------------
    // WORLD GENERATION — BASES + STARTING UNITS + MAP SCATTER
    // -----------------------------------------------------------------------------
    auto campExistsTooClose = [&](float x, float y, float minDist) {
        for (int i = 0; i < num_units; i++) {
            Unit &u = units[i];
            if(!u.speed) continue;
            float dx = u.x - x;
            float dy = u.y - y;
            if (dx*dx + dy*dy < (minDist+u.size) * (minDist+u.size))
                return true;
        }
        return false;
    };
    // ====================================================================
    // 1. PLAYER BASE (force spawn on speed == 1.0 tile)
    // ====================================================================
    {
        float bx = 0.f;
        float by = 0.f;
        bool placed = false;

        // Hard cap to avoid infinite loops
        for (int attempt = 0; attempt < 2000 && !placed; ++attempt) {
            int tx = GetRandomValue(20, GRID_SIZE - 20);
            int ty = GetRandomValue(20, GRID_SIZE - 20);
            Terrain &T = terrainGrid[ty][tx];
            // Require exactly neutral movement
            if (fabsf(T.speed - 1.0f) > 0.001f)
                continue;
            bx = (float)tx;
            by = (float)ty;
            placed = true;
        }
        // Absolute fallback (should basically never happen)
        if (!placed) {
            for (int y = 20; y < GRID_SIZE - 20 && !placed; ++y)
                for (int x = 20; x < GRID_SIZE - 20 && !placed; ++x)
                    if (fabsf(terrainGrid[y][x].speed - 1.0f) <= 0.001f) {
                        bx = (float)x;
                        by = (float)y;
                        placed = true;
                    }
        }
        // Spawn base
        CREATE_CAMP(&factions[0], bx-(player_preferred_start[0]==PREFERENCE_SPACING?8.f:0.3f), by);
        CREATE_CAMP(&factions[0], bx+(player_preferred_start[1]==PREFERENCE_SPACING?8.f:0.3f), by);

        for(int p=0;p<2;++p) {
            float px = p==0?(bx-3):(bx+3);
            int pref = player_preferred_start[p];
            if(pref==PREFERENCE_RAILGUN) {
                CREATE_RAILGUN(&factions[1], px, by-3);
                CREATE_RAILGUN(&factions[1], px, by+3);
            }
            else if(pref==PREFERENCE_FARM) {
                CREATE_FIELD(&factions[1], px, by-0.7f);
                CREATE_FIELD(&factions[1], px, by+0.7f);
            }
            else if(pref==PREFERENCE_ANIMAL) {
                if(GetRandomValue(0,99)<50) {CREATE_BISON(&factions[2], px, by-4);}
                else CREATE_WOLF(&factions[2], px, by-4);
                if(GetRandomValue(0,99)<50) {CREATE_BISON(&factions[2], px, by);}
                else CREATE_WOLF(&factions[2], px, by);
                if(GetRandomValue(0,99)<50) {CREATE_BISON(&factions[2], px, by+4);}
                else CREATE_WOLF(&factions[2], px, by+4);
                factions->technology_progress += 0.5;
            }
            else if(pref==PREFERENCE_TANK) {
                if(GetRandomValue(0,99)<50) {CREATE_TANK(&factions[1], px, by);}
                else {CREATE_VAN(&factions[1], px, by);}
            }
            else if(pref==PREFERENCE_WAREHOUSE) {
                px = p==0?(bx-5):(bx+5);
                CREATE_WAREHOUSE(&factions[1], px, by);
                RevealUnitToAllFactions(num_units - 1);
            }
            else if(pref==PREFERENCE_CURIO) {
                px = p==0?(bx-5):(bx+5);
                CREATE_CURIO(&factions[1], px, by);
                //RevealUnitToAllFactions(num_units - 1);
            }
            else if(pref==PREFERENCE_LAB) {
                px = p==0?(bx-5):(bx+5);
                if(GetRandomValue(0,99)<50) {CREATE_LAB(&factions[1], px, by);}
                else CREATE_DATACENTER(&factions[1], px, by);
            }
            else if(pref==PREFERENCE_ESPER) {
                CREATE_ESPER(ANIMAL_FACTION, px, by);
                RevealUnitToAllFactions(num_units - 1);
            }
            else if(pref==PREFERENCE_ROOMBA) {
                CREATE_ROOMBA(&factions[1], px, by-3);
                CREATE_ROOMBA(&factions[1], px*0.5f+bx*0.5f, by);
                CREATE_ROOMBA(&factions[1], px, by+3);
            }
        }

        // Spawn starting humans
        for (int i=0; i<8+(player_preferred_start[0]==PREFERENCE_HUMAN?8:0)+(player_preferred_start[1]==PREFERENCE_HUMAN?8:0); i++) {
            float sx = bx + (GetRandomValue(-5000, 5000) * 0.0002f);
            float sy = by + (GetRandomValue(-5000, 5000) * 0.0002f);
            CREATE_HUMAN(&factions[0], sx, sy);
            if(player_preferred_start[0]==PREFERENCE_EXPERIENCE) units[num_units-1].experience += 8.f;
            if(player_preferred_start[1]==PREFERENCE_EXPERIENCE) units[num_units-1].experience += 8.f;
            if(player_preferred_start[0]==PREFERENCE_SPEED) units[num_units-1].speed *= 1.2f;
            if(player_preferred_start[1]==PREFERENCE_SPEED) units[num_units-1].speed *= 1.2f;
        }

        camera.target = {
            bx * TILE_SIZE,
            by * TILE_SIZE
        };
    }


    // ====================================================================
    // 2. AI BASES — placed randomly with spacing against existing camps
    // ====================================================================
    float BASE_MIN_DIST = GRID_SIZE/3;
    for (int fi = 3; fi < max_factions; fi++) {
        bool placed = false;
        for (int attempt = 0; attempt < 300 && !placed; attempt++) {
            float bx = GetRandomValue(20, GRID_SIZE - 20);
            float by = GetRandomValue(20, GRID_SIZE - 20);
            if (campExistsTooClose(bx, by, BASE_MIN_DIST))
                continue;
            for(int p=0;p<2;++p) {
                float px = p==0?(bx-3):(bx+3);
                int pref = GetRandomValue(0,100)<25?GetRandomValue(0,PREFERENCE_COUNT-1):0; // really prefer railguns in most situations
                if(p==0) {CREATE_CAMP(&factions[fi], bx-(pref==PREFERENCE_SPACING?8.f:0.3f), by);}
                else {CREATE_CAMP(&factions[fi], bx+(pref==PREFERENCE_SPACING?8.f:0.3f), by);}
                if(pref==PREFERENCE_RAILGUN) {
                    CREATE_RAILGUN(&factions[1], px, by-3);
                    CREATE_RAILGUN(&factions[1], px, by+3);
                }
                else if(pref==PREFERENCE_FARM) {
                    CREATE_FIELD(&factions[1], px, by-0.7f);
                    CREATE_FIELD(&factions[1], px, by+0.7f);
                }
                else if(pref==PREFERENCE_ANIMAL) {
                    if(GetRandomValue(0,99)<50) {CREATE_BISON(&factions[2], px, by-4);}
                    else CREATE_WOLF(&factions[2], px, by-4);
                    if(GetRandomValue(0,99)<50) {CREATE_BISON(&factions[2], px, by);}
                    else CREATE_WOLF(&factions[2], px, by);
                    if(GetRandomValue(0,99)<50) {CREATE_BISON(&factions[2], px, by+4);}
                    else CREATE_WOLF(&factions[2], px, by+4);
                    factions[fi].technology_progress += 0.5;
                }
                else if(pref==PREFERENCE_TANK) {
                    if(GetRandomValue(0,99)<50) {CREATE_TANK(&factions[1], px, by);}
                    else {CREATE_VAN(&factions[1], px, by);}
                }
                else if(pref==PREFERENCE_WAREHOUSE) {
                    px = p==0?(bx-5):(bx+5);
                    CREATE_WAREHOUSE(&factions[1], px, by);
                    RevealUnitToAllFactions(num_units - 1);
                }
                else if(pref==PREFERENCE_CURIO) {
                    px = p==0?(bx-5):(bx+5);
                    CREATE_CURIO(&factions[1], px, by);
                    //RevealUnitToAllFactions(num_units - 1);
                }
                else if(pref==PREFERENCE_ESPER) {
                    //CREATE_ESPER(ANIMAL_FACTION, px, by);
                    RevealUnitToAllFactions(num_units - 1);
                }
                else if(pref==PREFERENCE_LAB) {
                    px = p==0?(bx-5):(bx+5);
                    if(GetRandomValue(0,99)<50) {CREATE_LAB(&factions[1], px, by);}
                    else CREATE_DATACENTER(&factions[1], px, by);
                }
                else if(pref==PREFERENCE_ROOMBA) {
                    CREATE_ROOMBA(&factions[1], px, by-3);
                    CREATE_ROOMBA(&factions[1], px*0.5f+bx*0.5f, by);
                    CREATE_ROOMBA(&factions[1], px, by+3);
                }
            }
            for (int i = 0; i < 8; i++) {
                float sx = bx + (GetRandomValue(-5000, 5000) * 0.0002f);
                float sy = by + (GetRandomValue(-5000, 5000) * 0.0002f);
                CREATE_HUMAN(&factions[fi], sx, sy);
            }
            placed = true;
        }
        // If unable to place after many attempts, place somewhere anyway
        if (!placed) {
            float bx = GetRandomValue(20, GRID_SIZE - 20);
            float by = GetRandomValue(20, GRID_SIZE - 20);
            CREATE_CAMP(&factions[fi], bx-0.3, by);
            CREATE_CAMP(&factions[fi], bx+0.3, by);
            for (int i = 0; i < 4; i++) {
                float sx = bx + (GetRandomValue(-5000, 5000) * 0.0002f);
                float sy = by + (GetRandomValue(-5000, 5000) * 0.0002f);
                CREATE_HUMAN(&factions[fi], sx, sy);
            }
        }
    }

    // ====================================================================
    // 3. MAP SCATTER — neutral structures + tanks
    // ====================================================================
    int NUM_NEUTRAL_STRUCTURES = GRID_SIZE*GRID_SIZE/512;
    int NUM_NEUTRAL_TANKS = GRID_SIZE*GRID_SIZE/512/2;
    if(GetRandomValue(0,99)<50) NUM_NEUTRAL_TANKS *= 2;
    if(GetRandomValue(0,99)<50) NUM_NEUTRAL_TANKS *= 2;
    if(GetRandomValue(0,99)<50) NUM_NEUTRAL_TANKS *= 2;
    int NUM_WILD_ANIMALS= GRID_SIZE*GRID_SIZE/512/8;
    float AVOID_BASE_RADIUS = 7.0f;

    auto tooCloseToAnyCamp = [&](float x, float y) {return campExistsTooClose(x, y, AVOID_BASE_RADIUS);};
    for (int iy = 0; iy < GRID_SIZE; ++iy)
    for (int ix = 0; ix < GRID_SIZE; ++ix)
        if (terrainGrid[iy][ix].texture == &tex::treasure) {
            terrainGrid[iy][ix] = {
                &tex::mountain,
                0.4f,
                1.0f
            };
            switch (GetRandomValue(0, 3)) {
                case 0:{ CREATE_MINE(&factions[1], ix, iy);        break;}
                case 1:{ CREATE_CURIO(&factions[1], ix, iy);       break;}
                case 2:{ CREATE_LIGHTHOUSE(&factions[1], ix, iy);  break;}
                case 3:{ CREATE_RADIO(&factions[1], ix, iy);       break;}
            }
        }

    int count_warehouses = 0;
    int count_curio = 0;
    for (int i = 0; i < NUM_NEUTRAL_STRUCTURES*2; i+=2) {
        float x, y;
        x = GetRandomValue(20, GRID_SIZE - 20);
        y = GetRandomValue(20, GRID_SIZE - 20);
        if (tooCloseToAnyCamp(x, y)) continue;
        Terrain &T = terrainGrid[(int)y][(int)x];
        bool isGrass  = (T.texture == &tex::grass || T.texture == &tex::grass2 || T.texture == &tex::grass3 || T.texture == &tex::grass4);
        bool isDesert = (T.texture == &tex::desert);
        if(T.texture == &tex::water) {
            if(GetRandomValue(0, 99) < 90) {
                CREATE_HOVERCRAFT(&factions[1], x, y);
            }
            continue;
        }

        bool isNearWater = (terrainGrid[(int)y-2][(int)x].texture==&tex::water || terrainGrid[(int)y][(int)x-2].texture==&tex::water || terrainGrid[(int)y+2][(int)x].texture==&tex::water || terrainGrid[(int)y][(int)x+2].texture==&tex::water);
        if(isNearWater) i-=1;
        int type = GetRandomValue(0, 5);
        switch (type) {
            case 1:
                if (T.texture == &tex::mountain) {
                    CREATE_MINE(&factions[1], x, y);
                    continue;
                }
                if (!isGrass) continue;
                {
                float spacing = 0.7f;
                CREATE_FIELD(&factions[1], x-spacing, y-spacing);
                CREATE_FIELD(&factions[1], x-spacing, y+spacing);
                CREATE_FIELD(&factions[1], x+spacing, y+spacing);
                CREATE_FIELD(&factions[1], x+spacing, y-spacing);
                }
                break;
            case 3:
                if (!isDesert && GetRandomValue(0, 99) < 80) {
                    if(GetRandomValue(0, 99) < 40 && count_curio<3) {
                        count_curio++;
                        CREATE_CURIO(&factions[1], x, y);
                        //RevealUnitToAllFactions(num_units - 1);
                    }
                    continue;
                }
                CREATE_OIL(&factions[1], x, y);
                break;
            case 5:
                CREATE_DATACENTER(&factions[1], x, y);
                break;
            case 4:
                if(GetRandomValue(0, 99) < 80 && count_warehouses) {
                    CREATE_VAN(&factions[1], x, y);
                }
                else if(count_warehouses<3){
                    count_warehouses++;
                    CREATE_RAILGUN(&factions[1], x-GetRandomValue(0, 5)-2, y-GetRandomValue(0, 5)-2);
                    CREATE_RAILGUN(&factions[1], x-GetRandomValue(0, 5)-2, y+GetRandomValue(0, 5)+2);
                    CREATE_RAILGUN(&factions[1], x+GetRandomValue(0, 5)+2, y-GetRandomValue(0, 5)-2);
                    CREATE_RAILGUN(&factions[1], x+GetRandomValue(0, 5)+2, y+GetRandomValue(0, 5)-2);
                    CREATE_WAREHOUSE(&factions[1], x, y);
                    RevealUnitToAllFactions(num_units - 1);
                }
                break;
            case 0:
                if(GetRandomValue(0, 99) < 80) {
                    CREATE_LAB(&factions[1], x, y);
                }
                else {
                    CREATE_CAMP(&factions[1], x, y);
                }
                break;
            case 2:
                if(isNearWater && GetRandomValue(0, 99) < 30) {
                    CREATE_LIGHTHOUSE(&factions[1], x, y);
                    RevealUnitToAllFactions(num_units - 1);
                }
                else {
                    CREATE_RADIO(&factions[1], x, y);
                    float spacing = 10.f;
                    if(GetRandomValue(0, 99) < 30) { CREATE_VAN(&factions[1], x-spacing, y-spacing); }
                    if(GetRandomValue(0, 99) < 30) { CREATE_VAN(&factions[1], x-spacing, y+spacing); }
                    if(GetRandomValue(0, 99) < 30) { CREATE_VAN(&factions[1], x+spacing, y+spacing); }
                    if(GetRandomValue(0, 99) < 30) { CREATE_VAN(&factions[1], x+spacing, y-spacing); }
                    spacing = 15.f;
                    if(GetRandomValue(0, 99) < 30) { CREATE_VAN(&factions[1], x-spacing, y); }
                    if(GetRandomValue(0, 99) < 30) { CREATE_VAN(&factions[1], x+spacing, y); }
                    if(GetRandomValue(0, 99) < 30) { CREATE_VAN(&factions[1], x, y+spacing); }
                    if(GetRandomValue(0, 99) < 30) { CREATE_VAN(&factions[1], x, y-spacing); }
                }
                break;
        }
    }

    for (int i = 0; i < NUM_NEUTRAL_TANKS; i++) {
        float x, y;
        x = GetRandomValue(20, GRID_SIZE - 20);
        y = GetRandomValue(20, GRID_SIZE - 20);
        if (tooCloseToAnyCamp(x, y)) continue;
        Terrain &T = terrainGrid[(int)y][(int)x];
        bool isDesert = (T.texture == &tex::desert);
        if(T.texture==&tex::water) continue;
        bool isNearWater = (terrainGrid[(int)y-2][(int)x].texture==&tex::water || terrainGrid[(int)y][(int)x-2].texture==&tex::water || terrainGrid[(int)y+2][(int)x].texture==&tex::water || terrainGrid[(int)y][(int)x+2].texture==&tex::water);
        if(isNearWater) {
            if(GetRandomValue(0,100)<30) {
                CREATE_LIGHTHOUSE(&factions[1], x, y);
                RevealUnitToAllFactions(num_units - 1);
            }
            CREATE_HOVERCRAFT(&factions[1], x, y-1);
            continue;
        }
        if (T.texture == &tex::mountain || T.texture == &tex::hill
            || T.texture == &tex::hill2 || T.texture == &tex::hill3 || T.texture == &tex::hill4)
            {CREATE_RAILGUN(&factions[1], x, y);continue;}
        if (!isDesert && GetRandomValue(0, 99) < 50) {
            if(T.texture==&tex::grass && T.speed<1.f) { // roombas in the forest
                CREATE_ROOMBA(&factions[1], x-0.3f, y-0.3f);
                CREATE_ROOMBA(&factions[1], x+0.3f, y-0.3f);
                CREATE_ROOMBA(&factions[1], x-0.3f, y+0.3f);
                CREATE_ROOMBA(&factions[1], x+0.3f, y+0.3f);
            }
            continue; // more tanks in the desert - roombas elsewhere
        }
        if (GetRandomValue(0, 99) < 70) continue; // too many mechas saturate the early game, so make them rarer without droping firepowser by clustering
        //if (GetRandomValue(0, 99) < 85) continue;
        //cluster some tanks - the player should feel lucky to find something like this
        CREATE_TANK(&factions[1], x-0.3f, y-0.3f);
        CREATE_TANK(&factions[1], x+0.3f, y-0.3f);
        CREATE_TANK(&factions[1], x, y+0.3f);
    }

    for (int i = 0; i < NUM_WILD_ANIMALS; i++) {
        float x, y;
        x = GetRandomValue(20, GRID_SIZE - 20);
        y = GetRandomValue(20, GRID_SIZE - 20);
        if (tooCloseToAnyCamp(x, y)) continue;
        Terrain &T = terrainGrid[(int)y][(int)x];
        if(T.texture==&tex::grass) {
            if(GetRandomValue(0, 99) < 50) {
                CREATE_BISON(ANIMAL_FACTION, x, y);
                CREATE_BISON(ANIMAL_FACTION, x+1, y+1);
            }
            /*else {
                CREATE_WOLF(ANIMAL_FACTION, x, y);
                CREATE_WOLF(ANIMAL_FACTION, x+1, y+1);
            }*/
        }
        if (T.texture == &tex::hill) {
            CREATE_RAT(ANIMAL_FACTION, x-0.2, y+0.2);
            CREATE_RAT(ANIMAL_FACTION, x-0.2, y-0.2);
            CREATE_RAT(ANIMAL_FACTION, x+0.2, y+0.2);
            CREATE_RAT(ANIMAL_FACTION, x+0.2, y-0.2);
            CREATE_RAT(ANIMAL_FACTION, x, y);
        }
        if(T.texture==&tex::mountain) {
            CREATE_SNOWMAN(ANIMAL_FACTION, x-1, y-1);
            CREATE_SNOWMAN(ANIMAL_FACTION, x-1, y+1);
            CREATE_SNOWMAN(ANIMAL_FACTION, x+1, y-1);
            CREATE_SNOWMAN(ANIMAL_FACTION, x+1, y+1);
        }
    }


    // setup scene
    bool selecting = false;
    Vector2 selectStart = {0,0};
    Vector2 selectEnd = {0,0};

    bool draggingCamera = false;
    Vector2 dragStartScreen = {0,0};
    Vector2 dragStartTarget = {0,0};

    bool visible[GRID_SIZE][GRID_SIZE];
    bool explored[GRID_SIZE][GRID_SIZE];
    for (int y = 0; y < GRID_SIZE; y++)
        for (int x = 0; x < GRID_SIZE; x++) {
            visible[y][x] = explored[y][x] = false;
            //explored[y][x]  = true;
        }

    RenderTexture2D fog_mask = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    Image fog_hole_image = GenImageGradientRadial(1024, 1024, 0.7f, WHITE, BLANK);
    Texture2D fog_hole = LoadTextureFromImage(fog_hole_image);
    UnloadImage(fog_hole_image);

    //Color fogCenter = { 0, 0, 0, 220 };  // not full 255 so blending is smoother
    //Color fogEdge   = { 0, 0, 0, 0   };  // fully transparent
    // Image fog_edges_image = GenImageGradientRadial(1024, 1024, 0.7f, fogCenter, fogEdge);
    // Texture2D fog_edges   = LoadTextureFromImage(fog_edges_image);
    // UnloadImage(fog_edges_image);

    // duration
    game_time = 0.f;
    float time_norm = 0.f; // 0..1
    float last_message_counter = 0.f;
    const char* last_message = "Create the best utopia before re-pollution.";
    float prev_game_time = 0.f;

    while (true) {
        UpdateMusicStream(sound::bg);
        //factions[0].technology |= TECHNOLOGY_EXPLORE; // for debug
        float dt = GetFrameTime();
        float polution_speedup = 0.f;// just track this for this frame
        if (prev_game_time / GAME_DURATION < 0.7f && game_time / GAME_DURATION >= 0.7f ) {
            last_message_counter = 0.f;
            last_message = "Pollution is coming back! How is our utopia looking?";
        }
        if (prev_game_time / GAME_DURATION < 0.9f && game_time / GAME_DURATION >= 0.9f ) {
            last_message_counter = 0.f;
            last_message = "Pollution has peaked! This forray will end soon.";
        }
        prev_game_time = game_time;
        game_time += dt;
        if (game_time >= GAME_DURATION) {
            game_time = GAME_DURATION;
            showTechTree = false;
            goto GAME_OVER;
        }
        time_norm = game_time / GAME_DURATION;

        bool mouseCapturedByUI = false;
        if (IsKeyPressed(KEY_SPACE)) {
            PlaySound(sound::select2);
            if(currentMovementMode==MovementMode::Scattered) {
                currentMovementMode = MovementMode::Explore;
                last_message = "Explore formation";
                last_message_counter = 0.f;
            }
            else if(currentMovementMode==MovementMode::Explore) {
                currentMovementMode = MovementMode::Tight;
                last_message = "Tight formation";
                last_message_counter = 0.f;
            }
            else {
                currentMovementMode = MovementMode::Scattered;
                last_message = "Scattered formation";
                last_message_counter = 0.f;
            }
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            showTechTree = !showTechTree;
            PlaySound(sound::select2);
        }
        if(IsKeyPressed(KEY_TAB)) {
            showHelp = !showHelp;
            PlaySound(sound::select2);
        }

        fort_creation_px = 0;
        fort_creation_py = 0;
        fort_creation_num = 0;
        trench_creation_num = 0;
        turtle_creation_num = 0;
        fort_creation_total_health = 0;
        fort_creation_has_nearby = false;
        bool has_any_selected = false;

        {
            for (int i = 0; i < num_units; i++) {
                Unit &u = units[i];
                if(u.selected && u.speed) has_any_selected = true;
                if (u.selected && u.health && u.speed && (u.texture==&tex::human || u.texture==&tex::scout || u.texture==&tex::hero || u.texture==&tex::tank || u.texture==&tex::van || u.texture==&tex::fort)) {
                    fort_creation_total_health += u.max_health;
                    fort_creation_px += u.x;
                    fort_creation_py += u.y;
                    fort_creation_num++;
                }
                if(u.selected && (u.texture==&tex::tank || u.texture==&tex::van)) 
                    trench_creation_num++;
                if(u.selected && u.health>3.1f && u.speed && !is_mecha(u))
                    turtle_creation_num++;
            }
            fort_creation_px /= fort_creation_num;
            fort_creation_py /= fort_creation_num;
            for (int i = 0; i < num_units; i++) {
                Unit &u = units[i];
                if (!u.selected && u.health && !u.speed && (u.x-fort_creation_px)*(u.x-fort_creation_px)+(u.y-fort_creation_py)*(u.y-fort_creation_py)<25) {
                    fort_creation_has_nearby = true;
                    break;
                }
            }
        }

        if (CheckCollisionPointRec(GetMousePosition(), techBtn)) {
            mouseCapturedByUI = true;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                showTechTree = !showTechTree;
                PlaySound(sound::select2);
            }
        }
        if (!showTechTree && CheckCollisionPointRec(GetMousePosition(), helpBtn)) {
            mouseCapturedByUI = true;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                showHelp = !showHelp;
                PlaySound(sound::select2);
            }
        }
        
        if(!showTechTree && (factions->technology&TECHNOLOGY_TRENCHES) && !mouseCapturedByUI && (CheckCollisionPointRec(GetMousePosition(), trenchBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) && trench_creation_num) {
            mouseCapturedByUI = true;
            for (int i = 0; i < num_units; i++) {
                Unit &u = units[i];
                if (u.selected && u.health && (u.texture==&tex::tank || u.texture==&tex::van)) {
                    u.texture = &tex::ghost; // prevent explosion
                    CREATE_RAILGUN(factions, u.x, u.y);
                    units[num_units-1].health = units[num_units-1].max_health*u.health/u.max_health;
                    units[num_units-1].popup = "entrenched";
                    units[num_units-1].capturing = nullptr;
                    u.health = 0;
                }
            }
            PlaySound(sound::select2);
            trench_creation_num = 0;
        }

        if(!showTechTree && (factions->technology&TECHNOLOGY_TURTLING) && !mouseCapturedByUI && (CheckCollisionPointRec(GetMousePosition(), turtleBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) && turtle_creation_num) {
            mouseCapturedByUI = true;
            for (int i = 0; i < num_units; i++) {
                Unit &u = units[i];
                if (u.selected && u.health>3.1f && u.speed && !is_mecha(u)) {
                    u.health -= 3;
                    u.max_health -= 3;
                    if(true) {
                        float x = u.x + cos(u.angle*DEG2RAD)*u.size;
                        float y = u.y + sin(u.angle*DEG2RAD)*u.size;
                        CREATE_ROCK(u.faction, x, y);
                        units[num_units-1].popup = "turtling";
                        units[num_units-1].capturing = nullptr;
                    }
                    else {
                        u.popup = "harmed";
                        u.capturing = nullptr;
                    }
                }
            }
            PlaySound(sound::select2);
            turtle_creation_num = 0;
        }

        if(!showTechTree && (factions->technology&TECHNOLOGY_FORT) && !mouseCapturedByUI && (CheckCollisionPointRec(GetMousePosition(), fortBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) && fort_creation_num) {
            mouseCapturedByUI = true;
            if(fort_creation_has_nearby) {
                last_message = "Cannot build fort close to other structures";
                last_message_counter = 0.f;
            }
            else if(fort_creation_total_health<100) {
                last_message = "Forts need at least 20 stationed non-bloo industry";
                last_message_counter = 0.f;
            }
            else {
                for (int i = 0; i < num_units; i++) {
                    Unit &u = units[i];
                    if (u.selected && u.health && u.speed && (u.texture==&tex::human || u.texture==&tex::scout || u.texture==&tex::hero || u.texture==&tex::tank || u.texture==&tex::van || u.texture==&tex::fort)) {
                        u.health = 0;
                        u.texture = &tex::ghost;
                    }
                }
                CREATE_FORT(factions, fort_creation_px, fort_creation_py);
                fort_creation_total_health = fort_creation_total_health/5;
                units[num_units-1].max_health = fort_creation_total_health;
                units[num_units-1].health = fort_creation_total_health;
                units[num_units-1].size = sqrtf(fort_creation_total_health/10);
                PlaySound(sound::select2);
                last_message = "Built a fort";
                last_message_counter = 0.f;
                fort_creation_num = 0;
            }
        }

        if (showTechTree) showHelp = false;


        // camera
        const float move = 2000.0f * GetFrameTime() / camera.zoom;
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) camera.target.x += move;
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))  camera.target.x -= move;
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))    camera.target.y -= move;
        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))  camera.target.y += move;
        if(camera.target.x<0) camera.target.x = 0;
        if(camera.target.y<0) camera.target.y = 0;
        if(camera.target.x>=GRID_SIZE*TILE_SIZE) camera.target.x = GRID_SIZE*TILE_SIZE;
        if(camera.target.y>=GRID_SIZE*TILE_SIZE) camera.target.y = GRID_SIZE*TILE_SIZE;
        //camera.zoom += GetMouseWheelMove() * 0.2f;
        //if (camera.zoom < 0.5f) camera.zoom = 0.5f;
        //if (camera.zoom < 0.25f) camera.zoom = 0.25f;
        float wheel = GetMouseWheelMove();
        if (IsKeyDown(KEY_Q))  wheel += dt*20.f;
        if (IsKeyDown(KEY_E))  wheel -= dt*20.f;
        if (wheel) target_zoom += wheel * 0.1f;
        if (camera.zoom!=target_zoom) {
            Vector2 mouseWorldBefore = GetScreenToWorld2D(GetMousePosition(), camera);
            if (target_zoom < 0.1f) target_zoom = 0.1f;
            if (target_zoom > 1.7f) target_zoom = 1.7f;
            float diff = camera.zoom-target_zoom;
            camera.zoom -= diff*dt*5*(1+camera.zoom);
            if(diff*(camera.zoom-target_zoom)<=0.00001f) {
                camera.zoom = target_zoom;
            }
            Vector2 mouseWorldAfter = GetScreenToWorld2D(GetMousePosition(), camera);
            if(wheel>0) {
                camera.target.x += mouseWorldBefore.x - mouseWorldAfter.x;
                camera.target.y += mouseWorldBefore.y - mouseWorldAfter.y;
            }
        }


        Vector2 screenMin = GetScreenToWorld2D({0,0}, camera);
        Vector2 screenMax = GetScreenToWorld2D({(float)GetScreenWidth(), (float)GetScreenHeight()}, camera);
        int xMin = (int)(screenMin.x / TILE_SIZE) - 2;
        int yMin = (int)(screenMin.y / TILE_SIZE) - 2;
        int xMax = (int)(screenMax.x / TILE_SIZE) + 2;
        int yMax = (int)(screenMax.y / TILE_SIZE) + 2;
        if (xMin <= 0) xMin = 1;
        if (yMin <= 0) yMin = 1;
        if (xMax >= GRID_SIZE) xMax = GRID_SIZE-1;
        if (yMax >= GRID_SIZE) yMax = GRID_SIZE-1;
        float t = GetTime();

        #include "src/inline/gather_faction_stats.cpp"


        // process units
        #include "src/inline/process_units.cpp"
        #include "src/inline/process_projectiles.cpp"
        #include "src/inline/spawn_animals.cpp"

        
        // mouse over
        Vector2 worldMouse = GetScreenToWorld2D(GetMousePosition(), camera);
        Unit* hovered = nullptr;
        float bestDist = 999999.0f;
        for (int i = 0; i < num_units; i++) {
            Unit &u = units[i];
            if (u.health <= 0) continue;
            // skip non-visible units if fog hides them
            int ux = (int)u.x;
            int uy = (int)u.y;
            if (ux < 0 || uy < 0 || ux >= GRID_SIZE || uy >= GRID_SIZE) continue;
            if (!visible[uy][ux]) continue;
            // compute pixel center of unit
            float px = u.x * TILE_SIZE;
            float py = u.y * TILE_SIZE;
            // radius in world units (same as your rendering logic)
            float radius = u.size * TILE_SIZE * 0.5f;
            float dx = worldMouse.x - px;
            float dy = worldMouse.y - py;
            float dist2 = dx*dx + dy*dy;
            if (dist2 < radius * radius && dist2 < bestDist) {
                hovered = &u;
                bestDist = dist2;
            }
        }
        Terrain* hoveredTerrain = nullptr;
        {
            int ux = (int)(worldMouse.x/TILE_SIZE+0.5f);
            int uy = (int)(worldMouse.y/TILE_SIZE+0.5f);
            if (ux < 0 || uy < 0 || ux >= GRID_SIZE || uy >= GRID_SIZE){}
            else if(!visible[uy][ux]) {}
            else hoveredTerrain = &terrainGrid[uy][ux];
        }

        // camera movement
        if (!mouseCapturedByUI && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            draggingCamera = true;
            dragStartScreen = GetMousePosition();
            dragStartTarget = camera.target;
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
            draggingCamera = false;
        }

        if (draggingCamera) {
            Vector2 now = GetMousePosition();
            Vector2 delta = {
                (dragStartScreen.x - now.x) / camera.zoom,
                (dragStartScreen.y - now.y) / camera.zoom
            };

            camera.target = {
                dragStartTarget.x + delta.x,
                dragStartTarget.y + delta.y
            };
        }

        // issue movement order
        if (!mouseCapturedByUI && (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsKeyPressed(KEY_ENTER))) {
            Vector2 worldClick = IsKeyPressed(KEY_ENTER)
                ?GetScreenToWorld2D({(float)GetScreenWidth()*0.5f, (float)GetScreenHeight()*0.5f}, camera)
                :GetScreenToWorld2D(GetMousePosition(), camera);
            float tx = (worldClick.x / TILE_SIZE);
            float ty = (worldClick.y / TILE_SIZE);
            for (int i = 0; i < num_units; i++) {
                Unit &u = units[i];

                if (u.selected && u.speed && u.texture!=&tex::esper) {
                    u.stunned = 0; // we "cheat" in favor of smooth gameplay
                    if(currentMovementMode==MovementMode::Explore) {
                        u.target_x = tx + (float)GetRandomValue(-10,10);
                        u.target_y = ty + (float)GetRandomValue(-10,10);
                    }
                    else if(currentMovementMode==MovementMode::Scattered) {
                        u.target_x = tx + (float)GetRandomValue(-2,2);
                        u.target_y = ty + (float)GetRandomValue(-2,2);
                    }
                    else {

                        u.target_x = tx;
                        u.target_y = ty;
                    }
                }
            }
        }

        // selection
        if (!mouseCapturedByUI && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            selecting = true;
            selectStart = GetScreenToWorld2D(GetMousePosition(), camera);
            selectEnd = selectStart;
        }
        if (!mouseCapturedByUI && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && selecting)
            selectEnd = GetScreenToWorld2D(GetMousePosition(), camera);
        if (!mouseCapturedByUI && IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && selecting) {
            selecting = false;
            float minX = fminf(selectStart.x, selectEnd.x);
            float maxX = fmaxf(selectStart.x, selectEnd.x);
            float minY = fminf(selectStart.y, selectEnd.y);
            float maxY = fmaxf(selectStart.y, selectEnd.y);
            for (int i = 0; i < num_units; i++) {
                Unit &u = units[i];
                if(u.faction!=factions) {u.selected=false; continue;} //only first faction can be selected
                float px = u.x * TILE_SIZE;
                float py = u.y * TILE_SIZE;
                float extra = u.size+u.extra_scale+0.01f;
                extra *= TILE_SIZE/2;
                u.selected = (px >= minX-extra&& px <= maxX+extra && py >= minY-extra && py <= maxY+extra);
            }
        }

        if (!mouseCapturedByUI && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
            selectStart = GetScreenToWorld2D({(float)GetScreenWidth()*0,(float)GetScreenHeight()*0}, camera);
            selectEnd = GetScreenToWorld2D({(float)GetScreenWidth()*1,(float)GetScreenHeight()*1}, camera);
            float minX = fminf(selectStart.x, selectEnd.x);
            float maxX = fmaxf(selectStart.x, selectEnd.x);
            float minY = fminf(selectStart.y, selectEnd.y);
            float maxY = fmaxf(selectStart.y, selectEnd.y);
            for (int i = 0; i < num_units; i++) {
                Unit &u = units[i];
                if(u.faction!=factions) {u.selected=false; continue;} //only first faction can be selected
                float px = u.x * TILE_SIZE;
                float py = u.y * TILE_SIZE;
                u.selected = (px >= minX && px <= maxX && py >= minY && py <= maxY);
            }
        }

        // --- Fog of War computation ---
        for (int y = 0; y < GRID_SIZE; y++)
            for (int x = 0; x < GRID_SIZE; x++)
                visible[y][x] = false;

        // Vision radius in tile-units
        for (int i = 0; i < num_units; i++) {
            Unit &u = units[i];
            if (u.faction != &factions[0] && u.texture!=&tex::warehouse && u.texture!=&tex::esper && u.texture!=&tex::lighthouse) continue;  // only the player, warehouses, lighthouses, or espers give vision
            int ux = (int)u.x;
            int uy = (int)u.y;
            float extra_sight = terrainGrid[(int)u.y][(int)u.x].extra_sight;
            if((u.texture==&tex::railgun || u.texture==&tex::radio || u.texture==&tex::lighthouse) && extra_sight<0.f) extra_sight = 0.f;
            if(u.faction->technology&TECHNOLOGY_TRACK) extra_sight += 0.5f;
            float u_range = u.range*(1+extra_sight);
            if((u.texture==&tex::camp || u.texture==&tex::warehouse) && (u.faction->technology & TECHNOLOGY_EXPLORE) && u.faction==factions) u_range = 25.f;
            if(u.faction && (u.faction->technology & TECHNOLOGY_INFRASTRUCTURE) && (u.texture==&tex::radio || u.texture==&tex::fort)) u_range *= 2.0f;
            int VISION_RADIUS = (int)u_range+2;
            for (int dy = -VISION_RADIUS; dy <= VISION_RADIUS; dy++) {
                for (int dx = -VISION_RADIUS; dx <= VISION_RADIUS; dx++) {
                    int vx = ux + dx;
                    int vy = uy + dy;
                    if (vx < 0 || vy < 0 || vx >= GRID_SIZE || vy >= GRID_SIZE) continue;
                    if (dx*dx + dy*dy <= VISION_RADIUS * VISION_RADIUS) {
                        if(SHOW_MINIMAP && !explored[vy][vx]) {
                            BeginTextureMode(minimap);
                            Color c = ColorForTile(terrainGrid[vy][vx].texture);
                            DrawRectangle(vx * MINIMAP_SCALE, vy * MINIMAP_SCALE, MINIMAP_SCALE, MINIMAP_SCALE, c);
                            EndTextureMode();
                        }
                        visible[vy][vx] = true;
                        explored[vy][vx] = true;
                    }
                }
            }
        }

        #include "src/inline/ai_tech.cpp"
        #include "src/inline/ai_process.cpp"

        // drawing
        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode2D(camera);
        for (int y = yMin; y < yMax; y++)
            for (int x = xMin; x < xMax; x++) {
                int px = x * TILE_SIZE - TILE_SIZE/2;
                int py = y * TILE_SIZE - TILE_SIZE/2;
                if(!explored[y][x]) continue;
                if(terrainGrid[y][x].texture==&tex::water) continue;
                DrawTexture(*terrainGrid[y][x].texture, px, py, WHITE);
            }

        BeginShaderMode(waterShader);
        float tsec = GetTime()*2.f;
        SetShaderValue(waterShader, waterTimeLoc, &tsec, SHADER_UNIFORM_FLOAT);
        for (int y = yMin; y < yMax; y++)
            for (int x = xMin; x < xMax; x++) {
                int px = x * TILE_SIZE - TILE_SIZE/2;
                int py = y * TILE_SIZE - TILE_SIZE/2;
                if(!explored[y][x]) continue;
                if(terrainGrid[y][x].texture!=&tex::water) continue;
                DrawTexture(tex::water, px, py, WHITE);
            }
        EndShaderMode();


        for (int y = yMin; y < yMax; y++)
            for (int x = xMin; x < xMax; x++) {
                if (!explored[y][x]) continue;
                Texture2D* tx = terrainGrid[y][x].texture;
                int px = x * TILE_SIZE - TILE_SIZE/2;
                int py = y * TILE_SIZE - TILE_SIZE/2;

                bool hasN = (y > 0);
                bool hasS = (y < GRID_SIZE - 1);
                bool hasW = (x > 0);
                bool hasE = (x < GRID_SIZE - 1);

                Texture2D* n  = hasN ? terrainGrid[y-1][x].texture : tx;
                Texture2D* s  = hasS ? terrainGrid[y+1][x].texture : tx;
                Texture2D* w  = hasW ? terrainGrid[y][x-1].texture : tx;
                Texture2D* e  = hasE ? terrainGrid[y][x+1].texture : tx;

                if (tx==&tex::water) {
                    bool sN  = IsGrass(n);
                    bool sS  = IsGrass(s);
                    bool sW  = IsGrass(w);
                    bool sE  = IsGrass(e);
                    Texture2D &transition = (x+y)%2?tex::grass_transition:tex::grass_transition2;
                    if (sN && sW && !sE && !sS) DrawRot(transition,  px, py,   0);
                    if (sN && sE && !sW && !sS) DrawRot(transition, px, py,  90);
                    if (sS && sW && !sE && !sN) DrawRot(transition,  px, py, 270);
                    if (sS && sE && !sW && !sN) DrawRot(transition, px, py, 180);
                }
                if (!IsHill(tx) && !IsMountain(tx)) {
                    bool sN  = IsHill(n);
                    bool sS  = IsHill(s);
                    bool sW  = IsHill(w);
                    bool sE  = IsHill(e);
                    Texture2D *transition = (x+y)%2?&tex::hill_transition:&tex::hill_transition2;

                    bool drawn = false;
                    if (sN && sW && !sE && !sS) {
                        DrawRot(*transition,  px, py,0);
                        drawn=true;
                    }
                    if(sN && sW) {
                        if(x<GRID_SIZE-1) DrawRot(tex::hill_wedge,  px+TILE_SIZE, py,0);
                        if(y<GRID_SIZE-1) DrawRot(tex::hill_wedge,  px, py+TILE_SIZE,0);
                    }
                    if(sN && sE && !sW && !sS) {
                        DrawRot(*transition, px, py, 90);
                        drawn=true;
                    }
                    if(sN && sE) {
                        if(y<GRID_SIZE-1) DrawRot(tex::hill_wedge,  px, py+TILE_SIZE,90);
                        if(x) DrawRot(tex::hill_wedge,  px-TILE_SIZE, py,90);
                    }
                    if (sS && sW && !sE && !sN) {
                        DrawRot(*transition,  px, py,270);
                        drawn=true;
                    }
                    if(sS && sW) {
                        if(x) DrawRot(tex::hill_wedge,  px, py-TILE_SIZE,270);
                        if(x<GRID_SIZE-1)DrawRot(tex::hill_wedge,  px+TILE_SIZE, py,270);
                    }
                    if (sS && sE && !sW && !sN) {
                        DrawRot(*transition, px, py, 180);
                        drawn=true;
                    }
                    if(sS && sE) {
                        if(x) DrawRot(tex::hill_wedge,  px-TILE_SIZE, py,180);
                        if(y) DrawRot(tex::hill_wedge,  px, py-TILE_SIZE,180);
                    }
                    if(!drawn) {
                        if(sN) DrawRot(tex::hill_outline, px, py, 90);
                        if(sE) DrawRot(tex::hill_outline, px, py, 180);
                        if(sW) DrawRot(tex::hill_outline, px, py, 0);
                        if(sS) DrawRot(tex::hill_outline, px, py, 270);
                    }
                }
                if (!IsDesert(tx) && !IsHill(tx) && !IsMountain(tx)) {
                    bool sN  = IsDesert(n);
                    bool sS  = IsDesert(s);
                    bool sW  = IsDesert(w);
                    bool sE  = IsDesert(e);
                    Texture2D &transition = (x+y)%2?tex::desert_transition:tex::desert_transition;
                    if (sN && sW && !sE && !sS) DrawRot(transition,  px, py,   0);
                    if (sN && sE && !sW && !sS) DrawRot(transition, px, py,  90);
                    if (sS && sW && !sE && !sN) DrawRot(transition,  px, py, 270);
                    if (sS && sE && !sW && !sN) DrawRot(transition, px, py, 180);
                }
                if (!IsMountain(tx)) {
                    bool sN  = IsMountain(n);
                    bool sS  = IsMountain(s);
                    bool sW  = IsMountain(w);
                    bool sE  = IsMountain(e);
                    Texture2D *transition = (x+y)%2?&tex::mountain_transition:&tex::mountain_transition2;

                    bool drawn = false;
                    if (sN && sW && !sE && !sS) {
                        DrawRot(*transition,  px, py,0);
                        drawn=true;
                    }
                    if(sN && sW) {
                        if(x<GRID_SIZE-1) DrawRot(tex::hill_wedge,  px+TILE_SIZE, py,0);
                        if(y<GRID_SIZE-1) DrawRot(tex::hill_wedge,  px, py+TILE_SIZE,0);
                    }
                    if(sN && sE && !sW && !sS) {
                        DrawRot(*transition, px, py, 90);
                        drawn=true;
                    }
                    if(sN && sE) {
                        if(y<GRID_SIZE-1) DrawRot(tex::hill_wedge,  px, py+TILE_SIZE,90);
                        if(x) DrawRot(tex::hill_wedge,  px-TILE_SIZE, py,90);
                    }
                    if (sS && sW && !sE && !sN) {
                        DrawRot(*transition,  px, py,270);
                        drawn=true;
                    }
                    if(sS && sW) {
                        if(x) DrawRot(tex::hill_wedge,  px, py-TILE_SIZE,270);
                        if(x<GRID_SIZE-1)DrawRot(tex::hill_wedge,  px+TILE_SIZE, py,270);
                    }
                    if (sS && sE && !sW && !sN) {
                        DrawRot(*transition, px, py, 180);
                        drawn=true;
                    }
                    if(sS && sE) {
                        if(x) DrawRot(tex::hill_wedge,  px-TILE_SIZE, py,180);
                        if(y) DrawRot(tex::hill_wedge,  px, py-TILE_SIZE,180);
                    }
                    if(!drawn) {
                        if(sN) DrawRot(tex::hill_outline, px, py, 90);
                        if(sE) DrawRot(tex::hill_outline, px, py, 180);
                        if(sW) DrawRot(tex::hill_outline, px, py, 0);
                        if(sS) DrawRot(tex::hill_outline, px, py, 270);
                    }
                }

            }


        // --- UNDER UNIT LAYER ---
        Color target_line_color = Fade(BLUE, 0.3f);
        Color shadow_color = Fade(BLACK, 0.25f);
        // draw blood and explosion remnants
        for (int i = 0; i < num_units; i++) {
            Unit &u = units[i];
            int ux = (int)u.x;
            int uy = (int)u.y;
            int range = (int)u.size + 1;
            if (ux < xMin-range || ux >= xMax+range) continue;
            if (uy < yMin-range || uy >= yMax+range) continue;
            if (!visible[uy][ux]) continue;
            float px = u.x * TILE_SIZE;
            float py = u.y * TILE_SIZE;
            //float radius = (u.size * TILE_SIZE * 5);
            if(u.faction) continue;
            Texture2D* tex = u.texture;
            Rectangle src = { 0, 0, (float)tex->width, (float)tex->height };
            Rectangle dst = { px, py, TILE_SIZE * u.size, TILE_SIZE * u.size };
            Vector2 origin = { dst.width / 2.0f, dst.height / 2.0f };
            DrawTexturePro(*tex, src, dst, origin, u.angle, WHITE);
        }
        // Draw field units
        for(int faction_id=0;faction_id<max_factions;++faction_id) {
            Color fc = factions[faction_id].color;
            float fcf[4] = { fc.r/255.0f, fc.g/255.0f, fc.b/255.0f, 1.0f };
            BeginShaderMode(unitShader);
            SetShaderValue(unitShader, factionColorLoc, fcf, SHADER_UNIFORM_VEC4);
            for (int i = 0; i < num_units; i++) {
                Unit &u = units[i];
                if(u.texture!=&tex::field && u.texture!=&tex::field_little && u.texture!=&tex::field_empty && u.texture!=&tex::hide) continue;
                if(u.faction!=factions+faction_id) continue;
                int ux = (int)u.x;
                int uy = (int)u.y;
                int range = (int)u.size+1;
                if (ux < xMin-range || ux >= xMax+range) continue;
                if (uy < yMin-range || uy >= yMax+range) continue;
                if (visible[uy][ux]) {}
                else if (!factions[0].visible_knowledge[i]) continue;
                float px = u.x * TILE_SIZE;
                float py = u.y * TILE_SIZE;
                Texture2D* tex = u.texture;
                Rectangle src = { 0, 0, (float)tex->width, (float)tex->height };
                Rectangle dst = { px, py, TILE_SIZE * u.size, TILE_SIZE * u.size };
                Vector2 origin = { dst.width / 2.0f, dst.height / 2.0f };
                DrawTexturePro(*tex, src, dst, origin, u.angle, WHITE);
            }
            EndShaderMode(); // unitShader
        }
        // draw bases
        for (int i = 0; i < num_units; i++) {
            Unit &u = units[i];
            int ux = (int)u.x;
            int uy = (int)u.y;
            int range = (int)u.size + 1;
            if (ux < xMin-range || ux >= xMax+range) continue;
            if (uy < yMin-range || uy >= yMax+range) continue;
            if (!visible[uy][ux]) continue;
            float px = u.x * TILE_SIZE;
            float py = u.y * TILE_SIZE;
            float radius = (u.size * TILE_SIZE * 0.7f);
            if(!u.faction) continue;
            if (u.speed)
                DrawCircle(px, py, radius, u.selected?Fade(ColorBrightness(u.faction->color, -0.35f), 0.5f):shadow_color);//u.health < u.max_health?shadow_color_damaged:shadow_color);
            if (u.health < u.max_health) {
                int maxHealth = (int)(u.max_health+0.01f);
                int hp = (int)(u.health+0.01);
                float startBase = 0;//u.angle-45.f+180.f;
                float segmentAngle = 360.0f / maxHealth;  // 18°
                float gap = 75.f/maxHealth;
                Color hc = u.capturing?ColorBrightness(u.faction->color, 0.35f):RED;
                //Color hc = u.capturing?YELLOW:RED;
                float outer = radius * 0.99f;
                float inner = outer + (40.f*(0.82+(0.17*cos(t*8)+0.17f)*0.5f)- 40.f)/camera.zoom;
                // Draw one slice per health point
                for (int h = 0; h < hp; h++) {
                    float a0 = startBase + h * segmentAngle + gap * 0.5f;
                    float a1 = startBase + (h + 1) * segmentAngle - gap * 0.5f;
                    DrawRing({px, py},inner,outer,a0,a1,32,hc);
                }
            }
            if (u.selected && (u.target_x != 0 || u.target_y != 0)) {
                float tx = u.target_x * TILE_SIZE;
                float ty = u.target_y * TILE_SIZE;
                DrawDashedLine(tx, ty, px, py, target_line_color);
            }
        }

        // Draw non-field units
        for(int faction_id=0;faction_id<max_factions;++faction_id) {
            Color fc = factions[faction_id].color;
            float fcf[4] = { fc.r/255.0f, fc.g/255.0f, fc.b/255.0f, 1.0f };
            BeginShaderMode(unitShader);
            SetShaderValue(unitShader, factionColorLoc, fcf, SHADER_UNIFORM_VEC4);
            for (int i = 0; i < num_units; i++) {
                Unit &u = units[i];
                if(u.texture==&tex::field) continue;
                if(u.texture==&tex::field_little) continue;
                if(u.texture==&tex::field_empty) continue;
                if(u.texture==&tex::hide) continue;
                if(u.faction!=factions+faction_id) continue;
                int ux = (int)u.x;
                int uy = (int)u.y;
                int range = (int)u.size+1;
                if (ux < xMin-range || ux >= xMax+range) continue;
                if (uy < yMin-range || uy >= yMax+range) continue;
                if (visible[uy][ux]) {}
                else if (!factions[0].visible_knowledge[i]) continue;
                float px = u.x * TILE_SIZE;
                float py = u.y * TILE_SIZE;
                Texture2D* tex = u.texture;
                Rectangle src = { 0, 0, (float)tex->width, (float)tex->height };
                Rectangle dst = { px, py, TILE_SIZE * (u.size+u.extra_scale), TILE_SIZE * (u.size+u.extra_scale) };
                Vector2 origin = { dst.width / 2.0f, dst.height / 2.0f };
                DrawTexturePro(*tex, src, dst, origin, u.angle, WHITE);
            }
            EndShaderMode(); // unitShader
        }

        for (int i = 0; i < num_units; i++) {
            Unit &u = units[i];
            if (u.attack_target_x == 0 && u.attack_target_y == 0) continue;
            if (!visible[(int)(u.attack_y)][(int)(u.attack_x)]) continue;
            float px = u.attack_x * TILE_SIZE;
            float py = u.attack_y * TILE_SIZE;
            DrawCircle(px, py, 4.0f*(1+sqrtf(u.damage)*0.5f), RED);
        }

        Color transparent = Fade(WHITE, 0.55f);
        for (int i = 0; i < num_decorators; i++) {
            Decorator &d = decorators[i];
            int x = (int)d.x;
            int y = (int)d.y;
            if (x < xMin || x >= xMax) continue;
            if (y < yMin || y >= yMax) continue;
            if (!explored[y][x]) continue;
            float px = d.x * TILE_SIZE;
            float py = d.y * TILE_SIZE;
            Texture2D* tex = d.texture;

            Rectangle src = { 0, 0, (float)tex->width, (float)tex->height };
            Rectangle dst = {
                px,
                py,
                TILE_SIZE * d.size,
                TILE_SIZE * d.size
            };
            Vector2 origin = { 0, 0 };
            DrawTexturePro(*tex, src, dst, origin, 0.0f, visible[y][x]?transparent:WHITE);
        }


        // {
        //     Vector2 worldMouse = GetScreenToWorld2D(GetMousePosition(), camera);
        //     int x = (int)(worldMouse.x / (float)TILE_SIZE + 0.5f);
        //     int y = (int)(worldMouse.y / (float)TILE_SIZE + 0.5f);
        //     if (x >= 0 && y >= 0 && x < GRID_SIZE && y < GRID_SIZE) {
        //         Terrain &ter = terrainGrid[y][x];
        //         float px = x * TILE_SIZE - TILE_SIZE/2;
        //         float py = y * TILE_SIZE - TILE_SIZE/2;
        //         Color bg = Fade(BLACK, 0.55f);
        //         DrawRectangle(px, py, TILE_SIZE, TILE_SIZE, bg);
        //         DrawRectangleLines(px, py, TILE_SIZE, TILE_SIZE, Fade(WHITE, 0.4f));
        //         int fontSize = 24;
        //         int pad = 4;
        //         DrawText(TextFormat("Cover: %.0f%", ter.speed>0.f?(1.0-ter.speed)*100:0.f), px + pad, py + pad, fontSize, WHITE);
        //         DrawText(TextFormat("Sight: +%.0f%", ter.extra_sight*100), px + pad, py + pad + fontSize + 2, fontSize,WHITE);
        //     }
        // }



        EndMode2D(); // camera

        // Create fog mask
        BeginTextureMode(fog_mask);
        ClearBackground(Fade(BLACK, 0.5));
        for (int i = 0; i < num_units; i++) {
            Unit &u = units[i];
            if (u.faction != &factions[0] && u.texture!=&tex::warehouse && u.texture!=&tex::esper) continue;
            Vector2 world = { u.x * TILE_SIZE, u.y * TILE_SIZE };
            Vector2 screen = GetWorldToScreen2D(world, camera);
            float extra_sight = terrainGrid[(int)u.y][(int)u.x].extra_sight;
            if((u.texture==&tex::railgun || u.texture==&tex::radio || u.texture==&tex::lighthouse) && extra_sight<0.f) extra_sight = 0.f;
            if(u.faction->technology&TECHNOLOGY_TRACK) extra_sight += 0.7f;
            float u_range = u.range*(1+extra_sight);
            // important that here we extend the sight range only for stuff controlled by the player
            if((u.texture==&tex::camp || u.texture==&tex::warehouse) && (u.faction->technology & TECHNOLOGY_EXPLORE) && u.faction==factions) u_range = 25.f;
            if(u.faction && (u.faction->technology & TECHNOLOGY_INFRASTRUCTURE) && (u.texture==&tex::radio || u.texture==&tex::fort)) u_range *= 2.0f;
            float radiusPixels = (u_range * TILE_SIZE) * camera.zoom;
            Rectangle src = { 0, 0, (float)fog_hole.width, (float)fog_hole.height };
            Rectangle dst = {
                screen.x,
                screen.y,
                radiusPixels * 2.0f,
                radiusPixels * 2.0f
            };
            Vector2 origin = { radiusPixels, radiusPixels };
            DrawTexturePro(fog_hole, src, dst, origin, 0.f, WHITE);
        }
        float fogTileRadius = TILE_SIZE * camera.zoom * 1.5f;
        for (int y = yMin; y < yMax; y++) {
            for (int x = xMin; x < xMax; x++) {
                if (explored[y][x]) continue;
                bool hasExploredNeighbor = false;
                for (int ny = y - 1; ny <= y + 1 && !hasExploredNeighbor; ny++) {
                    for (int nx = x - 1; nx <= x + 1 && !hasExploredNeighbor; nx++) {
                        if (nx < 0 || ny < 0 || nx >= GRID_SIZE || ny >= GRID_SIZE) continue;
                        if (explored[ny][nx]) hasExploredNeighbor = true;
                    }
                }
                if (!hasExploredNeighbor) continue;
                float wx = x * TILE_SIZE + TILE_SIZE * 0.5f-fogTileRadius/2;
                float wy = y * TILE_SIZE + TILE_SIZE * 0.5f-fogTileRadius/2;
                Vector2 screen = GetWorldToScreen2D({wx, wy}, camera);
                DrawCircle(screen.x, screen.y, fogTileRadius, BLACK);
            }
        }


        EndTextureMode();

        // draw fow on screen coords
        BeginBlendMode(BLEND_MULTIPLIED);
        DrawTexturePro(
            fog_mask.texture,
            { 0, 0, (float)fog_mask.texture.width, -(float)fog_mask.texture.height },
            { 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() },
            { 0, 0 },
            0.0f,
            WHITE
        );
        EndBlendMode(); // multiplied

        // --------------------------------------------------
        // POPUPS — TOPMOST UI LAYER (WHITE)
        // --------------------------------------------------
        for (int i = 0; i < num_units; i++) {
            Unit &u = units[i];
            if (!u.popup) continue;
            if (u.health <= 0) continue;
            int ux = (int)u.x;
            int uy = (int)u.y;
            if (!visible[uy][ux]) continue;

            // Fade + rise animation
            float t = u.animation*2.f;
            if(t>1.f) continue;          // 0..1
            float alpha = 1.0f - t;
            float rise  = t * 40.0f+20.f;

            // World → screen
            Vector2 world = {
                u.x * TILE_SIZE,
                u.y * TILE_SIZE - (u.size * TILE_SIZE * 0.6f)
            };
            Vector2 screen = GetWorldToScreen2D(world, camera);

            // Cull if fully off-screen
            if (screen.x < -200 || screen.x > GetScreenWidth() + 200 ||
                screen.y < -200 || screen.y > GetScreenHeight() + 200)
                continue;

            Color c = Fade(WHITE, alpha);

            Vector2 pos = {
                screen.x - MeasureText(u.popup, 28) * 0.5f,
                screen.y - rise
            };

            Color outline = Fade(BLACK, c.a); // outline respects fade

            // outline (4 directions)
            DrawText(u.popup, pos.x - 1, pos.y,     32, outline);
            DrawText(u.popup, pos.x + 1, pos.y,     32, outline);
            DrawText(u.popup, pos.x,     pos.y - 1, 32, outline);
            DrawText(u.popup, pos.x,     pos.y + 1, 32, outline);

            // main text
            DrawText(u.popup, pos.x, pos.y, 32, c);
            if(u.popup_texture) DrawTextureEx(*u.popup_texture, {screen.x-20.f, pos.y-40}, 0, 40.f / u.popup_texture->width, WHITE);
        }

        // ---------------------
        // MINIMAP DRAWING
        // ---------------------
        if(!showTechTree && SHOW_MINIMAP)
        {
            float miniSize = 330.0f;   // onscreen size
            float padding = 20.0f;
            Rectangle src = {0,0,(float)minimap.texture.width,-(float)minimap.texture.height};
            Rectangle dst = {32,(float)GetScreenHeight() - miniSize - padding,miniSize,miniSize};
            DrawTexturePro(
                minimap.texture,
                src,
                dst,
                {0,0},
                0.0f,
                WHITE
            );
            DrawRectangleLines(dst.x, dst.y, dst.width, dst.height, WHITE);
            // ------------------------------------------------------------
            // DRAW CAMERA VIEWPORT OVER MINIMAP
            // ------------------------------------------------------------
            if(GRID_SIZE && TILE_SIZE){
                // 1. Determine world coords visible on screen
                Vector2 worldTL = GetScreenToWorld2D({0,0}, camera);
                Vector2 worldBR = GetScreenToWorld2D(
                    {(float)GetScreenWidth(), (float)GetScreenHeight()},
                    camera
                );

                // Convert world coords → tile coords
                float tileTLx = worldTL.x / TILE_SIZE;
                float tileTLy = worldTL.y / TILE_SIZE;
                float tileBRx = worldBR.x / TILE_SIZE;
                float tileBRy = worldBR.y / TILE_SIZE;

                // 2. Scale into minimap-space
                float scaleX = dst.width  / (float)GRID_SIZE;
                float scaleY = dst.height / (float)GRID_SIZE;

                float boxX = dst.x + tileTLx * scaleX;
                float boxY = dst.y + tileTLy * scaleY;
                float boxW = (tileBRx - tileTLx) * scaleX;
                float boxH = (tileBRy - tileTLy) * scaleY;

                if(boxX<dst.x) {
                    boxW -= (dst.x-boxX);
                    boxX = dst.x;
                }
                if(boxY<dst.y) {
                    boxH -= (dst.y-boxY);
                    boxY = dst.y;
                }
                if(boxX+boxW>dst.x+dst.width)
                    boxW = dst.x+dst.width-boxX;
                if(boxY+boxH>dst.y+dst.height)
                    boxH = dst.y+dst.height-boxY;
                DrawRectangleLines(boxX, boxY, boxW, boxH, BLUE);
            }
        }

        if (showTechTree) {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.85f));
            DrawTechs(factions[0]);
        }

        if (showHelp) {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.85f));
            float px = GetScreenWidth()*0.5f-600;
            float py = GetScreenHeight()*0.5f-380;
            DrawRectangleRounded(Rectangle{px-5, py-3, 260, 60}, 0.2f, 8, Fade(WHITE, 0.5f));
            DrawText("What to do",px,py,52,BLACK);
            py += 80;
            DrawText("Your units capture stuff - explore to find more.",px,py,32,WHITE);
            py += 40;
            DrawText("Gather the most utopia points before pollution fills.",px,py,32,WHITE);
            py += 40;
            DrawText("Spawning stops if you exceed available industry.",px,py,32,WHITE);
            py += 40;
            DrawText("Select techs once research fills.",px,py,32,WHITE);
            py += 40;
            py += 250;
            DrawRectangleRounded(Rectangle{px-5, py-3, 260, 60}, 0.2f, 8, Fade(WHITE, 0.5f));
            DrawText("Controls",px,py,52,BLACK);
            py += 80;
            DrawText("Zoom",px,py,32,WHITE);
            DrawText("QE/scroll",px+200,py,32,WHITE);
            py += 40;
            DrawText("Camera",px,py,32,WHITE);
            DrawText("WASD/right drag",px+200,py,32,WHITE);
            py += 40;
            DrawText("Description",px,py,32,WHITE);
            DrawText("Mouse over units",px+200,py,32,WHITE);
            py += 40;
            DrawText("Select",px,py,32,WHITE);
            DrawText("Left drag for selection rectangle/shift selects all visible",px+200,py,32,WHITE);
            py += 40;
            DrawText("Move",px,py,32,WHITE);
            DrawText("Right click (units auto-attack and may stop when near target), SPACE to change move mode",px+200,py,32,WHITE);
            py += 40;
            DrawText("Build fort",px,py,32,WHITE);
            DrawText("Del (station selected humans and tanks with at least 20 industry in a proportionally strong fort)",px+200,py,32,WHITE);
        }


        // --------------------------------------------------
        // RESEARCH BUTTON
        // --------------------------------------------------
        float prog = factions[0].technology_progress;
        bool techHover = CheckCollisionPointRec(GetMousePosition(), techBtn);
        DrawRectangleRounded(techBtn,0.2f, 8,techHover ? Fade(DARKBLUE, 0.6f) : Fade(BLACK, 0.6f));
        static float prev_prog = 0.f;
        const char* title = (prog >= 1.0f) ? "New tech" : (showTechTree?"Back":"Research");
        if(prev_prog<1.f && prog >= 1.0f) {
            last_message_counter = 0.f;
            last_message = "New tech can be selected.";
        }
        prev_prog = prog;
        DrawText(title,techBtn.x + 20,techBtn.y + 14,32,WHITE);
        float padding = 20.0f;
        float barW = techBtn.width - padding * 2;
        float barH = 20.0f;
        float bx = techBtn.x + padding;
        float by = techBtn.y + techBtn.height - barH - 18.0f;
        DrawTechProgressBar(bx, by, barW, barH, prog);
        DrawRectangleRounded({techBtn.x+techBtn.width-70, techBtn.y+15, 50, 25},0.2f, 8, WHITE);
        DrawTextSmall("esc",techBtn.x+techBtn.width-65+5,techBtn.y+15,24,BLACK);


        // --------------------------------------------------
        // FORT BUTTON
        // --------------------------------------------------
        if(!showTechTree && fort_creation_num && (factions->technology&TECHNOLOGY_FORT)) {
            float prog = fort_creation_total_health/100.0;
            if(prog>=2) prog = 1;
            bool techHover = CheckCollisionPointRec(GetMousePosition(), fortBtn);
            DrawRectangleRounded(fortBtn,0.2f, 8,techHover ? Fade(DARKBLUE, 0.6f) : Fade(BLACK, 0.6f));
            const char* title = fort_creation_has_nearby ? "Cannot build" : (fort_creation_total_health>=100?"Build fort":"Select more");
            DrawText(title,fortBtn.x + 20,fortBtn.y + 14,32,WHITE);
            float padding = 20.0f;
            float barW = fortBtn.width - padding * 2;
            float barH = 20.0f;
            float bx = fortBtn.x + padding;
            float by = fortBtn.y + fortBtn.height - barH - 18.0f;
            DrawTechProgressBar(bx, by, barW, barH, prog);
            // if(!fort_creation_has_nearby && fort_creation_total_health>=100) {
            //     DrawRectangleRounded({fortBtn.x+fortBtn.width-70, fortBtn.y+15, 50, 25},0.2f, 8, WHITE);
            //     DrawTextSmall("del",fortBtn.x+fortBtn.width-65+5,fortBtn.y+15,24,BLACK);
            // }
        }
        if(!showTechTree && trench_creation_num && (factions->technology&TECHNOLOGY_TRENCHES)) {
            float prog = fort_creation_total_health/100.0;
            if(prog>=2) prog = 1;
            bool techHover = CheckCollisionPointRec(GetMousePosition(), trenchBtn);
            DrawRectangleRounded(trenchBtn,0.2f, 8,techHover ? Fade(DARKBLUE, 0.6f) : Fade(BLACK, 0.6f));
            DrawText("Entrench",trenchBtn.x + 50,trenchBtn.y + 14,32,WHITE);
            DrawTextureEx(tex::trenches, {trenchBtn.x + 20, trenchBtn.y + 14}, 0, 20 / tex::trenches.width, WHITE);
        }
        if(!showTechTree && turtle_creation_num && (factions->technology&TECHNOLOGY_TURTLING)) {
            float prog = fort_creation_total_health/100.0;
            if(prog>=2) prog = 1;
            bool techHover = CheckCollisionPointRec(GetMousePosition(), turtleBtn);
            DrawRectangleRounded(turtleBtn,0.2f, 8,techHover ? Fade(DARKBLUE, 0.6f) : Fade(BLACK, 0.6f));
            DrawText(TextFormat("Stack %d rocks", turtle_creation_num),turtleBtn.x + 50,turtleBtn.y + 14,32,WHITE);
            DrawTextureEx(tex::rock, {turtleBtn.x + 20, turtleBtn.y + 14}, 0, 20 / tex::rock.width, WHITE);
        }

        // --------------------------------------------------
        // EXIT APP BUTTON
        // --------------------------------------------------
        if (showTechTree) {
            Vector2 mouse = GetMousePosition();
            bool hover = CheckCollisionPointRec(mouse, optionsButton);
            DrawRectangleRounded(optionsButton, 0.2f, 8, hover ? Fade(RED, 0.65f) : Fade(BLACK, 0.6f));
            DrawText("Concede",
                     optionsButton.x + 20,
                     optionsButton.y + 15,
                     32,WHITE);
            if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                factions[0].victory_points = -factions[0].victory_points;
                showTechTree = false;
                goto GAME_OVER;
            }
        }
        else {
            Vector2 mouse = GetMousePosition();
            bool hover = CheckCollisionPointRec(mouse, helpBtn);
            DrawRectangleRounded(helpBtn, 0.2f, 8, hover ? Fade(DARKGRAY, 0.5f) : Fade(BLACK, 0.5f));
            DrawText(showHelp?"Close":"Info",
                     helpBtn.x + 20,
                     helpBtn.y + 15,
                     32,WHITE);

            DrawRectangleRounded({helpBtn.x+helpBtn.width-70, helpBtn.y+15, 50, 25},0.2f, 8, WHITE);
            DrawTextSmall("tab",helpBtn.x+helpBtn.width-65+5,helpBtn.y+15,24,BLACK);
        }

        if (last_message) {
            static int   visible_chars = 0;
            static float type_timer = 0.0f;
            static bool  finished = false;

            if(last_message_counter==0) {
                visible_chars = 0;
                type_timer = 0.0f;
                finished = false;
                last_message_counter = 1.0;
            }

            const float CHAR_DELAY   = 0.02f;   // typing speed
            const float HOLD_TIME    = 3.0f;    // wait after full text

            const char* msg = last_message;
            int msg_len = (int)strlen(msg);

            // Typewriter progression
            if (!finished) {
                type_timer += dt;
                while (type_timer > CHAR_DELAY && visible_chars < msg_len) {
                    visible_chars++;
                    type_timer -= CHAR_DELAY;
                }
                if (visible_chars >= msg_len) {
                    finished = true;
                    last_message_counter = HOLD_TIME;
                }
            } else {
                last_message_counter -= dt;
                if (last_message_counter <= 0.0f) {
                    last_message_counter = 0.0f;
                    last_message = nullptr;
                    visible_chars = 0;
                    type_timer = 0.0f;
                    finished = false;
                }
            }

            // Build visible string with cursor
            char buffer[256];
            int n = visible_chars;
            memcpy(buffer, msg, n);
            buffer[n] = finished ? '\0' : '|';
            buffer[n + (finished ? 0 : 1)] = '\0';

            float px = GetScreenWidth()/2 - MeasureText(msg, 70)/2 + 70;
            float py = GetScreenHeight()/2 - 35;
            DrawText(buffer, px-2, py-3, 70, BLACK);
            DrawText(buffer, px+2, py-2, 70, BLACK);
            DrawText(buffer, px-2, py+2, 70, BLACK);
            DrawText(buffer, px+2, py+2, 70, BLACK);
            DrawText(buffer, px,   py,   70, WHITE);
        }


        // draw ui
        if (selecting && !showTechTree) {
            Vector2 s = selectStart;
            Vector2 e = selectEnd;
            Vector2 ss = GetWorldToScreen2D(s, camera);
            Vector2 ee = GetWorldToScreen2D(e, camera);
            float minX = fminf(ss.x, ee.x);
            float maxX = fmaxf(ss.x, ee.x);
            float minY = fminf(ss.y, ee.y);
            float maxY = fmaxf(ss.y, ee.y);
            DrawRectangleLines(minX, minY, maxX - minX, maxY - minY, SKYBLUE);
            DrawRectangle(minX, minY, maxX - minX, maxY - minY, Fade(SKYBLUE, 0.15f));
        }

        if (!showTechTree) {
            int player_points = factions[0].victory_points;
            int best_other_points = 0;
            for (int fi = 3; fi < max_factions; fi++) // skip wild faction
                if (factions[fi].victory_points > best_other_points)
                    best_other_points = factions[fi].victory_points;
            float offset = 5.f;
            DrawTexturePro(
                tex::overlay,
                Rectangle{0,0,(float)tex::overlay.width,(float)tex::overlay.height},
                Rectangle{0, offset, 512, 230},
                {0,0},0, WHITE);
            char msg[256];
            snprintf(msg, sizeof(msg), "Utopia %d", player_points);
            //DrawText(msg, (GetScreenWidth() - MeasureText(msg, font_size)) / 2, 10, font_size, BLACK);
            DrawText(msg, 124, 102+offset, 42, WHITE);
            snprintf(msg, sizeof(msg), "%d top AI", best_other_points);
            DrawText(msg, 330, 102+offset, 42, WHITE);
            DrawTexturePro(
                tex::utopia,
                Rectangle{0,0,(float)tex::utopia.width,(float)tex::utopia.height},
                           Rectangle{20, offset+85, 90, 90},
                           {0,0}, 0, WHITE);


            // ======================================================
            // POLLUTION BAR (GAME TIMER)
            // ======================================================
            {
                // time_norm assumed in [0..1], where 1 = end of game
                float pollution = time_norm;
                if (pollution < 0.0f) pollution = 0.0f;
                if (pollution > 1.0f) pollution = 1.0f;

                Rectangle bar_bg = { 12, offset+18, 490, 52 };
                Rectangle bar_fg = bar_bg;
                bar_fg.width *= pollution;

                Color bg = Color{ 40, 120, 200, 128 };
                Color fg =
                (pollution < 0.7f) ? DARKGRAY :
                (pollution < 0.9f) ? ORANGE :
                RED;

                DrawRectangleRec(bar_bg, bg);
                DrawRectangleRec(bar_fg, fg);

                if(polution_speedup<-0.2f)
                    DrawText("Pollution slowed down", bar_bg.x + 20, bar_bg.y + 10, 32, WHITE);
                else if(polution_speedup>0.2f)
                    DrawText("Pollution sped up from industry", bar_bg.x + 20, bar_bg.y + 10, 32, WHITE);
                else
                    DrawText("World pollution", bar_bg.x + 20, bar_bg.y + 10, 32, WHITE);
                DrawRectangleLinesEx(bar_bg, 8.0f, BLACK);
            }

            offset += 30;
            {
                int fi = 0;
                DrawTexturePro(tex::banner,Rectangle{0,0,(float)tex::banner.width,(float)tex::banner.height}, Rectangle{-20, 170+offset, 470, 48}, {0,0},0, factions[fi].color);
                if((int)factions[fi].count_members<factions[fi].industry)
                    snprintf(msg, sizeof(msg), "%d/%d industry (spawning)", (int)factions[fi].count_members, factions[fi].industry);
                else if((int)factions[fi].count_members==factions[fi].industry)
                    snprintf(msg, sizeof(msg), "%d/%d industry (cap)", (int)factions[fi].count_members, factions[fi].industry);
                else
                    snprintf(msg, sizeof(msg), "%d/%d industry (over cap)", (int)factions[fi].count_members, factions[fi].industry);
                DrawText(msg, 80, 182+offset, 24, WHITE);
                DrawTexturePro(tex::gear, Rectangle{0,0,(float)tex::gear.width,(float)tex::gear.height}, Rectangle{30, 180+offset+14, 32, 32}, {16,16}, game_time*6.f*factions[fi].industry, WHITE);
                DrawTexturePro(tex::gear, Rectangle{0,0,(float)tex::gear.width,(float)tex::gear.height}, Rectangle{30+24, 180+offset+14, 32, 32}, {16,16}, -game_time*6.f*factions[fi].industry-30, WHITE);
            }

            for (int fi = 3; fi < max_factions; fi++) {
                if((int)factions[fi].count_members<=factions[fi].industry) continue;
                offset += 52;
                DrawTexturePro(tex::banner,Rectangle{0,0,(float)tex::banner.width,(float)tex::banner.height}, Rectangle{-20, 170+offset, 470, 48}, {0,0},0, factions[fi].color);
                snprintf(msg, sizeof(msg), "%d/%d AI industry (over cap)", (int)factions[fi].count_members, factions[fi].industry);
                DrawText(msg, 80, 182+offset, 24, WHITE);
                DrawTexturePro(tex::gear, Rectangle{0,0,(float)tex::gear.width,(float)tex::gear.height}, Rectangle{30, 180+offset+14, 32, 32}, {16,16}, game_time*6.f*factions[fi].industry, WHITE);
                DrawTexturePro(tex::gear, Rectangle{0,0,(float)tex::gear.width,(float)tex::gear.height}, Rectangle{30+24, 180+offset+14, 32, 32}, {16,16}, -game_time*6.f*factions[fi].industry-30, WHITE);
            }

        }

        static Unit* lasthovered = nullptr;
        static float hoverdelay = 0.f;
        if(lasthovered!=hovered) {
            hoverdelay += dt;
            if(hoverdelay>0.1f) {
                hoverdelay = 0.f;
                lasthovered = hovered;
            }
            else hovered = lasthovered;
        }
        else hoverdelay = 0.f;
        if(!showTechTree && !hovered && hoveredTerrain && has_any_selected) {
            #include "src/inline/draw_terrain_tooltip.cpp"
        }
        if(!showTechTree && hovered) {
            #include "src/inline/draw_tooltip.cpp"
        }
        EndDrawing();
    }

    free(terrainBlock);
    free(terrainGrid);
    unload();
    return 0;
}
