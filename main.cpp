#include <raylib.h>
#include <cmath>
#include <iostream>
#include <bitset>
#include <cstring>

#define DrawText(txt, x, y, size, col) DrawTextEx(uiFont, txt, (Vector2){ (float)(x), (float)(y) }, (float)(size), 2.0f, col)
#define DrawTextSmall(txt, x, y, size, col) DrawTextEx(smallerFont, txt, (Vector2){ (float)(x), (float)(y) }, (float)(size), 1.0f, col)
static const int GRID_SIZE = 196;
static const int MAX_UNITS = 80000; // up to 200kb worth of units
static const int MAX_DECORATORS = 1000000;
static const int TILE_SIZE = 128;
static const float CAMERA_ZOOM = 0.8f;
static const float movement_speed_multiplier = 0.5f;

// difficulty controls
static const float CAMP_SPAWN_RATE = 2.f; //
static const float ANIMAL_SPAWN_RATE = 0.1f; // expected spawns per second
static const float CAPTURE_RATE = 1.0f; // also heals espers and partial captures
static const float OVER_CAP_REGEN_RATE = 0.3f; // 1.f is normal restoration from units being captured, this is lower if we have over-saturated industry


Font uiFont;
Font smallerFont;
namespace tex {
    static Texture2D grass;
    static Texture2D grass2;
    static Texture2D grass3;
    static Texture2D grass4;
    static Texture2D grass_transition;
    static Texture2D grass_transition2;
    static Texture2D water;
    static Texture2D desert;
    static Texture2D desert_transition;
    static Texture2D hill;
    static Texture2D hill2;
    static Texture2D hill3;
    static Texture2D hill4;
    static Texture2D hill_transition;
    static Texture2D hill_transition2;
    static Texture2D hill_outline;
    static Texture2D hill_wedge;
    static Texture2D mountain;
    static Texture2D mountain_transition;
    static Texture2D mountain_transition2;
    static Texture2D road;
    static Texture2D road_transition;
    static Texture2D human;
    static Texture2D esper;
    static Texture2D scout;
    static Texture2D tank;
    static Texture2D datacenter;
    static Texture2D railgun;
    static Texture2D bison;
    static Texture2D wolf;
    static Texture2D hide;
    static Texture2D heal;
    static Texture2D field;
    static Texture2D field_little;
    static Texture2D field_empty;
    static Texture2D mine;
    static Texture2D camp;
    static Texture2D lab;
    static Texture2D warehouse;
    static Texture2D overlay;
    static Texture2D banner;
    static Texture2D blood;
    static Texture2D ghost;
    static Texture2D crater;
    static Texture2D radio;
    static Texture2D info;
    static Texture2D oil;
    static Texture2D snow;
    static Texture2D tree;
    static Texture2D snowman;
    // static Texture2D helicopter;
    static Texture2D van;
    // static Texture2D hospital;
    // static Texture2D yeti;
    // static Texture2D wolf;
    // static Texture2D rat;
    // static Texture2D bison;
    static Texture2D gear;
    static Texture2D utopia;
    static Texture2D track;
    static Texture2D sun;
    static Texture2D reward;
    static Texture2D earth;
    static Texture2D research;
    static Texture2D rat;
    static Texture2D roomba;
}

struct SoundPool {
    Sound* voices;
    int next = 0;
    int voiceCount = 0;
    void Load(const char* path, int _voiceCount) {
        voiceCount = _voiceCount;
        voices = (Sound*)malloc(voiceCount*sizeof(Sound));
        for(int i=0; i<voiceCount; i++) voices[i] = LoadSound(path);
        next = 0;
    }
    void Unload() {
        for(int i=0; i<voiceCount; i++) UnloadSound(voices[i]);
        delete voices;
        voiceCount = 0;
    }
    void Play(float volume) {
        if(!voiceCount) return;
        int idx = -1;
        for(int i = 0;i < voiceCount;i++)
            if (!IsSoundPlaying(voices[i])) {
                idx = i;
                break;
            }
        if(idx < 0) idx = next;
        float pitch = GetRandomValue(95, 105) / 100.0f; // 0.95 -> 1.05
        SetSoundPitch(voices[idx], pitch);
        SetSoundVolume(voices[idx], volume);
        PlaySound(voices[idx]);
        next = (next+1)%voiceCount;
    }
};

namespace sound {
    static SoundPool boom;
    static SoundPool explosion;
    static SoundPool gun;
    static SoundPool damage;
    static SoundPool dead;
    static SoundPool ohm;
    static SoundPool moo;
    static SoundPool noise;
    static SoundPool notify;
    static Sound select;
    static Sound select2;

    static Music bg;
}


struct RankDef {
    const char* name;
    double min;
    double max;
    Texture2D* tex;
    Color color;
};

RankDef ranks[] = {
    {"STRUGGLER",  1200.0, 1400.0, &tex::gear, BROWN},
    {"SURVIVOR",   1400.0, 1600.0, &tex::gear, ORANGE},
    {"LEADER",     1600.0, 1800.0, &tex::gear, YELLOW},
    {"UTOPIC",     1800.0, 2200.0, &tex::utopia, WHITE}
};

static void DrawTechProgressBar(
    float x, float y,
    float w, float h,
    float progress   // 0..1
) {
    if (progress < 0.f) progress = 0.f;
    if (progress > 1.f) progress = 1.f;
    DrawRectangleRounded({x, y, w, h}, 0.3f, 8, Fade(DARKGRAY, 0.6f));
    DrawRectangleRounded({x + 2, y + 2, (w - 4) * progress, h - 4},0.3f, 8,Fade(GREEN, 0.85f));
    DrawRectangleRoundedLines({x, y, w, h}, 0.3f, 8, GRAY);
}


enum class MovementMode {
    Tight,
    Scattered,
    Explore
};


static Color ColorForTile(Texture* texture) {
    if (texture == &tex::water)
        return (Color){ 40, 120, 160, 255 };
    if (texture == &tex::grass || texture == &tex::grass2 ||
        texture == &tex::grass3 || texture == &tex::grass4)
        return (Color){ 60, 196, 40, 255 };
    if (texture == &tex::hill || texture == &tex::hill2 ||
        texture == &tex::hill3 || texture == &tex::hill4)
        return (Color){ 140, 80, 40, 255 };
    if (texture == &tex::desert)
        return (Color){ 196, 196, 64, 255 };
    if (texture == &tex::mountain)
        return (Color){ 90, 52, 24, 255 };
    return (Color){ 80, 80, 80, 255 };
}


#define TECHNOLOGY_TRACK            1ULL  // increased sight in low-visibility areas
#define TECHNOLOGY_EXPLORE          2ULL  // double scattering
#define TECHNOLOGY_MOBILE_FORTRESS  4ULL  // captured railguns become tanks
#define TECHNOLOGY_HOMUNCULI        8ULL  // killed bloos become allies
#define TECHNOLOGY_HUNTING          16ULL // double effectiveness of animal remains
#define TECHNOLOGY_INFRASTRUCTURE   32ULL // radio towers have the same sight as if they were in mountaintops
#define TECHNOLOGY_AGILE            64ULL // speed cannot be reduced to less than 70% (this is desert mobility)
#define TECHNOLOGY_SEAFARERING     128ULL // water increases instead of decreasing speed
#define TECHNOLOGY_FIGHT           256ULL // double unit experience
#define TECHNOLOGY_HEROICS         512ULL // heroes have +50% chance of dodging
#define TECHNOLOGY_REFINERY       1024ULL // oil also adds production
#define TECHNOLOGY_NERDS          2048ULL // +25% research speed, -1 health to your humans
#define TECHNOLOGY_RESEARCH       4096ULL // +25% research speed (stacks with nerds)
#define TECHNOLOGY_LUXURY        16384ULL // every camp grants an utopia point, half spawn rate
#define TECHNOLOGY_PROPAGANDA    32768ULL // every 2 radio stations grant an utopia point
#define TECHNOLOGY_SNIPING       65536ULL // increased hit chance in low-visibility areas
#define TECHNOLOGY_TOUGH        131072ULL // +1 health to your humans
#define TECHNOLOGY_UNSTABLE     262144ULL // -1 health to your humans, they turn into bloos on death
#define TECHNOLOGY_SPEEDY       524288ULL // your veterans and heroes are faster
#define TECHNOLOGY_GRIT        1048576ULL // 50% chance of dodging lethal damage
#define TECHNOLOGY_OWNERSHIP   2097152ULL // halves the attempts of enemy takeovers
#define TECHNOLOGY_SUPERIORITY 4194304ULL // 2 utopia, 25% of spawned units are hostile
#define TECHNOLOGY_SNIFFING    8388608ULL // idle units have a chance to move towards an unknown capturable location
#define TECHNOLOGY_MECHA      16777216ULL // 50% chance of mecha dodge
#define TECHNOLOGY_DRIVER     33554432ULL // mecha speed cannot be reduced
#define TECHNOLOGY_FARMING    67108864ULL // +3 industry from farms
#define TECHNOLOGY_HARDCORE  134217728ULL // -6 industry per camp, x2 spawn
#define TECHNOLOGY_AUTOREPAIRS 268435456ULL // mecha regeneration
#define TECHNOLOGY_BIOWEAPON   536870912ULL // kills become bloos
#define TECHNOLOGY_INDUSTRY   1073741824ULL // mechas can gain XP
#define TECHNOLOGY_NUCLEAR    2147483648ULL // double human damage, no regen
#define TECHNOLOGY_GIGAJOULE  4294967296ULL // no mecha cost to industry
#define TECHNOLOGY_REACTOR    8589934592ULL // +40 industry
#define TECHNOLOGY_EVOLUTION     17179869184ULL // 10% chance of spawning a snowman
#define TECHNOLOGY_ARTIFICIAL    34359738368ULL // bloos start as veterans (actual name in-game: HIVEMENIND)
#define TECHNOLOGY_TERRAFORIMING 68719476736ULL // anything you capture becomes farms
#define TECHNOLOGY_MECHANISED   137438953472ULL // 1 industry per 5 mecha health
#define TECHNOLOGY_HIJACK       274877906944ULL // hijack instead of destroying mecha
#define TECHNOLOGY_WONDER       549755813888ULL // new discoveries grant experience
#define TECHNOLOGY_HELLBRINGER 1099511627776ULL // rapid hero and veteran fire
#define TECHNOLOGY_TAMING      2199023255552ULL // defeated animals become allies
#define TECHNOLOGY_ATMOSPHERE  4398046511104ULL // fields slow down game end
#define TECHNOLOGY_HYPERMAGNET 8796093022208ULL // double industry cost and movement
#define TECHNOLOGY_AIFARM      17592186044416ULL // labs give +9 industry instead
#define TECHNOLOGY_TECHNOCRACY 35184372088832ULL // 1 utopia per 50 industry, lose half industry

#define PREFERENCE_RAILGUN     0
#define PREFERENCE_TANK        1
#define PREFERENCE_FARM        2
#define PREFERENCE_LAB         3
#define PREFERENCE_WAREHOUSE   4
#define PREFERENCE_SPACING     5
#define PREFERENCE_EXPERIENCE  6
#define PREFERENCE_SPEED       7
#define PREFERENCE_HUMAN       8
#define PREFERENCE_ROOMBA      9
#define PREFERENCE_ANIMAL     10
#define PREFERENCE_ESPER      11
#define PREFERENCE_COUNT      12


const char* preference_desc[PREFERENCE_COUNT] = {
    "near railguns",
    "near vehicle",
    "near fields",
    "near old tech",
    "near storage",
    "+camp spacing",
    "+starting vets",
    "+starting speed",
    "+5 humans",
    "near roombas",
    "near animals",
    "near esper"
};

Texture* preference_icon[PREFERENCE_COUNT] = {
    &tex::railgun,
    &tex::tank,
    &tex::field,
    &tex::lab,
    &tex::warehouse,
    &tex::camp,
    &tex::track,
    &tex::human,
    &tex::human,
    &tex::roomba,
    &tex::bison,
    &tex::esper
};

typedef unsigned long long PrefMask;
#define PREF_BIT(p) (1ULL << (p))
#define is_mecha(u) (u.texture==&tex::tank || u.texture==&tex::van || u.texture==&tex::railgun || u.texture==&tex::roomba)

struct Terrain {
    Texture2D* texture;
    float speed;
    float extra_sight;
};

struct Faction {
    Color color;
    const char* name;
    float victory_points;
    int industry;
    float count_members;
    std::bitset<MAX_UNITS> visible_knowledge;
    unsigned long long technology;
    float technology_progress;
};

struct Unit {
    Texture2D* texture;
    const char* name;
    float speed;
    float x,y;
    float attack_rate;
    float range;
    float damage;
    float experience;
    float angle;
    float size;
    float health;
    float max_health;
    Faction* faction;
    // leave the stuff below zero-initialized (target coords with zero values will just be skipped by convention)
    Faction* capturing; // set this to non-null to indicate that damage captures. in this case, nothing attacks it
    float extra_scale;
    float target_x, target_y;
    int selected; // only to mark selection by player
    float attack_x, attack_y;
    float attack_target_x, attack_target_y;
    float stunned;
    const char* popup;
    float animation;
};

int NOISE_SEED = 0;

#define CREATE_HUMAN(faction, x, y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = { \
            &tex::human,  /* texture */ \
            "Human",      /* name */ \
            5.0,         /* speed */ \
            (float)(x),   /* x */ \
            (float)(y),   /* y */ \
            1.0,          /* attack_rate */ \
            4.0,         /* range */ \
            1.0,          /* damage */ \
            0.0,          /* experience */ \
            (float)GetRandomValue(0,360),          /* angle */ \
            0.3,          /* size */ \
            5.0,          /* health */ \
            5.0,          /* max_health */ \
            (faction)     /* faction */ \
        };

#define CREATE_ESPER(faction, x, y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = { \
            &tex::esper,  /* texture */ \
            "Esper",      /* name */ \
            5.0,         /* speed */ \
            (float)(x),   /* x */ \
            (float)(y),   /* y */ \
            2.0,          /* attack_rate */ \
            4.0,         /* range */ \
            3.0,          /* damage */ \
            0.0,          /* experience */ \
            (float)GetRandomValue(0,360),          /* angle */ \
            0.4,          /* size */ \
            15.0,          /* health */ \
            15.0,          /* max_health */ \
            (faction),     /* faction */ \
            (faction),     /* capturable */ \
        };

#define CREATE_TANK(faction, x, y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = { \
            &tex::tank,   /* texture */ \
            "Tank",       /* name */ \
            3.0,          /* speed */ \
            (float)(x),   /* x */ \
            (float)(y),   /* y */ \
            0.5,          /* attack_rate */ \
            6.0,         /* range */ \
            8.0,          /* damage */ \
            0.0,          /* experience */ \
            (float)GetRandomValue(0,360),          /* angle */ \
            0.8,          /* size */ \
            20.0,         /* health */ \
            20.0,         /* max_health */ \
            (faction),    /* faction */ \
            (faction),    /* faction */ \
            0.3           /* extra scale*/\
        };

#define CREATE_ROOMBA(faction, x, y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = { \
            &tex::roomba, /* texture */ \
            "Roomba",     /* name */ \
            7.0,          /* speed */ \
            (float)(x),   /* x */ \
            (float)(y),   /* y */ \
            1.0,          /* attack_rate */ \
            3.0,          /* range */ \
            2.0,          /* damage */ \
            0.0,          /* experience */ \
            (float)GetRandomValue(0,360),          /* angle */ \
            0.4,          /* size */ \
            10.0,          /* health */ \
            10.0,          /* max_health */ \
            (faction),    /* faction */ \
            (faction),    /* faction */ \
            0.2           /* extra scale*/\
        };

#define CREATE_VAN(faction, x, y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = { \
            &tex::van,   /* texture */ \
            "Van",       /* name */ \
            9.0,          /* speed */ \
            (float)(x),   /* x */ \
            (float)(y),   /* y */ \
            1.0,          /* attack_rate */ \
            6.0,          /* range */ \
            1.0,          /* damage */ \
            0.0,          /* experience */ \
            (float)GetRandomValue(0,360),          /* angle */ \
            0.7,          /* size */ \
            20.0,         /* health */ \
            20.0,         /* max_health */ \
            (faction),    /* faction */ \
            (faction),    /* faction */ \
            0.3           /* extra scale*/\
        };


#define CREATE_SNOWMAN(faction, x, y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = { \
            &tex::snowman,   /* texture */ \
            "Snowman",    /* name */ \
            3.0,          /* speed */ \
            (float)(x),   /* x */ \
            (float)(y),   /* y */ \
            2.0,          /* attack_rate */ \
            2.5,          /* range */ \
            1.0,          /* damage */ \
            0.0,          /* experience */ \
            (float)GetRandomValue(0,360),          /* angle */ \
            0.6,          /* size */ \
            10.0,         /* health */ \
            10.0,         /* max_health */ \
            (faction),    /* faction */ \
            nullptr,      /* faction */ \
            0.2           /* extra scale*/\
        };

#define CREATE_BISON(faction, x, y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = { \
            &tex::bison,   /* texture */ \
            "Bison",       /* name */ \
            5.0,          /* speed */ \
            (float)(x),   /* x */ \
            (float)(y),   /* y */ \
            2.0,          /* attack_rate */ \
            1.0,          /* range */ \
            3.0,          /* damage */ \
            0.0,          /* experience */ \
            (float)GetRandomValue(0,360),          /* angle */ \
            0.6,          /* size */ \
            15.0,         /* health */ \
            15.0,         /* max_health */ \
            (faction),    /* faction */ \
            nullptr,      /* faction */ \
            0.3           /* extra scale*/\
        };


#define CREATE_RAT(faction, x, y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = { \
            &tex::rat,   /* texture */ \
            "Rat",       /* name */ \
            7.0,          /* speed */ \
            (float)(x),   /* x */ \
            (float)(y),   /* y */ \
            1.0,          /* attack_rate */ \
            1.0,          /* range */ \
            1.0,          /* damage */ \
            0.0,          /* experience */ \
            (float)GetRandomValue(0,360),          /* angle */ \
            0.2,          /* size */ \
            2.0,         /* health */ \
            2.0,         /* max_health */ \
            (faction),    /* faction */ \
            nullptr,      /* faction */ \
            0.0           /* extra scale*/\
        };


#define CREATE_WOLF(faction, x, y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = { \
            &tex::wolf,   /* texture */ \
            "Wolf",       /* name */ \
            9.0,          /* speed */ \
            (float)(x),   /* x */ \
            (float)(y),   /* y */ \
            1.0,          /* attack_rate */ \
            1.0,          /* range */ \
            3.0,          /* damage */ \
            0.0,          /* experience */ \
            (float)GetRandomValue(0,360),          /* angle */ \
            0.4,          /* size */ \
            5.0,          /* health */ \
            5.0,          /* max_health */ \
            (faction),    /* faction */ \
            nullptr,      /* faction */ \
            0.3           /* extra scale*/\
        };

#define CREATE_RAILGUN(faction, x, y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = { \
            &tex::railgun,   /* texture */ \
            "Railgun",    /* name */ \
            0.0,          /* speed */ \
            (float)(x),   /* x */ \
            (float)(y),   /* y */ \
            5.0,          /* attack_rate */ \
            6.0,         /* range */ \
            1.5,          /* damage */ \
            0.0,          /* experience */ \
            (float)GetRandomValue(0,360),          /* angle */ \
            0.7,          /* size */ \
            20.0,         /* health */ \
            20.0,         /* max_health */ \
            (faction),    /* faction */ \
            (faction),     /* faction */ \
            0.0           /* extra scale*/\
        };

#define CREATE_CAMP(faction, x, y) \
    if (num_units < MAX_UNITS) { \
        float bbx = x; \
        float bby = y; \
        units[num_units++] = { \
            &tex::camp,   /* texture */ \
            "Camp",       /* name */ \
            0.0,          /* speed */ \
            (float)(bbx),   /* x */ \
            (float)(bby),   /* y */ \
            4.0,          /* attack_rate (4 per min)*/ \
            1.0,          /* range */ \
            0.0,          /* damage */ \
            0.0,          /* experience */ \
            0.0,          /* angle */ \
            0.8,          /* size */ \
            50.0,         /* health */ \
            50.0,         /* max_health */ \
            (faction),    /* faction */ \
            (faction)     /* can only be captured */ \
        }; \
        for(int ppy=-2;ppy<=2;ppy++) \
            for(int ppx=-2;ppx<=2;ppx++) \
                if((ppy*ppy)+(ppx*ppx)<=4) terrainGrid[(int)(bby+0.5f)+ppy][(int)(bbx+0.5f)+ppx] = { &tex::grass, 1.0 }; \
    }

#define CREATE_FIELD(faction, x, y) \
        if (num_units < MAX_UNITS) { \
            float bbx = x; \
            float bby = y; \
            units[num_units++] = { \
                &tex::field,   /* texture */ \
                "Field",       /* name */ \
                0.0,          /* speed */ \
                (float)(bbx),   /* x */ \
                (float)(bby),   /* y */ \
                0.0,          /* attack_rate (store industry state here)*/ \
                4.0,          /* range */ \
                0.0,          /* damage */ \
                0.0,          /* experience */ \
                0.0,          /* angle */ \
                1.4,          /* size */ \
                50.0,         /* health */ \
                50.0,         /* max_health */ \
                (faction),    /* faction */ \
                (faction),    /* can only be captured */ \
                -0.1f \
            }; \
            for(int ppy=-2;ppy<=2;ppy++) \
                for(int ppx=-2;ppx<=2;ppx++) \
                    if((ppy*ppy)+(ppx*ppx)<=4) terrainGrid[(int)(bby+0.5f)+ppy][(int)(bbx+0.5f)+ppx] = { &tex::grass, 1.0 }; \
        }
#define CREATE_DATACENTER(faction, x, y) \
    if (num_units < MAX_UNITS) {\
        units[num_units++] = { \
            &tex::datacenter,   /* texture */ \
            "Databank",       /* name */ \
            0.0,          /* speed */ \
            (float)(x),   /* x */ \
            (float)(y),   /* y */ \
            0.0,          /* attack_rate (store industry state here)*/ \
            4.0,          /* range */ \
            0.0,          /* damage */ \
            0.0,          /* experience */ \
            0.0,          /* angle */ \
            2.0,          /* size */ \
            50.0,         /* health */ \
            50.0,         /* max_health */ \
            (faction),    /* faction */ \
            (faction),    /* can only be captured */ \
            -0.2f \
        }; \
        for(int ppy=-2;ppy<=2;ppy++) \
            for(int ppx=-2;ppx<=2;ppx++) \
                if((ppy*ppy)+(ppx*ppx)<=4) terrainGrid[(int)(y+0.5f)+ppy][(int)(x+0.5f)+ppx] = terrainGrid[(int)(y+0.5f)][(int)(x+0.5f)]; \
    }


#define CREATE_MINE(faction, x, y) \
    if (num_units < MAX_UNITS) { \
        units[num_units++] = { \
            &tex::mine,   /* texture */ \
            "Mine",       /* name */ \
            0.0,          /* speed */ \
            (float)(x),   /* x */ \
            (float)(y),   /* y */ \
            12.0,         /* attack_rate (6 industry)*/ \
            4.5,         /* range */ \
            0.0,          /* damage */ \
            0.0,          /* experience */ \
            0.0,          /* angle */ \
            2.0,          /* size */ \
            50.0,        /* health */ \
            50.0,        /* max_health */ \
            (faction),    /* faction */ \
            (faction),     /* can only be captured */ \
            0.5           /* extra scale*/\
        };\
        for(int ppy=-2;ppy<=2;ppy++) \
            for(int ppx=-2;ppx<=2;ppx++) \
                if((ppy*ppy)+(ppx*ppx)<=4) terrainGrid[(int)(y+0.5f)+ppy][(int)(x+0.5f)+ppx] = terrainGrid[(int)(y+0.5f)][(int)(x+0.5f)]; \
    }

#define CREATE_RADIO(faction, x, y) \
    if (num_units < MAX_UNITS) {\
        units[num_units++] = { \
            &tex::radio,   /* texture */ \
            "Radio",       /* name */ \
            0.0,          /* speed */ \
            (float)(x),   /* x */ \
            (float)(y),   /* y */ \
            0.0,          /* attack_rate */ \
            16.0,          /* range */ \
            0.0,          /* damage */ \
            0.0,          /* experience */ \
            0.0,          /* angle */ \
            2.0,          /* size */ \
            50.0,         /* health */ \
            50.0,         /* max_health */ \
            (faction),    /* faction */ \
            (faction),    /* can only be captured */ \
            0.5           /* extra scale*/\
        };\
        for(int ppy=-2;ppy<=2;ppy++) \
            for(int ppx=-2;ppx<=2;ppx++) \
                if((ppy*ppy)+(ppx*ppx)<=4) terrainGrid[(int)(y+0.5f)+ppy][(int)(x+0.5f)+ppx] = terrainGrid[(int)(y+0.5f)][(int)(x+0.5f)]; \
    }


#define CREATE_LAB(faction, x, y) \
    if (num_units < MAX_UNITS) { \
        units[num_units++] = { \
            &tex::lab,   /* texture */ \
            "Lab",       /* name */ \
            0.0,          /* speed */ \
            (float)(x),   /* x */ \
            (float)(y),   /* y */ \
            0.0,          /* attack_rate */ \
            4.5,          /* range */ \
            0.0,          /* damage */ \
            0.0,          /* experience */ \
            0.0,          /* angle */ \
            1.8,          /* size */ \
            50.0,         /* health */ \
            50.0,         /* max_health */ \
            (faction),    /* faction */ \
            (faction),    /* can only be captured */ \
            0.2           /* extra scale*/\
        };\
        for(int ppy=-2;ppy<=2;ppy++) \
            for(int ppx=-2;ppx<=2;ppx++) \
                if((ppy*ppy)+(ppx*ppx)<=4) terrainGrid[(int)(y+0.5f)+ppy][(int)(x+0.5f)+ppx] = terrainGrid[(int)(y+0.5f)][(int)(x+0.5f)]; \
    }

#define CREATE_OIL(faction, x, y) \
    if (num_units < MAX_UNITS) { \
        units[num_units++] = { \
            &tex::oil,   /* texture */ \
            "Oil",       /* name */ \
            0.0,          /* speed */ \
            (float)(x),   /* x */ \
            (float)(y),   /* y */ \
            0.0,          /* attack_rate */ \
            1.0,          /* range */ \
            0.0,          /* damage */ \
            0.0,          /* experience */ \
            0.0,          /* angle */ \
            1.2,          /* size */ \
            25.0,         /* health */ \
            25.0,         /* max_health */ \
            (faction),    /* faction */ \
            (faction)     /* can only be captured */ \
        };\
        for(int ppy=-2;ppy<=2;ppy++) \
            for(int ppx=-2;ppx<=2;ppx++) \
                if((ppy*ppy)+(ppx*ppx)<=4) terrainGrid[(int)(y+0.5f)+ppy][(int)(x+0.5f)+ppx] = terrainGrid[(int)(y+0.5f)][(int)(x+0.5f)]; \
    }

#define CREATE_WAREHOUSE(faction, x, y) \
    if (num_units < MAX_UNITS) {\
        units[num_units++] = { \
            &tex::warehouse,   /* texture */ \
            "Storage",  /* name */ \
            0.0,          /* speed */ \
            (float)(x),   /* x */ \
            (float)(y),   /* y */ \
            0.0,          /* attack_rate */ \
            4.5,          /* range */ \
            0.0,          /* damage */ \
            0.0,          /* experience */ \
            0.0,          /* angle */ \
            2.0,          /* size */ \
            50.0,         /* health */ \
            50.0,         /* max_health */ \
            (faction),    /* faction */ \
            (faction)     /* can only be captured */ \
        };\
        for(int ppy=-2;ppy<=2;ppy++) \
            for(int ppx=-2;ppx<=2;ppx++) \
                if((ppy*ppy)+(ppx*ppx)<=4) terrainGrid[(int)(y+0.5f)+ppy][(int)(x+0.5f)+ppx] = terrainGrid[(int)(y+0.5f)][(int)(x+0.5f)]; \
    }

inline bool IsGrass(Texture2D* t) {
    return t==&tex::grass || t==&tex::grass2 || t==&tex::grass3 || t==&tex::grass4;
}
inline bool IsHill(Texture2D* t) {
    return t==&tex::hill || t==&tex::hill2 || t==&tex::hill3 || t==&tex::hill4;
}
inline bool IsMountain(Texture2D* t) {
    return t==&tex::mountain;
}
inline bool IsDesert(Texture2D* t) {
    return t==&tex::desert;
}
inline void DrawRot(Texture2D tex, int px, int py, float rot){
    Rectangle src = {0.0f,0.0f,(float)tex.width,(float)tex.height};
    Rectangle dst = {(float)px + tex.width  * 0.5f,(float)py + tex.height * 0.5f,(float)tex.width,(float)tex.height};
    Vector2 origin = {tex.width  * 0.5f, tex.height * 0.5f};
    DrawTexturePro(tex, src, dst, origin, rot, WHITE);
}


static bool DrawTechNode(
    float x, float y,
    const char* title,
    const char* desc,
    unsigned long long &tech,
    unsigned long long bit
) {
    const int W = GetScreenWidth()/6-40;
    const int H = GetScreenHeight()/14-20;

    Rectangle rect = { x, y, (float)W, (float)H };
    Vector2 mouse = GetMousePosition();

    bool hovered = CheckCollisionPointRec(mouse, rect);
    bool owned   = (tech & bit) != 0;

    // --- colors ---
    Color bg =
        owned   ? Fade(GREEN, 0.35f) :
        hovered ? Fade(DARKGRAY, 0.75f) :
                  Fade(GRAY, 0.55f);

    Color edge =
        owned   ? GREEN :
        hovered ? WHITE :
                  GRAY;

    // --- draw node ---
    DrawRectangleRounded(rect, 0.2f, 6, bg);
    DrawRectangleRoundedLines(rect, 0.2f, 6, edge);

    DrawText(title, x + 12, y + 6, W/10, WHITE);
    DrawTextSmall(desc,  x + 12, y + W/8, (H-W/10)/1.7f, Fade(WHITE, 0.85f));

    // --- hover hint ---
    // if (hovered && !owned) {
    //     DrawText(
    //         "Click to get it once research completes",
    //         x + 8,
    //         y - 26,
    //         18,
    //         Fade(WHITE, 0.7f)
    //     );
    // }

    // --- click logic ---
    if (hovered && !owned && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        tech |= bit;
        return true;
    }

    return false;
}


static const char* veteran_name = "Veteran";
static const char* hero_name = "Hero";


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
    unsigned int n = (unsigned int)(x * 374761393u
                                   + y * 668265263u
                                   + NOISE_SEED * 374761393u);
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


// static Color ColorForTile(Texture2D* tex) {
//     if (tex == &tex::grass || tex == &tex::grass2 || tex == &tex::grass3 || tex == &tex::grass4) return (Color){ 60, 180, 60, 255 };
//     if (tex == &tex::desert) return (Color){ 220, 200, 120, 255 };
//     if (IsHill(tex)) return (Color){ 140, 120, 80, 255 };
//     if (IsMountain(tex)) return (Color){ 110, 110, 110, 255 };
//     if (tex == &tex::water) return (Color){ 110, 110, 255, 255 };
//     return WHITE;
// }

struct Decorator {
    Texture2D* texture;
    float x,y;
    float size;
};

const int GAME_W = 2560;
const int GAME_H = 1600;


void unload() {
    UnloadTexture(tex::gear);
    UnloadTexture(tex::sun);
    UnloadTexture(tex::reward);
    UnloadTexture(tex::research);
    UnloadTexture(tex::utopia);
    UnloadTexture(tex::track);
    UnloadTexture(tex::grass);
    UnloadTexture(tex::grass2);
    UnloadTexture(tex::grass3);
    UnloadTexture(tex::grass4);
    UnloadTexture(tex::grass_transition);
    UnloadTexture(tex::grass_transition2);
    UnloadTexture(tex::hill);
    UnloadTexture(tex::hill2);
    UnloadTexture(tex::hill3);
    UnloadTexture(tex::hill4);
    UnloadTexture(tex::hill_transition);
    UnloadTexture(tex::hill_transition2);
    UnloadTexture(tex::hill_outline);
    UnloadTexture(tex::hill_wedge);
    UnloadTexture(tex::desert);
    UnloadTexture(tex::desert_transition);
    UnloadTexture(tex::mountain);
    UnloadTexture(tex::mountain_transition);
    UnloadTexture(tex::mountain_transition2);
    UnloadTexture(tex::human);
    UnloadTexture(tex::esper);
    UnloadTexture(tex::scout);
    UnloadTexture(tex::tank);
    UnloadTexture(tex::van);
    UnloadTexture(tex::snowman);
    UnloadTexture(tex::field);
    UnloadTexture(tex::field_little);
    UnloadTexture(tex::field_empty);
    UnloadTexture(tex::heal);
    UnloadTexture(tex::camp);
    UnloadTexture(tex::lab);
    UnloadTexture(tex::blood);
    UnloadTexture(tex::bison);
    UnloadTexture(tex::rat);
    UnloadTexture(tex::roomba);
    UnloadTexture(tex::wolf);
    UnloadTexture(tex::warehouse);
    UnloadTexture(tex::ghost);
    UnloadTexture(tex::crater);
    UnloadTexture(tex::radio);
    UnloadTexture(tex::oil);
    UnloadTexture(tex::datacenter);
    UnloadTexture(tex::info);
    UnloadTexture(tex::overlay);
    UnloadTexture(tex::railgun);
    UnloadTexture(tex::road);
    UnloadTexture(tex::road_transition);
    UnloadTexture(tex::mine);
    UnloadTexture(tex::snow);
    UnloadTexture(tex::tree);
    UnloadTexture(tex::water);
    UnloadTexture(tex::earth);
    sound::boom.Unload();
    sound::explosion.Unload();
    sound::gun.Unload();
    sound::damage.Unload();
    sound::dead.Unload();
    sound::ohm.Unload();
    sound::moo.Unload();
    sound::noise.Unload();
    sound::notify.Unload();
    UnloadSound(sound::select);
    UnloadSound(sound::select2);
    CloseAudioDevice();
    CloseWindow();
}


int main() {
    PrefMask unlocked_preferences = 0;
    const double AI_ELO = 1800;
    double player_rating = 1200;
    {
        FILE* f = fopen("midnight-save.dat", "rb");
        if(f) {
            if(fread(&unlocked_preferences, sizeof(unlocked_preferences), 1, f)!=1)
                unlocked_preferences = 0;
            if(fread(&player_rating, sizeof(player_rating), 1, f)!=1)
                player_rating = 0;
            fclose(f);
        }
    }

    SetTraceLogLevel(LOG_NONE); // disable raylib logs
    SetConfigFlags(FLAG_FULLSCREEN_MODE);
    InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "MIDNIGHT - next morn");
    InitAudioDevice();
    SetRandomSeed((unsigned)time(NULL));
    MaximizeWindow();
    SetTextureFilter(uiFont.texture, TEXTURE_FILTER_BILINEAR);
    uiFont = LoadFontEx("data/Beholden-Regular.ttf", 96, nullptr, 0);
    SetTextureFilter(uiFont.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(smallerFont.texture, TEXTURE_FILTER_BILINEAR);
    smallerFont = LoadFontEx("data/Beholden-Regular.ttf", 32, nullptr, 0);
    SetTextureFilter(smallerFont.texture, TEXTURE_FILTER_BILINEAR);
    tex::grass = LoadTexture("data/grass.png");
    tex::grass2 = LoadTexture("data/grass2.png");
    tex::grass3 = LoadTexture("data/grass3.png");
    tex::grass4 = LoadTexture("data/grass4.png");
    tex::grass_transition = LoadTexture("data/grass_transition.png");
    tex::grass_transition2 = LoadTexture("data/grass_transition2.png");
    tex::hill  = LoadTexture("data/hill3.png");
    tex::hill2 = LoadTexture("data/hill2.png");
    tex::hill3 = LoadTexture("data/hill.png");
    tex::hill4 = LoadTexture("data/hill4.png");
    tex::hill_transition = LoadTexture("data/hill_transition.png");
    tex::hill_transition2 = LoadTexture("data/hill_transition2.png");
    tex::hill_outline = LoadTexture("data/hill_outline.png");
    tex::hill_wedge = LoadTexture("data/hill_wedge.png");
    tex::mountain = LoadTexture("data/mountain.png");
    tex::mountain_transition = LoadTexture("data/mountain_transition.png");
    tex::mountain_transition2 = LoadTexture("data/mountain_transition2.png");
    tex::snow = LoadTexture("data/snow.png");
    tex::tree = LoadTexture("data/tree.png");
    tex::desert = LoadTexture("data/desert.png");
    tex::water = LoadTexture("data/water.png");
    tex::desert_transition = LoadTexture("data/desert_transition.png");
    tex::human = LoadTexture("data/human.png");
    tex::esper = LoadTexture("data/esper.png");
    tex::scout = LoadTexture("data/scout.png");
    tex::tank = LoadTexture("data/tank.png");
    tex::van = LoadTexture("data/van.png");
    tex::gear = LoadTexture("data/gear.png");
    tex::utopia = LoadTexture("data/utopia.png");
    tex::track = LoadTexture("data/track.png");
    tex::sun = LoadTexture("data/sun.png");
    tex::reward = LoadTexture("data/reward.png");
    tex::earth = LoadTexture("data/earth.png");
    tex::research = LoadTexture("data/research.png");
    tex::snowman = LoadTexture("data/snowman.png");
    tex::railgun = LoadTexture("data/railgun.png");
    tex::hide = LoadTexture("data/hide.png");
    tex::rat = LoadTexture("data/rat.png");
    tex::roomba = LoadTexture("data/roomba.png");
    tex::bison = LoadTexture("data/bison.png");
    tex::wolf = LoadTexture("data/wolf.png");
    tex::heal = LoadTexture("data/heal.png");
    tex::field = LoadTexture("data/field.png");
    tex::field_empty = LoadTexture("data/field_empty.png");
    tex::field_little = LoadTexture("data/field_little.png");
    tex::mine = LoadTexture("data/mine.png");
    tex::road = LoadTexture("data/road.png");
    tex::road_transition = LoadTexture("data/road_transition.png");
    tex::camp = LoadTexture("data/camp.png");
    tex::lab = LoadTexture("data/lab.png");;
    tex::warehouse = LoadTexture("data/warehouse.png");
    tex::blood = LoadTexture("data/blood.png");
    tex::ghost = LoadTexture("data/ghost.png");
    tex::crater = LoadTexture("data/crater.png");
    tex::radio = LoadTexture("data/radio.png");
    tex::oil = LoadTexture("data/oil.png");
    tex::datacenter = LoadTexture("data/datacenter.png");
    tex::info = LoadTexture("data/info.png");
    tex::overlay = LoadTexture("data/overlay.png");
    tex::banner = LoadTexture("data/banner.png");
    tex::banner = LoadTexture("data/banner.png");


    sound::gun.Load("data/gun.ogg", 24);
    sound::boom.Load("data/boom.ogg", 5);
    sound::explosion.Load("data/explosion.ogg", 10);
    sound::damage.Load("data/damage.wav", 10);
    sound::dead.Load("data/dead.ogg", 10);
    sound::ohm.Load("data/ohm.ogg", 5);
    sound::moo.Load("data/moo.wav", 10);
    sound::noise.Load("data/noise.wav", 5);
    sound::notify.Load("data/notify.ogg", 5);
    sound::select = LoadSound("data/select.wav");
    sound::select2 = LoadSound("data/select2.wav");
    sound::bg = LoadMusicStream("data/track.ogg");
    sound::bg.looping = true;
    PlayMusicStream(sound::bg);

    SetTargetFPS(60);

    int num_units = 0;
    int max_factions = 7; // can never be less than 3 if we include the player, unclaimed, and wild - can also not include the last Wild faction
    static const float GAME_DURATION = 17.0f * 60.0f; // 17 minutes
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
    Rectangle clusteringBtn = {
        GetScreenWidth() - 260.0f,
        GetScreenHeight() - 95.0f,
        240.0f,
        90.0f
    };
    bool showTechTree = false;
    bool showHelp = false;
    Rectangle techBtn = {
        GetScreenWidth() - 260.0f,
        GetScreenHeight() - 190.0f,
        240.0f,
        90.0f
    };
    Rectangle helpBtn = {
        GetScreenWidth() - 260.0f,
        GetScreenHeight() - 190.0f-65.f,
        240.0f,
        60.0f
    };
    Rectangle optionsButton = {
        GetScreenWidth() - 260.0f,
        10.f,
        240.0f,
        60.0f
    };

    // preallocate stuff
    Terrain* terrainBlock = (Terrain*)malloc(GRID_SIZE * GRID_SIZE * sizeof(Terrain));
    Terrain** terrainGrid = (Terrain**)malloc(GRID_SIZE * sizeof(Terrain*));
    for (int y = 0; y < GRID_SIZE; y++)
        terrainGrid[y] = &terrainBlock[y * GRID_SIZE];
    static Unit units[MAX_UNITS];
    static Decorator decorators[MAX_DECORATORS];
    static Faction factions[11] = {
        { BLUE, "Player", 0 },
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
        for (int fi = 0; fi < max_factions; fi++) {
            factions[fi].visible_knowledge.set(unitIndex);
        }
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

        float baseY = (float)GetScreenHeight()/2 - 600;
        float baseX = (float)GetScreenWidth()/2;
        if(main_menu_transition_mode==-2) baseX += (1.0f-main_menu_progress)*(1.0f-main_menu_progress)*GetScreenWidth();
        else if(main_menu_transition_mode==-1) baseX -= (1.0f-main_menu_progress)*(1.0f-main_menu_progress)*GetScreenWidth();
        else if(main_menu_transition_mode==2) baseX -= main_menu_progress*main_menu_progress*GetScreenWidth();
        else baseX -= main_menu_progress*main_menu_progress*GetScreenWidth();
        const Rectangle btnStart = {baseX - 400, baseY+720,900, 80};
        const Rectangle btnQuit = {baseX - 400, baseY+820,900, 80};
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

        // Start button
        bool hoverStart = !main_menu_transition_mode && CheckCollisionPointRec(mouse, btnStart);
        DrawRectangleRec(btnStart, hoverStart ? DARKGRAY : BLACK);
        DrawRectangleLinesEx(btnStart, 2, DARKGRAY);
        DrawText("New expedition", btnStart.x + 30, btnStart.y + 8, 58, hoverStart ? WHITE : GRAY);
        // DrawTexturePro(tex::utopia,
        //                Rectangle{0,0,(float)tex::utopia.width,(float)tex::utopia.height},
        //                Rectangle{baseX+400, btnStart.y+5, 80, 80},
        //                {0,0}, 0, WHITE);

        // rank
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
            float radius = size*0.55f;
            float end_angle = (float)(rank_progress * 360.0) - 90;
            float cx = baseX - 400 + 32;
            float cy = baseY + 660;
            DrawRing({cx, cy}, radius - 8, radius, -90, end_angle, 64, ranks[rank_index].color);
            DrawRing({cx, cy}, radius - 8, radius, end_angle, 360-90, 64, Fade(ranks[rank_index].color, 0.4f));
            DrawTexturePro(
                *ranks[rank_index].tex,
                {0,0,(float)ranks[rank_index].tex->width,(float)ranks[rank_index].tex->height},
                           {cx - size/2, cy - size/2, size, size},
                           {0,0}, 0, ranks[rank_index].color
            );
            DrawText(ranks[rank_index].name, cx+40, cy-22, 48, ranks[rank_index].color);
        }

        // quit button
        bool hoverQuit = !main_menu_transition_mode && CheckCollisionPointRec(mouse, btnQuit);
        DrawRectangleRec(btnQuit, hoverQuit ? Fade(RED, 0.85) : BLACK);
        DrawRectangleLinesEx(btnQuit, 2, DARKGRAY);
        DrawText("Quit", btnQuit.x + 30, btnQuit.y + 8, 58, hoverQuit ? WHITE : DARKGRAY);
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
    NOISE_SEED = GetRandomValue(1, 1'000'000);
    GenerateGrass(terrainGrid);
    GenerateHillsAndDesert(terrainGrid);
    GenerateRivers(terrainGrid);

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
        const Rectangle btnStart = {baseX - 400, baseY+720,900, 80};
        const Rectangle btnQuit = {baseX - 400, baseY+820,900, 80};
        Rectangle prefBox[2] = {
            { baseX+220-260, baseY + 720-70-10-70-30, 310, 70 },
            { baseX+220-260, baseY + 720-70-30, 310, 70 }
        };
        BeginDrawing();
        ClearBackground(BLACK);

        DrawTexturePro(
            tex::earth,
            Rectangle{0,0,(float)tex::earth.width,(float)tex::earth.height},
            Rectangle{baseX-256, baseY+100, 512, 256},
            {0,0}, 0, Fade(WHITE, 0.85f));
        DrawText("Terrain peek", baseX - 320, baseY+400, 128, WHITE);

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
                    (Vector2){ leftArrow.x + leftArrow.width, leftArrow.y },
                             (Vector2){ leftArrow.x, leftArrow.y + leftArrow.height/2 },
                             (Vector2){ leftArrow.x + leftArrow.width, leftArrow.y + leftArrow.height },
                             hoverL ? ORANGE : DARKGRAY);
                DrawTriangle(
                    (Vector2){ rightArrow.x, rightArrow.y },
                             (Vector2){ rightArrow.x, rightArrow.y + rightArrow.height },
                             (Vector2){ rightArrow.x + rightArrow.width, rightArrow.y + rightArrow.height/2 },
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
        DrawRectangleRec(btnStart, hoverStart ? Fade(GREEN, 0.28) : BLACK);
        DrawRectangleLinesEx(btnStart, 2, DARKGRAY);
        DrawText("Ready", btnStart.x + 30, btnStart.y + 8, 58, hoverStart ? WHITE : GRAY);
        // quit button
        bool hoverQuit = !new_game_transition_mode && CheckCollisionPointRec(mouse, btnQuit);
        DrawRectangleRec(btnQuit, hoverQuit ? DARKGRAY : BLACK);
        DrawRectangleLinesEx(btnQuit, 2, DARKGRAY);
        DrawText("Back", btnQuit.x + 30, btnQuit.y + 8, 58, hoverQuit ? WHITE : DARKGRAY);
        EndDrawing();
        if (hoverStart && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            PlaySound(sound::select);
            new_game_transition_mode = 1;
        }
        if ((hoverQuit && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_ESCAPE)) {
            new_game_transition_mode = 2;
            PlaySound(sound::select);
        }
    }

    GAME_OVER:
    {
        int player_points = factions[0].victory_points;
        int best_ai_points = 0;
        for (int fi = 3; fi < max_factions; fi++)
            if (factions[fi].victory_points > best_ai_points)
                best_ai_points = factions[fi].victory_points;

        bool victory = (player_points > best_ai_points) && factions[0].count_members;
        if(player_points<0) player_points = -player_points;

        const char* badTechNames[8];
        int badTechCount = 0;
        auto player_techs = factions[0].technology;
        if (player_techs & TECHNOLOGY_BIOWEAPON) badTechNames[badTechCount++] = "BIOWEAPONS";
        if (player_techs & TECHNOLOGY_PROPAGANDA) badTechNames[badTechCount++] = "PROPAGANDA";
        if (player_techs & TECHNOLOGY_SUPERIORITY) badTechNames[badTechCount++] = "SUPERIORITY";
        if (player_techs & TECHNOLOGY_ARTIFICIAL) badTechNames[badTechCount++] = "HIVEMENIND";
        if (player_techs & TECHNOLOGY_AIFARM) badTechNames[badTechCount++] = "AI FARMS";
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
            float baseY = (float)GetScreenHeight()/2 - 600;
            float baseX = (float)GetScreenWidth()/2;
            if(game_over_transition_mode) baseX -= game_over_progress*game_over_progress*GetScreenWidth();
            Rectangle btnOk = {baseX - 400, baseY+820,800, 80};

            BeginDrawing();
            ClearBackground(BLACK);
            if(victory) {
                DrawTexturePro(
                    tex::sun,
                    Rectangle{0,0,(float)tex::sun.width,(float)tex::sun.height},
                    Rectangle{baseX-256, baseY+100, 512, 256},
                    {0,0}, 0, WHITE);
                DrawText("BEST UTOPIA", baseX - MeasureText("BEST UTOPIA", 96)/2+70, baseY+420, 96, GREEN);
            }
            else if (game_time >= GAME_DURATION) {
                DrawTexturePro(
                    tex::blood,
                    Rectangle{0,0,(float)tex::blood.width,(float)tex::blood.height},
                    Rectangle{baseX-128, baseY+100, 256, 256},
                    {0,0}, 0, WHITE);
                DrawTexturePro(
                    tex::blood,
                    Rectangle{0,0,(float)tex::blood.width,(float)tex::blood.height},
                    Rectangle{baseX-256+32, baseY+100, 256, 256},
                    {0,0}, 0, WHITE);
                DrawTexturePro(
                    tex::blood,
                    Rectangle{0,0,(float)tex::blood.width,(float)tex::blood.height},
                    Rectangle{baseX-32, baseY+100, 256, 256},
                    {0,0}, 0, WHITE);
                DrawText("SURVIVED", baseX - MeasureText("SURVIVED", 96)/2+50, baseY+420, 96, ORANGE);
            }
            else {
                DrawTexturePro(
                    tex::blood,
                    Rectangle{0,0,(float)tex::blood.width,(float)tex::blood.height},
                    Rectangle{baseX-128, baseY+100, 256, 256},
                    {0,0}, 0, WHITE);
                DrawTexturePro(
                    tex::blood,
                    Rectangle{0,0,(float)tex::blood.width,(float)tex::blood.height},
                    Rectangle{baseX-256+32, baseY+100, 256, 256},
                    {0,0}, 0, WHITE);
                DrawTexturePro(
                    tex::blood,
                    Rectangle{0,0,(float)tex::blood.width,(float)tex::blood.height},
                    Rectangle{baseX-32, baseY+100, 256, 256},
                    {0,0}, 0, WHITE);
                DrawText("ELIMINATED", baseX - MeasureText("ELIMINATED", 96)/2+50, baseY+420, 96, RED);
            }

            // --- Score ---
            char score[128];
            snprintf(score, sizeof(score), "Your utopia: %d   |   Best AI: %d", player_points, best_ai_points);
            if (game_time >= GAME_DURATION) DrawText(score, baseX - MeasureText(score, 42)/2+40, baseY+540, 42, WHITE);

            // --- Ethical tech disclosure ---
            if (badTechCount) {
                DrawTextSmall("Was it really worth it?", baseX - MeasureText("Was it really worth it?", 28)/2,baseY+610,28,ORANGE);
                for (int i = 0; i < badTechCount; i++)
                    DrawTextSmall(TextFormat("- %s", badTechNames[i]), baseX - 260,baseY+650 + i * 28,24, DARKGRAY);
            }

            // --- OK button ---
            Vector2 mouse = GetMousePosition();
            bool hoverOk = !game_over_transition_mode && CheckCollisionPointRec(mouse, btnOk);
            DrawRectangleRec(btnOk, hoverOk ? DARKGRAY : BLACK);
            DrawRectangleLinesEx(btnOk, 2, GRAY);
            DrawText("OK", btnOk.x + btnOk.width/2 - MeasureText("OK", 48)/2, btnOk.y + 14, 48,hoverOk ? WHITE : GRAY);
            EndDrawing();
            if ((hoverOk && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_ESCAPE)) {
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




    START_GAME:
    max_factions = 7;
    num_units = 0;
    for(int i=0;i<max_factions;i++) {
        factions[i].visible_knowledge.reset();
        factions[i].technology = 0;
        factions[i].technology_progress = 0.f;
    }

    // minimap
    // const int MINIMAP_SCALE = 2;
    // RenderTexture2D minimap = LoadRenderTexture(GRID_SIZE * MINIMAP_SCALE, GRID_SIZE * MINIMAP_SCALE);
    // BeginTextureMode(minimap);
    // ClearBackground(BLACK);
    // for (int y = 0; y < GRID_SIZE; y++)
    //     for (int x = 0; x < GRID_SIZE; x++)
    //         DrawRectangle(x * MINIMAP_SCALE, y * MINIMAP_SCALE, MINIMAP_SCALE, MINIMAP_SCALE, Color{ 20,20,20,255 });
    // EndTextureMode(); // minimap


    // create grid (we have already done this)
    // GenerateGrass();
    // GenerateHillsAndDesert(terrainGrid);
    // GenerateRivers(terrainGrid);



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
                if(GetRandomValue(0,99)<50) {CREATE_BISON(&factions[2], px, by+4);}
                else CREATE_WOLF(&factions[2], px, by+4);
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
        for (int i=0; i<8+(player_preferred_start[0]==PREFERENCE_HUMAN?5:0)+(player_preferred_start[1]==PREFERENCE_HUMAN?5:0); i++) {
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
                int pref = GetRandomValue(0,PREFERENCE_COUNT-1);
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
                    if(GetRandomValue(0,99)<50) {CREATE_BISON(&factions[2], px, by+4);}
                    else CREATE_WOLF(&factions[2], px, by+4);
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
    int count_warehouses = 0;
    for (int i = 0; i < NUM_NEUTRAL_STRUCTURES; i++) {
        float x, y;
        x = GetRandomValue(20, GRID_SIZE - 20);
        y = GetRandomValue(20, GRID_SIZE - 20);
        if (tooCloseToAnyCamp(x, y)) continue;
        Terrain &T = terrainGrid[(int)y][(int)x];
        bool isGrass  = (T.texture == &tex::grass || T.texture == &tex::grass2 || T.texture == &tex::grass3 || T.texture == &tex::grass4);
        bool isDesert = (T.texture == &tex::desert);
        if(T.texture == &tex::water) continue;
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
                if (!isDesert && GetRandomValue(0, 99) < 80) continue;
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
                    CREATE_WAREHOUSE(&factions[1], x, y);
                    CREATE_RAILGUN(&factions[1], x-GetRandomValue(0, 5)-2, y-GetRandomValue(0, 5)-2);
                    CREATE_RAILGUN(&factions[1], x-GetRandomValue(0, 5)-2, y+GetRandomValue(0, 5)+2);
                    CREATE_RAILGUN(&factions[1], x+GetRandomValue(0, 5)+2, y-GetRandomValue(0, 5)-2);
                    CREATE_RAILGUN(&factions[1], x+GetRandomValue(0, 5)+2, y+GetRandomValue(0, 5)-2);
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
                CREATE_RADIO(&factions[1], x, y);
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
        else if (CheckCollisionPointRec(GetMousePosition(), clusteringBtn)) {
            mouseCapturedByUI = true;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
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
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            showTechTree = !showTechTree;
            PlaySound(sound::select2);
        }
        if(IsKeyPressed(KEY_TAB)) {
            showHelp = !showHelp;
            PlaySound(sound::select2);
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
            if (target_zoom > 1.5f) target_zoom = 1.5f;
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

        for(int i=0;i<max_factions;i++) {
            factions[i].industry = (factions[i].technology&TECHNOLOGY_REACTOR)?60:20;
            factions[i].count_members = 0;
            factions[i].victory_points = (factions[i].technology&TECHNOLOGY_SUPERIORITY)?1.f:0.f;
            factions[i].technology_progress += dt*0.009f;
            if(factions[i].technology==0 && factions[i].technology_progress<2.f) factions[i].technology_progress += dt*0.009f;
            if(factions[i].technology & TECHNOLOGY_NERDS) factions[i].technology_progress += dt*0.003f;
            if(factions[i].technology & TECHNOLOGY_TAMING) factions[i].technology_progress -= dt*0.0045f;
            if(factions[i].technology & TECHNOLOGY_RESEARCH) factions[i].technology_progress += dt*0.003f;
        }
        for (int i = 0; i < num_units; i++) {
            Unit &u = units[i];
            if(u.texture==&tex::lab && u.faction) {
                if(u.faction->technology & TECHNOLOGY_AIFARM) {
                    u.faction->industry += 16.f;
                    game_time += dt*0.08f;
                    polution_speedup += 0.08f;
                }
                else u.faction->technology_progress += dt*0.0009f;
            }
            if(u.texture==&tex::datacenter &&  u.faction && u.faction!=factions+1 && u.faction!=ANIMAL_FACTION){
                if((float)GetRandomValue(0, 1000000) / 1000000.0f * 300.f < dt) {
                    bool applied = false;
                    for (int j = 0; j < num_units; j++)
                        if (units[j].faction==u.faction && units[j].texture==&tex::blood) {
                            float dx = units[j].x - u.x;
                            float dy = units[j].y - u.y;
                            float d2 = dx*dx + dy*dy;
                            if(d2<25.f*25.f) {
                                units[j] = { \
                                    &tex::ghost,  /* texture */
                                    "Bloo",       /* name */
                                    2.0,          /* speed */
                                    u.x,          /* x */
                                    u.y,          /* y */
                                    2.0,          /* attack_rate */
                                    7.0,          /* range */
                                    0.5,          /* damage */
                                    0.0,          /* experience */
                                    u.angle,      /* angle */
                                    0.4,          /* size */
                                    3.0,          /* health */
                                    3.0,          /* max_health */
                                    factions+2,   /* faction */
                                    nullptr   /* faction */
                                };
                                units[j].popup = "radiation";
                                applied = true;
                            }
                        }
                    if(u.faction==factions && applied) {
                        last_message = "Databank: released radiation that turned nearby blood to bloo.";
                        last_message_counter = 0.f;
                    }
                }
                else if((float)GetRandomValue(0, 1000000) / 1000000.0f * 300.f < dt) {
                    bool applied = false;
                    for (int j = 0; j < num_units; j++)
                        if (units[j].faction==u.faction && units[j].texture==&tex::human && units[j].health<units[j].max_health) {
                            units[j].health += 2;
                            units[j].max_health += 2;
                            units[j].popup = "healthcare";
                            applied = true;
                        }
                    if(u.faction==factions && applied) {
                        last_message = "Databank: found healthcare products and increased HP of your humans.";
                        last_message_counter = 0.f;
                    }
                }
                else if((float)GetRandomValue(0, 1000000) / 1000000.0f * 300.f < dt) {
                    bool applied = false;
                    for (int j = 0; j < num_units; j++)
                        if (units[j].faction==u.faction && is_mecha(units[j]) && units[j].health<units[j].max_health) {
                            units[j].health = units[j].max_health;
                            units[j].popup = "parts";
                            applied = true;
                        }
                    if(u.faction==factions && applied) {
                        last_message = "Databank: found spare parts and fixed your mechas.";
                        last_message_counter = 0.f;
                    }
                }
                else if((float)GetRandomValue(0, 1000000) / 1000000.0f * 5000.f < dt) {
                    unsigned long long candidate = 1ULL << GetRandomValue(0, 62);
                    if(!(u.faction->technology & candidate)) {
                        u.faction->technology = u.faction->technology | candidate;
                        u.popup = "new tech";
                        if(u.faction==factions) {
                            last_message = "Databank: recovered a tech";
                            last_message_counter = 0.f;
                        }
                    }
                }
            }
            if(u.texture==&tex::camp && u.faction && (u.faction->technology & TECHNOLOGY_HUNTING)) u.faction->industry += 3.f;
            if(u.texture==&tex::camp && u.faction && (u.faction->technology & TECHNOLOGY_HARDCORE)) u.faction->industry -= 7.f;
            if(u.texture==&tex::camp || u.speed) u.faction->count_members += 0.00001f;
            if(u.texture==&tex::oil && u.faction && (u.faction->technology & TECHNOLOGY_REFINERY)) u.faction->industry += 25.f;
            if(u.texture==&tex::field || u.texture==&tex::field_little || u.texture==&tex::field_empty || u.texture==&tex::mine || u.texture==&tex::hide) {
                if((u.texture==&tex::field || u.texture==&tex::field_little || u.texture==&tex::field_empty) && u.faction && (u.faction->technology & TECHNOLOGY_ATMOSPHERE)) {
                    game_time -= dt*0.02f;
                    polution_speedup -= 0.02f;
                }
                if(u.texture==&tex::field) u.faction->industry += 4.f;
                if(u.texture==&tex::field_little) u.faction->industry += 2.f;
                if(u.texture==&tex::hide) u.faction->industry += 4.f;
                if(u.texture==&tex::mine) u.faction->industry += 12.f;
                if(u.texture==&tex::hide && u.faction && (u.faction->technology & TECHNOLOGY_HUNTING)) u.faction->industry += 3.f;
                continue;
            }
            if(is_mecha(u) && u.faction) {
                if(u.faction->technology & TECHNOLOGY_MECHANISED) u.faction->industry += u.health/5.f;
                if(u.faction->technology & TECHNOLOGY_GIGAJOULE) continue;
            }
            if(!u.faction) continue;
            if(u.capturing) continue;
            if(!u.speed) continue;
            if(u.texture!=&tex::bison && u.texture!=&tex::wolf && u.texture!=&tex::rat && u.texture!=&tex::snowman && u.texture!=&tex::roomba) { // don't count animals or roombas for industry needs'
                u.faction->count_members += u.max_health/5.f;
                if(u.faction->technology & TECHNOLOGY_HYPERMAGNET) u.faction->count_members += u.max_health/5.f;
            }
        }
        if(factions[0].count_members==0) {
            goto GAME_OVER;
        }
        for(int i=0;i<max_factions;i++) {
            if(factions[i].industry<0) factions[i].industry = 0;
        }

        // process units
        const float TURN_RATE = 36.0f;
        const float AIM_THRESHOLD = 5.0f;
        for (int i = 0; i < num_units; i++) {
            Unit &u = units[i];
            if(u.popup) {
                u.animation += dt*0.3f;
                if(u.animation>1.0f) {
                    u.animation = 0.f;
                    u.popup = nullptr;
                }
            }
            if(u.x<2) u.x = 2;
            if(u.y<2) u.y = 2;
            if(u.x>=GRID_SIZE-2) u.x = GRID_SIZE-2;
            if(u.y>=GRID_SIZE-2) u.y = GRID_SIZE-2;
            if (u.texture == &tex::blood) {
                if ((float)GetRandomValue(0, 1000000) / 1000000.0f < 0.005f * dt) {
                    num_units--;
                    u = units[num_units];
                    i--;
                    continue;
                }
                else if ((float)GetRandomValue(0, 1000000) / 1000000.0f < 0.005f * dt) {
                    u = { \
                        &tex::ghost,  /* texture */
                        "Bloo",       /* name */
                        2.0,          /* speed */
                        u.x,          /* x */
                        u.y,          /* y */
                        2.0,          /* attack_rate */
                        7.0,          /* range */
                        0.5,          /* damage */
                        0.0,          /* experience */
                        u.angle,      /* angle */
                        0.4,          /* size */
                        3.0,          /* health */
                        3.0,          /* max_health */
                        factions+2,   /* faction */
                        nullptr   /* faction */
                    };
                }
                continue; // blood does nothing else
            }
            if(!u.faction)
                continue;
            if (u.capturing && u.faction && (float)GetRandomValue(0, 1000000) / 1000000.0f < dt
                    *((u.faction->technology&TECHNOLOGY_OWNERSHIP)?1.0f:0.5f)
                    *(u.faction->count_members<=u.faction->industry?1.0f:OVER_CAP_REGEN_RATE)
                ) {
                u.health += CAPTURE_RATE;
                if (u.health > u.max_health)
                    u.health = u.max_health;
            }
            if(u.health<=0 && u.max_health) {
                if(!u.speed || !u.faction) {
                    u.health = u.max_health;
                }
                if(terrainGrid[(int)u.y][(int)u.x].texture==&tex::water){
                    num_units--;
                    u = units[num_units];
                    i--;
                }
                else if(u.texture==&tex::bison){// || u.texture==&tex::wolf) {
                    terrainGrid[(int)u.y][(int)u.x].speed /= 2; // terrain becomes uneven
                    u = { \
                        &tex::hide,  /* texture */
                        "Hide",       /* name */
                        0.0,          /* speed */
                        u.x,          /* x */
                        u.y,          /* y */
                        4.0,          /* attack_rate */
                        2.0,          /* range */
                        0.0,          /* damage */
                        0.0,          /* experience */
                        u.angle,      /* angle */
                        0.8,          /* size */
                        15.0,          /* health */
                        15.0,          /* max_health */
                        factions+1,       /* faction */
                        factions+1       /* faction */
                    };
                }
                else if(is_mecha(u)) {
                    if(visible[(int)u.y][(int)u.x]) {
                        int ux = (int)u.x;
                        int uy = (int)u.y;
                        int range = (int)u.size + 1;
                        if (ux >= xMin-range && ux < xMax+range && uy >= yMin-range && uy < yMax+range)
                            sound::explosion.Play(camera.zoom*0.2f);
                    }
                    terrainGrid[(int)u.y][(int)u.x].speed /= 2; // terrain becomes uneven
                    if(terrainGrid[(int)u.y][(int)u.x].extra_sight>0.5f)
                        terrainGrid[(int)u.y][(int)u.x].extra_sight = 0.5f; // extra dodge
                    u = { \
                        &tex::crater,  /* texture */
                        "Crater",      /* name */
                        0.0,          /* speed */
                        u.x,          /* x */
                        u.y,          /* y */
                        0.0,          /* attack_rate */
                        0.0,          /* range */
                        0.0,          /* damage */
                        0.0,          /* experience */
                        u.angle,      /* angle */
                        1.5,          /* size */
                        0.0,          /* health */
                        0.0,          /* max_health */
                        nullptr       /* faction */
                    };
                }
                else if(u.texture==&tex::ghost) { // use bloos to clean up units
                    num_units--;
                    u = units[num_units];
                }
                else {
                    if(visible[(int)u.y][(int)u.x]) {
                        int ux = (int)u.x;
                        int uy = (int)u.y;
                        int range = (int)u.size + 1;
                        if (ux >= xMin-range && ux < xMax+range && uy >= yMin-range && uy < yMax+range) {
                            if(u.texture==&tex::human) sound::dead.Play(camera.zoom*1.0f);
                            else sound::damage.Play(camera.zoom*0.2f);
                        }
                    }
                    u = { \
                        &tex::blood,  /* texture */
                        "Blood",      /* name */
                        0.0,          /* speed */
                        u.x,          /* x */
                        u.y,          /* y */
                        0.0,          /* attack_rate */
                        0.0,          /* range */
                        0.0,          /* damage */
                        0.0,          /* experience */
                        u.angle,      /* angle */
                        0.5,          /* size */
                        0.0,          /* health */
                        0.0,          /* max_health */
                        nullptr       /* faction */
                    };
                }
                continue;
            }
            if(u.texture==&tex::oil)
                u.faction->victory_points += 3.f;
            if(u.texture==&tex::esper)
                u.faction->victory_points += 2.f;
            if(u.texture==&tex::warehouse)
                u.faction->victory_points += 2.f;
            if(u.texture==&tex::radio && u.faction && (u.faction->technology & TECHNOLOGY_PROPAGANDA) )
                u.faction->victory_points += 0.334f;
            if(u.texture==&tex::lab)
                continue;
            if(u.texture==&tex::field) {
                if(u.faction->technology & TECHNOLOGY_FARMING) {
                    if((float)GetRandomValue(0, 1000000) / 1000000.0f <dt*0.01) {u.texture = &tex::field_empty;u.popup = "barren";} // once every 100 sec
                }
                else if((float)GetRandomValue(0, 1000000) / 1000000.0f <dt*0.05) {u.texture = &tex::field_empty;u.popup = "barren";} // once every 20 sec
                continue;
            }
            if(u.texture==&tex::field_little) {
                if((float)GetRandomValue(0, 1000000) / 1000000.0f <dt*0.05) {u.texture = &tex::field;u.popup = "bloom";} // once every 20 sec
                continue;
            }
            if(u.texture==&tex::field_empty) {
                if(u.faction->technology & TECHNOLOGY_FARMING) {
                    if((float)GetRandomValue(0, 1000000) / 1000000.0f <dt*0.1) {u.texture = &tex::field_little;u.popup = "grows";} // once every 10 sec
                }
                else if((float)GetRandomValue(0, 1000000) / 1000000.0f <dt*0.05) {u.texture = &tex::field_little;u.popup = "grows";} // once every 20 sec
                continue;
            }
            if(u.texture==&tex::hide) {
                if((float)GetRandomValue(0, 1000000) / 1000000.0f <dt*0.005) { // once every 10 mins it may become rats
                    CREATE_RAT(ANIMAL_FACTION, u.x-0.2, u.y+0.2);
                    CREATE_RAT(ANIMAL_FACTION, u.x-0.2, u.y-0.2);
                    CREATE_RAT(ANIMAL_FACTION, u.x+0.2, u.y+0.2);
                    CREATE_RAT(ANIMAL_FACTION, u.x+0.2, u.y-0.2);
                    CREATE_RAT(ANIMAL_FACTION, u.x, u.y);
                    u = { \
                        &tex::blood,  /* texture */
                        "Blood",      /* name */
                        0.0,          /* speed */
                        u.x,          /* x */
                        u.y,          /* y */
                        0.0,          /* attack_rate */
                        0.0,          /* range */
                        0.0,          /* damage */
                        0.0,          /* experience */
                        u.angle,      /* angle */
                        0.5,          /* size */
                        0.0,          /* health */
                        0.0,          /* max_health */
                        nullptr       /* faction */
                    };
                }
                continue;
            }
            if(u.texture==&tex::mine)
                continue;
            if (u.texture == &tex::rat) {
                if((float)GetRandomValue(0, 1000000) / 1000000.0f * 30.f<dt*u.attack_rate*(1-time_norm)*(1-time_norm)*(1-time_norm)*(1-time_norm)) {
                    int canMake = (int)u.faction->industry-(int)u.faction->count_members;
                    float sx = u.x + (GetRandomValue(-5000, 5000) * 0.0002f);
                    float sy = u.y + (GetRandomValue(-5000, 5000) * 0.0002f);
                    if(canMake>0) {
                        CREATE_RAT(u.faction, sx, sy);
                    }
                }
            }
            if (u.texture == &tex::camp) {
                if(u.faction && (float)GetRandomValue(0, 1000000) / 1000000.0f * 30.f<CAMP_SPAWN_RATE*dt*u.attack_rate*(1.1f-u.faction->count_members/(float)(1+u.faction->industry)) && u.faction!=factions+1) {
                    int canMake = (int)u.faction->industry-(int)u.faction->count_members;
                    if (canMake > 0) {
                        const int can_make_limit = (u.faction->technology & TECHNOLOGY_HARDCORE)?4:2;
                        if (canMake > can_make_limit) canMake = can_make_limit;
                        for (int k = 0; k < canMake; k++) {
                            if (num_units >= MAX_UNITS) break;
                            float sx = u.x + (GetRandomValue(-5000, 5000) * 0.0002f);
                            float sy = u.y + (GetRandomValue(-5000, 5000) * 0.0002f);
                            if(u.faction&&(u.faction->technology&TECHNOLOGY_EVOLUTION)&&GetRandomValue(0, 100)<10) {
                                CREATE_SNOWMAN(u.faction, sx, sy);
                            }
                            else {
                                CREATE_HUMAN(u.faction, sx, sy);
                            }
                            // superiority has 25% chance of spawning something hostile
                            /*if(u.faction&&(u.faction->technology&TECHNOLOGY_SUPERIORITY)&&GetRandomValue(0, 100)<25)
                                units[num_units-1].faction = factions+2;
                            else*/
                            {
                                if(u.faction && (u.faction->technology & TECHNOLOGY_NERDS)) {
                                    units[num_units-1].max_health -= 1;
                                    units[num_units-1].health -= 1;
                                }
                                if(u.faction && (u.faction->technology & TECHNOLOGY_UNSTABLE)) {
                                    units[num_units-1].max_health -= 1;
                                    units[num_units-1].health -= 1;
                                }
                                if(u.faction && (u.faction->technology & TECHNOLOGY_TOUGH)) {
                                    units[num_units-1].max_health += 1;
                                    units[num_units-1].health += 1;
                                }
                            }
                        }
                    }
                }
                continue;
            }
            if(u.faction && (u.faction->technology & TECHNOLOGY_MOBILE_FORTRESS) && u.texture==&tex::railgun && (float)GetRandomValue(0, 1000000) / 1000000.0f<dt*0.015) {
                u = { \
                    &tex::tank,   /* texture */ \
                    "Tank",       /* name */ \
                    3.0,          /* speed */ \
                    (float)(u.x),   /* x */ \
                    (float)(u.y),   /* y */ \
                    0.5,          /* attack_rate */ \
                    6.0,         /* range */ \
                    8.0,          /* damage */ \
                    0.0,          /* experience */ \
                    0.0,          /* angle */ \
                    0.8,          /* size */ \
                    20.0,         /* health */ \
                    20.0,         /* max_health */ \
                    (u.faction),    /* faction */ \
                    nullptr,    /* faction */ \
                    0.3           /* extra scale*/\
                };
                u.popup = "mobile fort";
                continue;
            }
            if(u.texture==&tex::esper && (float)GetRandomValue(0, 1000000) / 1000000.0f<0.03*dt) { // once every 30 seconds the esper changes their mind
                u.target_x = u.x+GetRandomValue(-40,40);
                u.target_y = u.y+GetRandomValue(-40,40);
                if(u.target_x<1) u.target_x = 1;
                if(u.target_y<1) u.target_y = 1;
                if(u.target_x>=GRID_SIZE-2) u.target_x = GRID_SIZE-2;
                if(u.target_y>=GRID_SIZE-2) u.target_y = GRID_SIZE-2;
            }

            float u_speed = terrainGrid[(int)u.y][(int)u.x].speed;
            if(u_speed<1.f && u.texture==&tex::snowman && (terrainGrid[(int)u.y][(int)u.x].texture==&tex::mountain || terrainGrid[(int)u.y][(int)u.x].texture==&tex::hill)) u_speed = 2.f;
            if(u_speed<1.f && is_mecha(u) && u.faction && (u.faction->technology && TECHNOLOGY_DRIVER)) u_speed = 1.f;
            if(u_speed<1.f && u.faction && (u.faction->technology && TECHNOLOGY_AGILE)) u_speed = (1.f+u_speed)*0.5f;
            if(u_speed<1.f && u.faction && (u.faction->technology & TECHNOLOGY_SEAFARERING) && terrainGrid[(int)u.y][(int)u.x].texture==&tex::water) u_speed = 1.5f;
            u_speed *= u.speed;
            float extra_sight = terrainGrid[(int)u.y][(int)u.x].extra_sight;
            if((u.texture==&tex::railgun || u.texture==&tex::radio) && extra_sight<0.f) extra_sight = 0.f;
            if(u.faction->technology&TECHNOLOGY_TRACK) extra_sight += 0.7f;
            float u_range = u.range*(1+extra_sight);
            if((u.texture==&tex::camp || u.texture==&tex::warehouse) && (u.faction->technology & TECHNOLOGY_EXPLORE)) u_range = 25.f;
            if(u.faction && (u.faction->technology & TECHNOLOGY_INFRASTRUCTURE) && u.texture==&tex::radio) u_range *= 1.5f;
            // attack (interrupt movement to attack)
            if (u.attack_target_x == 0 && u.attack_target_y == 0 && u.capturing!=factions+1) {
                float r = (float)GetRandomValue(0, 1000000) / 1000000.0f;
                float mul = 1.f;
                //if(u.target_x==0 && u.target_y==0) mul *= 40.f;
                float u_attack_rate = u.attack_rate;
                if(u.faction && (u.faction->technology&TECHNOLOGY_HELLBRINGER) && (u.name==veteran_name || u.name==hero_name)) u_attack_rate *= 3.f;
                if (r < dt * mul * u_attack_rate) {
                    u.attack_x = 0;
                    u.attack_y = 0;
                    float bestDist = 999999.0f;
                    float bestCaptureDist = bestDist;
                    Unit* best = nullptr;
                    Unit* bestCapture = nullptr;
                    bool is_roomba = u.texture==&tex::roomba;
                    // find closest enemy in range
                    for (int j = 0; j < num_units; j++) {
                        if(i == j) continue;
                        Unit &o = units[j];
                        //if (o.capturing) continue;
                        //if (o.faction == u.faction && (!o.capturing || o.health>=o.max_health)) continue;
                        if(o.health <=0) continue;
                        float dx = o.x - u.x;
                        float dy = o.y - u.y;
                        float d2 = dx*dx + dy*dy;
                        float effective_range = (u_range+1+o.size);
                        effective_range *= effective_range;
                        bool in_range = d2 < effective_range;
                        if(in_range && o.capturing) {
                            if(u.faction && !u.faction->visible_knowledge[j] && (u.faction->technology & TECHNOLOGY_WONDER)) {
                                u.faction->technology_progress += 0.02f;
                                // if(u.faction==factions) {
                                //     last_message_counter = 0.f;
                                //     last_message = "Wonder: research from new discovery";
                                // }
                            }
                            u.faction->visible_knowledge.set(j);
                        }
                        if(is_roomba && o.texture!=&tex::ghost && o.texture!=&tex::wolf && o.texture!=&tex::rat && o.texture!=&tex::bison) continue;
                        if(o.faction == u.faction) continue;
                        if(in_range) {
                            if(o.capturing) {
                                if(d2 < bestCaptureDist) {
                                    bestCaptureDist = d2;
                                    bestCapture = &o;
                                }
                            }
                            else {
                                if(d2 < bestDist) {
                                    bestDist = d2;
                                    best = &o;
                                }
                            }
                        }
                    }
                    // we re going to try to capture only if there's no enemy
                    if(!best) best = bestCapture;
                    if(best) {
                        u.attack_target_x = best->x;
                        u.attack_target_y = best->y;
                    }
                }
            }
            // if we are going to attack but are still rotating
            else if(u.attack_x==0 && u.attack_y==0 && u.attack_target_x && u.attack_target_y) {
                float dx = u.attack_target_x - u.x;
                float dy = u.attack_target_y - u.y;
                float targetAngle = atan2f(dy, dx) * RAD2DEG;

                // normalize angles into [-180, +180]
                float diff = targetAngle - u.angle;
                while (diff > 180.0f) diff -= 360.0f;
                while (diff < -180.0f) diff += 360.0f;

                // if not facing target, rotate toward it
                if (fabs(diff) > AIM_THRESHOLD) {
                    float rot = TURN_RATE * dt * (u_speed?u_speed:1.f);
                    if(u.texture==&tex::human) rot *= 3.f; // humans turn very fast
                    if(is_mecha(u) && (u.faction->technology&TECHNOLOGY_DRIVER)) rot *= 2.f;
                    if(u.faction && (u.faction->technology&TECHNOLOGY_SPEEDY)) rot *= 3.f; // even faster turning for speedy
                    if (diff > 0) {
                        u.angle += rot;
                        if(diff-rot<0) u.angle = targetAngle;
                    }
                    else {
                        u.angle -= rot;
                        if(diff+rot>0) u.angle = targetAngle;
                    }
                    continue;
                }
                float rad = u.size*TILE_SIZE;//(u.size+u.extra_scale) * TILE_SIZE;   // same radius as the drawn circle, NOT the image that may be larger

                float ang = u.angle * DEG2RAD;

                u.attack_x = u.x + cosf(ang) * (rad / TILE_SIZE);
                u.attack_y = u.y + sinf(ang) * (rad / TILE_SIZE);
                u.stunned += 0.1/u.attack_rate;
                if(visible[(int)u.y][(int)u.x]) {
                    int ux = (int)u.x;
                    int uy = (int)u.y;
                    int range = (int)u.size + 1;
                    if (ux >= xMin-range && ux < xMax+range && uy >= yMin-range && uy < yMax+range) {
                        if(u.texture==&tex::bison) sound::moo.Play(camera.zoom*0.5f);
                        else if(u.texture==&tex::wolf || u.texture==&tex::snowman) sound::damage.Play(camera.zoom*0.4f);
                        else if(u.texture==&tex::tank) sound::boom.Play(camera.zoom*0.9f);
                        else if(u.texture==&tex::esper) sound::ohm.Play(camera.zoom*0.9f);
                        else sound::gun.Play(camera.zoom*0.2f);
                    }
                }
                continue;
            }
            // TODO: perhaps heal only if not moving, but for now this is hard to properly check
            if (u.health < u.max_health && (!is_mecha(u) || (u.faction && (u.faction->technology & TECHNOLOGY_AUTOREPAIRS)))) {
                if ((float)GetRandomValue(0, 1000000) / 1000000.0f < dt*0.5f && (is_mecha(u) || u.faction==nullptr || !(u.faction->technology & TECHNOLOGY_NUCLEAR))) {
                    u.health += 1.0f;
                    if (u.health > u.max_health)
                        u.health = u.max_health;
                }
            }
            if((u.attack_x || u.attack_y) && (u.x-u.target_x)*(u.x-u.target_x)+(u.y-u.target_y)*(u.y-u.target_y) < u_range*u_range/3)
                continue;

            if(u.target_x==0 && u.target_y==0)
                continue;
            // --- ROTATE TOWARD MOVEMENT DIRECTION BEFORE MOVING ---
            float dx = u.target_x - u.x;
            float dy = u.target_y - u.y;
            float dist2 = dx*dx + dy*dy;

            // reached destination?
            if (dist2 < 0.1f) {
                u.target_x = 0;
                u.target_y = 0;
                continue;
            }

            float dist = sqrtf(dist2);
            float desiredAngle = atan2f(dy, dx) * RAD2DEG;

            // compute smallest signed difference
            float diff = desiredAngle - u.angle;
            while (diff > 180.0f) diff -= 360.0f;
            while (diff < -180.0f) diff += 360.0f;

            // rotate until close enough
            if (fabs(diff) > AIM_THRESHOLD) {
                float rot = TURN_RATE * dt * u_speed * 2;
                if(u.texture==&tex::human) rot *= 3.f; // humans turn very fast
                if(is_mecha(u) && (u.faction->technology&TECHNOLOGY_DRIVER)) rot *= 2.f;
                if(u.faction && (u.faction->technology&TECHNOLOGY_SPEEDY)) rot *= 3.f; // even faster turning for speedy
                if (diff>0) {
                    u.angle += rot;
                    if(diff-rot<0) u.angle = desiredAngle;
                }
                else {
                    u.angle -= rot;
                    if(diff+rot>0) u.angle = desiredAngle;
                }
                continue;
            }
            if(u.stunned>0) {
                u.stunned -= dt;
                if(u.stunned<0)
                    u.stunned = 0;
                if(u.faction!=factions) continue; // non-player factions stay and fight
            }
            float step = u_speed * dt * movement_speed_multiplier;
            if(u.faction && (u.faction->technology & TECHNOLOGY_HYPERMAGNET)) step *= 2;
            u.x += (dx / dist) * step;
            u.y += (dy / dist) * step;
        }

        //
        for(int i=0;i<max_factions;i++) {
            if(factions[i].technology & TECHNOLOGY_TECHNOCRACY) {
                factions[i].victory_points += factions[i].industry*0.01f;
                //factions[i].industry *= 0.5f;
            }
            game_time += dt*0.08f*factions[i].industry/200.f;
            polution_speedup += 0.08f*factions[i].industry/200.f;
            game_time += dt*0.08f*factions[i].count_members/200.f;
            polution_speedup += 0.08f*factions[i].count_members/200.f;
        }

        // repulse units
        const float stiffness = 1.0f;   // tune this
        const float radiusFactor = 0.7f;
        for (int i = 0; i < num_units; i++) {
            Unit &u = units[i];
            if (!u.max_health) continue;
            if (u.texture==&tex::field) continue;
            if (u.texture==&tex::field_empty) continue;
            if (u.texture==&tex::field_little) continue;
            if (u.texture==&tex::hide) continue;

            for (int j = i + 1; j < num_units; j++) {
                Unit &o = units[j];
                if (!o.max_health) continue;
                if (o.texture==&tex::field) continue;
                if (o.texture==&tex::field_little) continue;
                if (o.texture==&tex::field_empty) continue;
                if (o.texture==&tex::hide) continue;

                float dx = u.x - o.x;
                float dy = u.y - o.y;
                float d2 = dx*dx + dy*dy;

                float minDist = (u.size + o.size) * radiusFactor;
                float minDist2 = minDist * minDist;

                if (d2 < minDist2 && d2 > 0.0001f) {
                    float d = sqrtf(d2);
                    float overlap = minDist - d;
                    float force = overlap * stiffness;
                    float nx = dx / d;
                    float ny = dy / d;

                    bool u_movable = (u.speed > 0.0f);
                    bool o_movable = (o.speed > 0.0f);

                    if (u_movable && o_movable) {
                        // Both move equally
                        u.x += nx * force * 0.5f;
                        u.y += ny * force * 0.5f;
                        o.x -= nx * force * 0.5f;
                        o.y -= ny * force * 0.5f;
                    }
                    else if (u_movable && !o_movable) {
                        // Only u moves
                        u.x += nx * force;
                        u.y += ny * force;
                    }
                    else if (!u_movable && o_movable) {
                        // Only o moves
                        o.x -= nx * force;
                        o.y -= ny * force;
                    }
                    // else: both immovable → do nothing
                }
            }
        }



        // --- PROCESS ATTACK PROJECTILES ---
        for (int i = 0; i < num_units; i++) {
            Unit &u = units[i];
            if (u.attack_target_x == 0 && u.attack_target_y == 0) continue;
            if (u.attack_x == 0 && u.attack_y == 0) continue;
            float ax = u.attack_x;
            float ay = u.attack_y;
            float tx = u.attack_target_x;
            float ty = u.attack_target_y;
            float dx = tx - ax;
            float dy = ty - ay;
            float dist = dx*dx + dy*dy;
            if (dist < 0.3f) {
                for (int j = 0; j < num_units; j++) {
                    Unit &o = units[j];
                    if (o.faction == u.faction && !o.capturing) continue;
                    float ox = o.x;
                    float oy = o.y;
                    float pdx = ox - tx;
                    float pdy = oy - ty;
                    if (pdx*pdx + pdy*pdy < 0.3f) { // hit radius approx
                        int oxi = (int)o.x;
                        int oyi = (int)o.y;
                        float skipChance = 0.f;//0.25f;
                        float u_damage = u.damage;
                        if(u.faction && (u.faction->technology & TECHNOLOGY_NUCLEAR)) u_damage *= 2.f;
                        if (oxi >= 0 && oyi >= 0 && oxi < GRID_SIZE && oyi < GRID_SIZE) skipChance -= terrainGrid[oyi][oxi].extra_sight/2.f;
                        if(o.faction && (o.faction->technology&TECHNOLOGY_MECHA) && is_mecha(o)) skipChance += 0.5f;
                        if(o.faction && (o.faction->technology&TECHNOLOGY_HEROICS) && o.name==hero_name) skipChance += 0.3f;
                        if(o.faction && (o.faction->technology&TECHNOLOGY_HEROICS) && o.name==veteran_name) skipChance += 0.3f;
                        if(o.faction && (o.faction->technology&TECHNOLOGY_LUXURY) && (u.texture==&tex::ghost || u.texture==&tex::bison || u.texture==&tex::wolf || u.texture==&tex::rat || u.texture==&tex::snowman)) skipChance += 0.5f;
                        if(u_damage>=o.health && o.faction && (o.faction->technology & TECHNOLOGY_GRIT)) skipChance += 0.5f;
                        if(u.faction && (u.faction->technology&TECHNOLOGY_SNIPING)) skipChance -= 0.5f;
                        if(skipChance<0.f) skipChance = 0.f;
                        if(skipChance>0.95f) skipChance = 0.95f;

                        if ((float)GetRandomValue(0, 1000000) / 1000000.0f >= skipChance) {
                            if(o.capturing && o.faction == u.faction) o.health += 1;
                            else if(o.capturing && o.capturing==factions+1) o.health -= CAPTURE_RATE*u_damage;
                            else if(o.capturing) o.health -= CAPTURE_RATE*u_damage*0.5f;
                            else {
                                if(is_mecha(o)) u_damage *= 0.33f;
                                o.health -= u_damage;
                            }
                        }
                        else if(u_damage>=o.health && o.faction && (o.faction->technology & TECHNOLOGY_GRIT)) {o.popup = "grit";}
                        else if(o.name==hero_name && (o.faction->technology&TECHNOLOGY_HEROICS)) {o.popup = "heroics";}
                        else if(o.name==veteran_name && (o.faction->technology&TECHNOLOGY_HEROICS)) {o.popup = "heroics";}
                        else if(o.faction && (o.faction->technology&TECHNOLOGY_MECHA) && is_mecha(o)) {o.popup = "mecha";}
                        else if(o.faction && (o.faction->technology&TECHNOLOGY_LUXURY) && (u.texture==&tex::ghost || u.texture==&tex::bison || u.texture==&tex::wolf || u.texture==&tex::rat || u.texture==&tex::snowman)) {o.popup = "pristine";}
                        else {o.popup = "cover";}
                        if (o.health >= o.max_health) o.health = o.max_health;
                        if (o.health < 0) o.health = 0;
                        if (o.health <=0 && u.max_health) {
                            float experience_bonus = o.experience/2 + (float)o.max_health/(float)u.max_health;
                            if(u.faction && (u.faction->technology&TECHNOLOGY_FIGHT)) experience_bonus *= 2.f;
                            if(u.texture==&tex::ghost && u.faction && (u.faction->technology&TECHNOLOGY_ARTIFICIAL)) experience_bonus *= 5.f;
                            u.experience += experience_bonus;
                            if(u.experience>=10 && (!is_mecha(u) || (u.faction && (u.faction->technology & TECHNOLOGY_INDUSTRY)))
                                && u.name!=veteran_name && u.name!=hero_name) {
                                u.size *= 1.2;
                                u.name = veteran_name;
                                u.damage *= 1.5;
                                u.max_health += 5;
                                u.health += 5;
                                u.popup = "new veteran";
                            }
                            if(u.experience>=20 && u.name==veteran_name) {
                                u.size *= 1.2;
                                u.name = hero_name;
                                u.damage *= 1.5;
                                u.max_health += 5;
                                u.health += 5;
                                u.popup = "new hero";
                            }
                            if(u.experience>50 && u.name==hero_name) {
                                u.experience -= 20;
                                int r = GetRandomValue(0, 100);
                                if(r<25) {
                                    u.attack_rate *= 1.5f;
                                    u.popup = "hero: aggression";
                                }
                                else if(r<50) {
                                    u.speed *= 1.2f;
                                    u.popup = "hero: faster";
                                }
                                else if(r<75){
                                    u.max_health += 2.f;
                                    u.health += 2.f;
                                    u.popup = "hero: healthier";
                                }
                                else {
                                    u.range *= 1.2f;
                                    u.popup = "hero: farsight";
                                }
                            }
                        }
                        if(o.health<=0 && u.faction && (u.faction->technology & TECHNOLOGY_HOMUNCULI) && o.texture==&tex::ghost) {
                            o.faction = u.faction;
                            o.health = o.max_health;
                            o.popup = "homunculi";
                        }
                        else if(o.health<=0 && u.faction && (u.faction->technology & TECHNOLOGY_HIJACK) && is_mecha(o) && GetRandomValue(0, 99) < 50) {
                            o.faction = u.faction;
                            o.health = o.max_health;
                            o.popup = "hijacked";
                            o.animation = 0.f;
                        }
                        else if(o.health<=0 && o.texture==&tex::wolf && GetRandomValue(0, 99) < 50) {
                            o.faction = u.faction;
                            o.health = o.max_health;
                            o.popup = o.faction==ANIMAL_FACTION?"wild":"tamed";
                        }
                        else if(o.health<=0 && u.faction && (u.faction->technology & TECHNOLOGY_TAMING) && (o.texture==&tex::bison || o.texture==&tex::rat || o.texture==&tex::snowman) && GetRandomValue(0, 99) < 50) {
                            o.faction = u.faction;
                            o.health = o.max_health;
                            o.popup = "tamed";
                        }
                        else if(o.health<=0 && o.faction && (o.faction->technology & TECHNOLOGY_UNSTABLE) && o.texture==&tex::human) {
                            units[j] = { \
                                &tex::ghost,  /* texture */
                                "Bloo",       /* name */
                                2.0,          /* speed */
                                o.x,          /* x */
                                o.y,          /* y */
                                2.0,          /* attack_rate */
                                7.0,          /* range */
                                0.5,          /* damage */
                                0.0,          /* experience */
                                o.angle,      /* angle */
                                0.4,          /* size */
                                3.0,          /* health */
                                3.0,          /* max_health */
                                o.faction,   /* faction */
                                nullptr   /* faction */
                            };
                            units[j].popup = "unstable";
                        }
                        else if(o.health<=0 && u.faction && (u.faction->technology & TECHNOLOGY_BIOWEAPON) && o.texture==&tex::human) {
                            units[j] = { \
                                &tex::ghost,  /* texture */
                                "Bloo",       /* name */
                                2.0,          /* speed */
                                o.x,          /* x */
                                o.y,          /* y */
                                2.0,          /* attack_rate */
                                7.0,          /* range */
                                0.5,          /* damage */
                                0.0,          /* experience */
                                o.angle,      /* angle */
                                0.4,          /* size */
                                3.0,          /* health */
                                3.0,          /* max_health */
                                o.faction,   /* faction */
                                nullptr   /* faction */
                            };
                            units[j].popup = "bioweapon";
                        }
                        if (o.health>0 && o.health<o.max_health && o.capturing) {
                            if(o.capturing==factions+1) o.popup = "capturing";
                            else o.popup = "contested";
                        }
                        if (o.health<=0 && o.capturing) {
                            o.animation = 0.f;
                            if(o.faction==factions) {
                                if(o.texture==&tex::camp) {
                                    last_message = "Important loss: Camp";
                                    last_message_counter = 0.f;
                                }
                                else if(o.texture==&tex::oil) {
                                    last_message = "Important loss: Oil";
                                    last_message_counter = 0.f;
                                }
                                else if(o.texture==&tex::warehouse) {
                                    last_message = "Important loss: Storage";
                                    last_message_counter = 0.f;
                                }
                                else if(o.texture==&tex::warehouse) {
                                    last_message = "Important loss: Esper";
                                    last_message_counter = 0.f;
                                }
                            }
                            if(u.faction==factions) {
                                if(o.texture==&tex::camp) {
                                    last_message = "Nice capture: Camp";
                                    last_message_counter = 0.f;
                                }
                                else if(o.texture==&tex::oil) {
                                    last_message = "Nice capture: Oil";
                                    last_message_counter = 0.f;
                                }
                                else if(o.texture==&tex::warehouse) {
                                    last_message = "Nice capture: Storage";
                                    last_message_counter = 0.f;
                                }
                                else if(o.texture==&tex::esper) {
                                    last_message = "Nice capture: Esper";
                                    last_message_counter = 0.f;
                                }
                            }
                            o.popup = "captured";
                            o.capturing = u.faction;
                            if(o.capturing==ANIMAL_FACTION) {
                                o.capturing = factions+1; // animals cannot capture
                                o.faction = o.capturing;
                                o.health = o.max_health;
                            }
                            else {
                                o.faction = o.capturing;
                                o.health = o.max_health;
                                if(o.texture==&tex::tank) o.capturing = nullptr; // only capture tanks once
                                if(o.texture==&tex::van) o.capturing = nullptr; // only capture vans once
                                if(o.texture==&tex::railgun) o.capturing = nullptr; // only capture railguns once
                                if(o.texture==&tex::roomba) o.capturing = nullptr; // only capture roombas once
                            }


                            if(u.faction && (u.faction->technology & TECHNOLOGY_TERRAFORIMING)) {
                                units[j] = { \
                                    &tex::field,   /* texture */ \
                                    "Field",       /* name */ \
                                    0.0,          /* speed */ \
                                    (float)(o.x),   /* x */ \
                                    (float)(o.y),   /* y */ \
                                    6.0,          /* attack_rate (6 industry)*/ \
                                    4.0,         /* range */ \
                                    0.0,          /* damage */ \
                                    0.0,          /* experience */ \
                                    0.0,          /* angle */ \
                                    1.3,          /* size */ \
                                    50.0,         /* health */ \
                                    50.0,         /* max_health */ \
                                    (u.faction),    /* faction */ \
                                    (u.faction),    /* can only be captured */ \
                                    -0.1f \
                                };
                                units[j].popup = "terraform";
                            }

                        }
                    }
                    else {
                        //u.popup = "missed";
                    }
                }
                u.attack_target_x = 0;
                u.attack_target_y = 0;
                continue;
            }
            dist = sqrtf(dist);
            float attackSpeed = 3.0f * dt * (u.attack_rate<1.0f?1.0f:u.attack_rate);
            if(u.faction && (u.faction->technology&TECHNOLOGY_HELLBRINGER) && (u.name==veteran_name || u.name==hero_name)) attackSpeed *= 2.f;
            u.attack_x += (dx / dist) * attackSpeed;
            u.attack_y += (dy / dist) * attackSpeed;
        }

        // spawn units
        if (GetRandomValue(0, 1000000) < (int)(ANIMAL_SPAWN_RATE * dt * 1000000.0f)) {
            for (int k = 0; k < 4; k++) { // small burst
                float x = GetRandomValue(10, GRID_SIZE - 10);
                float y = GetRandomValue(10, GRID_SIZE - 10);
                if (tooCloseToAnyCamp(x, y)) continue;
                Terrain &T = terrainGrid[(int)y][(int)x];
                if(GetRandomValue(0,1000)<10 && time_norm>0.3) {
                    CREATE_ESPER(ANIMAL_FACTION, x, y); // 2 every 250 seconds, but only after 33% of progress - they balance the late game by giving opportunities to players to come back
                    RevealUnitToAllFactions(num_units - 1);
                    last_message_counter = 0.f;
                    last_message = "An esper broadcasts their brainwaves!";
                }
                else if(T.texture == &tex::grass) {
                    if(GetRandomValue(0,100)<50) {
                        CREATE_BISON(ANIMAL_FACTION, x, y);
                    }
                    else {
                        CREATE_WOLF(ANIMAL_FACTION, x, y);
                    }
                }
                else if(T.texture == &tex::hill) {
                    CREATE_RAT(ANIMAL_FACTION, x-0.2, y+0.2);
                    CREATE_RAT(ANIMAL_FACTION, x-0.2, y-0.2);
                    CREATE_RAT(ANIMAL_FACTION, x+0.2, y+0.2);
                    CREATE_RAT(ANIMAL_FACTION, x+0.2, y-0.2);
                    CREATE_RAT(ANIMAL_FACTION, x, y);
                }
                else if(T.texture == &tex::mountain) {
                    CREATE_SNOWMAN(ANIMAL_FACTION, x, y);
                }
            }
        }

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

        // Example vision radius in tile-units
        for (int i = 0; i < num_units; i++) {
            Unit &u = units[i];
            if (u.faction != &factions[0] && u.texture!=&tex::warehouse && u.texture!=&tex::esper) continue;  // only the player, warehouses, or espers give vision
            int ux = (int)u.x;
            int uy = (int)u.y;
            float extra_sight = terrainGrid[(int)u.y][(int)u.x].extra_sight;
            if((u.texture==&tex::railgun || u.texture==&tex::radio) && extra_sight<0.f) extra_sight = 0.f;
            if(u.faction->technology&TECHNOLOGY_TRACK) extra_sight += 0.5f;
            float u_range = u.range*(1+extra_sight);
            if((u.texture==&tex::camp || u.texture==&tex::warehouse) && (u.faction->technology & TECHNOLOGY_EXPLORE) && u.faction==factions) u_range = 25.f;
            if(u.faction && (u.faction->technology & TECHNOLOGY_INFRASTRUCTURE) && u.texture==&tex::radio) u_range *= 1.5f;
            int VISION_RADIUS = (int)u_range+2;
            for (int dy = -VISION_RADIUS; dy <= VISION_RADIUS; dy++) {
                for (int dx = -VISION_RADIUS; dx <= VISION_RADIUS; dx++) {
                    int vx = ux + dx;
                    int vy = uy + dy;
                    if (vx < 0 || vy < 0 || vx >= GRID_SIZE || vy >= GRID_SIZE) continue;
                    if (dx*dx + dy*dy <= VISION_RADIUS * VISION_RADIUS) {
                        // if(!explored[vy][vx]) {
                        //     BeginTextureMode(minimap);
                        //     Color c = ColorForTile(terrainGrid[vy][vx].texture);
                        //     DrawRectangle(vx * MINIMAP_SCALE, vy * MINIMAP_SCALE, MINIMAP_SCALE, MINIMAP_SCALE, c);
                        //     EndTextureMode();
                        // }
                        visible[vy][vx] = true;
                        explored[vy][vx] = true;
                    }
                }
            }
        }

        // AI FOR TECH TREE
        for(int i=3;i<max_factions;++i) {
            Faction &F = factions[i];
            if(F.technology_progress < 1.f) continue;
            unsigned long long prev = F.technology;
            unsigned long long chosen = 0;
            int k = GetRandomValue(0, 62);
            unsigned long long candidate = 1ULL << k;
            if(prev & candidate) continue;

            if (candidate == TECHNOLOGY_EXPLORE) chosen = candidate;
            else if (candidate == TECHNOLOGY_HUNTING) chosen = candidate;
            else if (candidate == TECHNOLOGY_NERDS) chosen = candidate;
            else if (candidate == TECHNOLOGY_TAMING) chosen = candidate;
            else if (candidate == TECHNOLOGY_HARDCORE) chosen = candidate;

            else if (candidate == TECHNOLOGY_TRACK && (prev & TECHNOLOGY_EXPLORE)) chosen = candidate;
            else if (candidate == TECHNOLOGY_AGILE && (prev & TECHNOLOGY_EXPLORE)) chosen = candidate;
            else if (candidate == TECHNOLOGY_DRIVER && (prev & TECHNOLOGY_EXPLORE)) chosen = candidate;

            else if (candidate == TECHNOLOGY_WONDER && (prev & (TECHNOLOGY_AGILE | TECHNOLOGY_TAMING))) chosen = candidate;

            else if (candidate == TECHNOLOGY_FIGHT && (prev & TECHNOLOGY_HARDCORE)) chosen = candidate;
            else if (candidate == TECHNOLOGY_FARMING && (prev & TECHNOLOGY_HUNTING)) chosen = candidate;

            else if (candidate == TECHNOLOGY_RESEARCH && (prev & TECHNOLOGY_NERDS)) chosen = candidate;
            else if (candidate == TECHNOLOGY_HOMUNCULI && (prev & TECHNOLOGY_NERDS)) chosen = candidate;
            else if (candidate == TECHNOLOGY_MECHA && (prev & TECHNOLOGY_NERDS)) chosen = candidate;
            else if (candidate == TECHNOLOGY_SPEEDY && (prev & TECHNOLOGY_HELLBRINGER)) chosen = candidate;

            else if (candidate == TECHNOLOGY_AUTOREPAIRS && (prev & TECHNOLOGY_MECHA)) chosen = candidate;
            else if (candidate == TECHNOLOGY_MOBILE_FORTRESS && (prev & TECHNOLOGY_AUTOREPAIRS)) chosen = candidate;
            else if (candidate == TECHNOLOGY_HIJACK && (prev & TECHNOLOGY_AUTOREPAIRS)) chosen = candidate;

            else if (candidate == TECHNOLOGY_HEROICS && (prev & (TECHNOLOGY_FIGHT | TECHNOLOGY_TAMING))) chosen = candidate;
            else if (candidate == TECHNOLOGY_GRIT && (prev & (TECHNOLOGY_HUNTING))) chosen = candidate;
            else if (candidate == TECHNOLOGY_TOUGH && (prev & TECHNOLOGY_FIGHT)) chosen = candidate;

            else if (candidate == TECHNOLOGY_HELLBRINGER && (prev & TECHNOLOGY_HEROICS)) chosen = candidate;

            else if (candidate == TECHNOLOGY_SEAFARERING && (prev & TECHNOLOGY_AGILE)) chosen = candidate;

            else if (candidate == TECHNOLOGY_SNIPING && (prev & TECHNOLOGY_TRACK)) chosen = candidate;
            else if (candidate == TECHNOLOGY_SNIFFING && (prev & TECHNOLOGY_TRACK)) chosen = candidate;

            else if (candidate == TECHNOLOGY_INFRASTRUCTURE && (prev & (TECHNOLOGY_FARMING | TECHNOLOGY_RESEARCH))) chosen = candidate;
            else if (candidate == TECHNOLOGY_OWNERSHIP && (prev & (TECHNOLOGY_SEAFARERING | TECHNOLOGY_INFRASTRUCTURE))) chosen = candidate;

            else if (candidate == TECHNOLOGY_LUXURY && (prev & (TECHNOLOGY_OWNERSHIP | TECHNOLOGY_WONDER))) chosen = candidate;
            else if (candidate == TECHNOLOGY_PROPAGANDA && (prev & (TECHNOLOGY_OWNERSHIP | TECHNOLOGY_HEROICS))) chosen = candidate;
            else if (candidate == TECHNOLOGY_SUPERIORITY && (prev & TECHNOLOGY_PROPAGANDA)) chosen = candidate;

            else if (candidate == TECHNOLOGY_INDUSTRY && (prev & TECHNOLOGY_DRIVER)) chosen = candidate;
            else if (candidate == TECHNOLOGY_GIGAJOULE && (prev & TECHNOLOGY_INDUSTRY)) chosen = candidate;
            else if (candidate == TECHNOLOGY_MECHANISED && (prev & TECHNOLOGY_GIGAJOULE)) chosen = candidate;
            else if (candidate == TECHNOLOGY_TERRAFORIMING && (prev & TECHNOLOGY_GIGAJOULE)) chosen = candidate;

            else if (candidate == TECHNOLOGY_ATMOSPHERE && (prev & TECHNOLOGY_TERRAFORIMING)) chosen = candidate;

            else if (candidate == TECHNOLOGY_UNSTABLE && (prev & TECHNOLOGY_HOMUNCULI)) chosen = candidate;
            else if (candidate == TECHNOLOGY_BIOWEAPON && (prev & TECHNOLOGY_UNSTABLE)) chosen = candidate;
            else if (candidate == TECHNOLOGY_EVOLUTION && (prev & (TECHNOLOGY_BIOWEAPON | TECHNOLOGY_GRIT))) chosen = candidate;
            else if (candidate == TECHNOLOGY_ARTIFICIAL && (prev & TECHNOLOGY_BIOWEAPON)) chosen = candidate;

            else if (candidate == TECHNOLOGY_NUCLEAR && (prev & TECHNOLOGY_RESEARCH)) chosen = candidate;
            else if (candidate == TECHNOLOGY_REACTOR && (prev & TECHNOLOGY_NUCLEAR)) chosen = candidate;
            else if (candidate == TECHNOLOGY_HYPERMAGNET && (prev & TECHNOLOGY_NUCLEAR)) chosen = candidate;
            else if (candidate == TECHNOLOGY_AIFARM && (prev & TECHNOLOGY_INFRASTRUCTURE)) chosen = candidate;
            else if (candidate == TECHNOLOGY_REFINERY && (prev & (TECHNOLOGY_OWNERSHIP | TECHNOLOGY_GIGAJOULE))) chosen = candidate;

            if (chosen) { F.technology |= candidate; F.technology_progress -= 1.f; }
        }


        // ============================================================================
        // AI FOR FACTIONS >= 3
        // ============================================================================
        float AI_ORDER_CHANCE = 0.1f;
        float AI_ORDER_RADIUS = 10.0f;

        for (int i = 0; i < num_units; i++) {
            Unit &u = units[i];
            if (u.health <= 0) continue;
            if (u.speed <= 0) continue;
            if (u.texture==&tex::esper) continue;
            //if (u.target_x != 0 || u.target_y != 0) continue; // already moving → skip
            if (!u.faction) continue;
            if (u.faction==factions && (u.selected || !(u.faction->technology & TECHNOLOGY_SNIFFING))) continue; // disable player AI is sniffing is disabled
            if(u.faction==factions+1) continue;
            if (u.faction == ANIMAL_FACTION) {
                if ((float)GetRandomValue(0, 1000000) / 1000000.0f > AI_ORDER_CHANCE * 20.f * dt) continue;
                Texture2D* baseTex = terrainGrid[(int)u.y][(int)u.x].texture;
                float tx = 0, ty = 0;
                bool found = false;
                float bestDist = 40.f*40.f;
                if(u.max_health<=u.health) bestDist *= 0.25f;
                for (int j = 0; j < num_units; j++) {
                    Unit &o = units[j];
                    if (!o.faction || o.faction==factions+1) continue;
                    if (o.faction==ANIMAL_FACTION) continue;
                    if (o.capturing) continue;
                    if (o.health <= 0) continue;
                    if (time_norm>0.8f && o.texture!=&tex::oil && o.texture!=&tex::warehouse && o.texture!=&tex::esper) continue; // at the last stretch attack the victory locations with all means
                    float dx = o.x - u.x;
                    float dy = o.y - u.y;
                    float d2 = dx*dx + dy*dy;
                    if (d2 < bestDist) {
                        bestDist = d2;
                        if(u.health>u.max_health/2) {
                            tx = o.x;
                            ty = o.y;
                        }
                        else {
                            tx = u.x-(o.x-u.x)+GetRandomValue(-1,1);
                            ty = u.y-(o.y-u.y)+GetRandomValue(-1,1);
                        }
                        found = true;
                    }
                }
                if (!found) {
                    tx = GetRandomValue(10, GRID_SIZE - 10);
                    ty = GetRandomValue(10, GRID_SIZE - 10);
                    if(baseTex==terrainGrid[(int)ty][(int)tx].texture) {
                        u.target_x = tx;
                        u.target_y = ty;
                    }
                }
                else if(bestDist>u.range*u.range) {
                    u.target_x = tx;
                    u.target_y = ty;
                }
                continue;
            }
            if ((float)GetRandomValue(0, 1000000) / 1000000.0f > AI_ORDER_CHANCE * dt) continue;
            // ---------------------------------------------------------
            // Movement
            // ---------------------------------------------------------
            float bestDist = 999999.0f;
            float tx = 0, ty = 0;
            bool found = false;
            // if we are far from target, stop only for stuff that is close to here
            if(u.target_x && u.target_y && (u.target_x-u.x)*(u.target_x-u.x)+(u.target_y-u.y)*(u.target_y-u.y)>10) bestDist = 40.f;
            if(GetRandomValue(0, 100) > 10)
                for (int j = 0; j < num_units; j++) {
                    if(!u.faction->visible_knowledge[j]) continue;
                    Unit &o = units[j];
                    //if (!o.faction) continue;
                    if (o.health <= 0) continue;
                    bool isEnemyCapturable =
                        (o.capturing != nullptr) &&
                        (o.faction != u.faction && o.faction);
                    bool isOwnDamagedStructure =
                        (o.faction == u.faction) &&
                        (o.speed == 0 && o.capturing) &&
                        (o.health < o.max_health*0.8f);
                    if (!isEnemyCapturable && !isOwnDamagedStructure && o.faction) continue;
                    if (time_norm>0.85f && o.texture!=&tex::oil && o.texture!=&tex::warehouse) continue; // at the last stretch attack the victory locations with all means
                    float dx = o.x - u.x;
                    float dy = o.y - u.y;
                    float d2 = dx*dx + dy*dy;
                    if (d2 < bestDist && d2>=0) {
                        bestDist = d2;
                        tx = o.x;
                        ty = o.y;
                        found = true;
                    }
                }
            if (!found) {
                if(GetRandomValue(0, 100)<5 ) {
                    tx = GetRandomValue(10, GRID_SIZE - 10);
                    ty = GetRandomValue(10, GRID_SIZE - 10);
                    u.target_x = tx;
                    u.target_y = ty;
                }
                continue;
            }
            u.target_x = tx;
            u.target_y = ty;

            // ---------------------------------------------------------
            // Order nearby units of same faction to follow
            // ---------------------------------------------------------
            for (int j = 0; j < num_units; j++) {
                if (i == j) continue;
                Unit &o = units[j];
                if (!o.faction) continue;
                if (o.faction != u.faction) continue;
                if (o.health <= 0) continue;
                if (o.speed <= 0) continue;
                if (o.texture==&tex::esper) continue;
                float dx = o.x - u.x;
                float dy = o.y - u.y;
                // grab everything if the game has progressed enough
                if (dx*dx + dy*dy <= AI_ORDER_RADIUS * AI_ORDER_RADIUS && (GetRandomValue(0, 100) < 10 || time_norm>0.5f) && !o.selected) {
                    // only order units not already moving
                    o.target_x = tx;
                    o.target_y = ty;
                }
            }
        }



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
        Color shadow_color = Fade(GRAY, 0.5f);
        float t = GetTime();
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
                DrawCircle(px, py, radius, u.selected?Fade(u.faction->color, 0.5f):shadow_color);//u.health < u.max_health?shadow_color_damaged:shadow_color);
            if (u.health < u.max_health) {
                int maxHealth = (int)(u.max_health+0.01f);
                int hp = (int)(u.health+0.01);
                float startBase = 0;//u.angle-45.f+180.f;
                float segmentAngle = 360.0f / maxHealth;  // 18°
                float gap = 75.f/maxHealth;
                Color hc = u.capturing?u.faction->color:RED;
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
        //         DrawText(TextFormat("Cover: %.0f\%", ter.speed>0.f?(1.0-ter.speed)*100:0.f), px + pad, py + pad, fontSize, WHITE);
        //         DrawText(TextFormat("Sight: +%.0f\%", ter.extra_sight*100), px + pad, py + pad + fontSize + 2, fontSize,WHITE);
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
            if((u.texture==&tex::railgun || u.texture==&tex::radio) && extra_sight<0.f) extra_sight = 0.f;
            if(u.faction->technology&TECHNOLOGY_TRACK) extra_sight += 0.7f;
            float u_range = u.range*(1+extra_sight);
            // important that here we extend the sight range only for stuff controlled by the player
            if((u.texture==&tex::camp || u.texture==&tex::warehouse) && (u.faction->technology & TECHNOLOGY_EXPLORE) && u.faction==factions) u_range = 25.f;
            if(u.faction && (u.faction->technology & TECHNOLOGY_INFRASTRUCTURE) && u.texture==&tex::radio) u_range *= 1.5f;
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
        }

        // ---------------------
        // MINIMAP DRAWING
        // ---------------------
        // if(!showTechTree)
        // {
        //     float miniSize = 330.0f;   // onscreen size
        //     float padding = 20.0f;
        //     Rectangle src = {0,0,(float)minimap.texture.width,-(float)minimap.texture.height};
        //     Rectangle dst = {32,(float)GetScreenHeight() - miniSize - 350,miniSize,miniSize};
        //     DrawTexturePro(
        //         minimap.texture,
        //         src,
        //         dst,
        //         {0,0},
        //         0.0f,
        //         WHITE
        //     );
        //     DrawRectangleLines(dst.x, dst.y, dst.width, dst.height, WHITE);
        //     // ------------------------------------------------------------
        //     // DRAW CAMERA VIEWPORT OVER MINIMAP
        //     // ------------------------------------------------------------
        //     {
        //         // 1. Determine world coords visible on screen
        //         Vector2 worldTL = GetScreenToWorld2D({0,0}, camera);
        //         Vector2 worldBR = GetScreenToWorld2D(
        //             {(float)GetScreenWidth(), (float)GetScreenHeight()},
        //             camera
        //         );
        //
        //         // Convert world coords → tile coords
        //         float tileTLx = worldTL.x / TILE_SIZE;
        //         float tileTLy = worldTL.y / TILE_SIZE;
        //         float tileBRx = worldBR.x / TILE_SIZE;
        //         float tileBRy = worldBR.y / TILE_SIZE;
        //
        //         // 2. Scale into minimap-space
        //         float scaleX = dst.width  / (float)GRID_SIZE;
        //         float scaleY = dst.height / (float)GRID_SIZE;
        //
        //         float boxX = dst.x + tileTLx * scaleX;
        //         float boxY = dst.y + tileTLy * scaleY;
        //         float boxW = (tileBRx - tileTLx) * scaleX;
        //         float boxH = (tileBRy - tileTLy) * scaleY;
        //
        //         DrawRectangleLines(boxX, boxY, boxW, boxH, BLUE);
        //     }
        // }

        if (showTechTree) {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.85f));
            Faction &F = factions[0];
            unsigned long long tech = F.technology;
            unsigned long long prev_tech = F.technology;

            //F.technology_progress += 1; // for debug

            const float actual_cell_width = GetScreenWidth()/6-40;
            const float actual_cell_height = (GetScreenHeight()/14-20)/2;
            const float ICON_SIZE = actual_cell_height;
            const float ICON_DX = actual_cell_width-ICON_SIZE-3.f;
            const float ICON_DY = 3.f;
            float top = actual_cell_height;

            const float DX = GetScreenWidth()/6.0f;
            const float DY = GetScreenHeight()/15.0f;
            float cx = GetScreenWidth() * 0.5f-DX+20.f;

            // ROOTS
            Vector2 explore   = { cx - 2*DX, top+DY };
            Vector2 hunting   = { cx - 2*DX, top+7*DY };
            Vector2 nerds     = { cx - 2*DX, top+10*DY };
            Vector2 taming    = { cx - 2*DX, top+4*DY };

            // SECOND TIER
            Vector2 track     = { explore.x+DX, explore.y };
            Vector2 agile     = { explore.x+DX, explore.y + DY*2};
            Vector2 driver    = { explore.x+DX, explore.y + DY };
            Vector2 wonder    = { agile.x+DX, agile.y+DY};
            Vector2 fight     = { hunting.x+DX, hunting.y-DY};
            Vector2 farming   = { hunting.x+DX, hunting.y+DY};
            Vector2 research  = { nerds.x + DX, nerds.y-DY };
            Vector2 homunculi = { nerds.x + DX, nerds.y };
            Vector2 mecha     = { nerds.x + DX, nerds.y+DY };

            // THIRD TIER
            Vector2 heroics    = { fight.x+DX,      fight.y-DY };
            Vector2 grit       = { fight.x+DX,      fight.y+DY };
            Vector2 tough      = { fight.x+DX,      fight.y };
            Vector2 infrastructure = { research.x + DX, research.y-DY };
            Vector2 aifarm     = { infrastructure.x + 2*DX, infrastructure.y };
            Vector2 technocracy= { aifarm.x + DX, aifarm.y };
            Vector2 unstable   = { homunculi.x + DX, homunculi.y };

            Vector2 seafaring  = { agile.x+DX,      agile.y };
            Vector2 hellbringer= { heroics.x+DX*2,  heroics.y+DY };
            Vector2 speedy     = { hellbringer.x+DX,hellbringer.y };

            Vector2 sniping    = { track.x+DX,      track.y };
            Vector2 sniffing   = { track.x+DX,      track.y-DY };
            Vector2 hardcore   = { hunting.x,       hunting.y-DY };
            Vector2 ownership  = { seafaring.x + DX, seafaring.y };

            Vector2 luxury     = { ownership.x+DX,   ownership.y+DY };
            Vector2 refinery   = { ownership.x+DX,   ownership.y };
            Vector2 propaganda = { ownership.x+DX,   ownership.y+DY*2 };
            Vector2 superiority= { propaganda.x+DX,  propaganda.y };

            Vector2 autorepair  = { mecha.x + DX, mecha.y };
            Vector2 mobile      = { autorepair.x + DX, autorepair.y };
            Vector2 hijack      = { autorepair.x + DX, autorepair.y + DY };
            Vector2 industry    = { driver.x + DX, driver.y };
            Vector2 gigajoule   = { industry.x + DX, industry.y };
            Vector2 terraforming= { gigajoule.x+DX,   gigajoule.y - DY };
            Vector2 mechanised  = { gigajoule.x + DX, gigajoule.y };
            Vector2 bioweapon   = { unstable.x + DX, unstable.y};
            Vector2 nuclear     = { research.x + DX, research.y};
            Vector2 reactor     = { nuclear.x + DX, nuclear.y};
            Vector2 hypermagnet = { reactor.x + DX, reactor.y};
            Vector2 evolution   = { bioweapon.x + DX, bioweapon.y-DY*3};
            Vector2 artificial  = { bioweapon.x + DX, bioweapon.y};
            Vector2 atmosphere  = { terraforming.x+DX, terraforming.y};;

            // EXPLORE
            DrawConnector(explore.x+actual_cell_width, explore.y+actual_cell_height, track.x, track.y+actual_cell_height, tech & TECHNOLOGY_EXPLORE);
            DrawConnector(explore.x+actual_cell_width, explore.y+actual_cell_height, agile.x, agile.y+actual_cell_height, tech & TECHNOLOGY_EXPLORE);
            DrawConnector(explore.x+actual_cell_width, explore.y+actual_cell_height, driver.x, driver.y+actual_cell_height, tech & TECHNOLOGY_EXPLORE);
            DrawConnector(agile.x+actual_cell_width, agile.y+actual_cell_height, wonder.x, wonder.y+actual_cell_height, tech & TECHNOLOGY_AGILE);

            // HUNTING
            DrawConnector(hunting.x+actual_cell_width, hunting.y+actual_cell_height, farming.x, farming.y+actual_cell_height, tech & TECHNOLOGY_HUNTING);
            DrawConnector(taming.x+actual_cell_width, taming.y+actual_cell_height, heroics.x, heroics.y+actual_cell_height, tech & TECHNOLOGY_TAMING);

            // NERDS
            DrawConnector(nerds.x+actual_cell_width, nerds.y+actual_cell_height, research.x, research.y+actual_cell_height, tech & TECHNOLOGY_NERDS);
            DrawConnector(nerds.x+actual_cell_width, nerds.y+actual_cell_height, homunculi.x, homunculi.y+actual_cell_height, tech & TECHNOLOGY_NERDS);
            DrawConnector(nerds.x+actual_cell_width, nerds.y+actual_cell_height, mecha.x, mecha.y+actual_cell_height, tech & TECHNOLOGY_NERDS);

            // HARDCORE
            DrawConnector(hardcore.x+actual_cell_width, hardcore.y+actual_cell_height, fight.x, fight.y+actual_cell_height, tech & TECHNOLOGY_HARDCORE);

            // FIGHT
            DrawConnector(fight.x+actual_cell_width, fight.y+actual_cell_height, heroics.x, heroics.y+actual_cell_height, tech & TECHNOLOGY_FIGHT);
            DrawConnector(hunting.x+actual_cell_width, hunting.y+actual_cell_height, grit.x, grit.y+actual_cell_height, tech & TECHNOLOGY_HUNTING);
            DrawConnector(fight.x+actual_cell_width, fight.y+actual_cell_height, tough.x, tough.y+actual_cell_height, tech & TECHNOLOGY_FIGHT);
            DrawConnector(taming.x+actual_cell_width, taming.y+actual_cell_height, wonder.x, wonder.y+actual_cell_height, tech & TECHNOLOGY_TAMING);
            DrawConnector(wonder.x+actual_cell_width, wonder.y+actual_cell_height, luxury.x, luxury.y+actual_cell_height, tech & TECHNOLOGY_WONDER);

            // AGILE
            DrawConnector(agile.x+actual_cell_width, agile.y+actual_cell_height, seafaring.x, seafaring.y+actual_cell_height, tech & TECHNOLOGY_AGILE);
            //DrawConnector(agile.x+actual_cell_width, agile.y+actual_cell_height, speedy.x, speedy.y+actual_cell_height, tech & TECHNOLOGY_AGILE);
            DrawConnector(seafaring.x+actual_cell_width, seafaring.y+actual_cell_height, ownership.x, ownership.y+actual_cell_height, tech & TECHNOLOGY_SEAFARERING);
            DrawConnector(heroics.x+actual_cell_width, heroics.y+actual_cell_height, hellbringer.x, hellbringer.y+actual_cell_height, tech & TECHNOLOGY_HEROICS);
            DrawConnector(hellbringer.x+actual_cell_width, hellbringer.y+actual_cell_height, speedy.x, speedy.y+actual_cell_height, tech & TECHNOLOGY_HELLBRINGER);

            //INFRA & OWNERSHIP
            DrawConnector(farming.x+actual_cell_width, farming.y+actual_cell_height, infrastructure.x, infrastructure.y+actual_cell_height, tech & TECHNOLOGY_FARMING);
            DrawConnector(infrastructure.x+actual_cell_width, infrastructure.y+actual_cell_height, aifarm.x, aifarm.y+actual_cell_height, tech & TECHNOLOGY_INFRASTRUCTURE);
            DrawConnector(aifarm.x+actual_cell_width, aifarm.y+actual_cell_height, technocracy.x, technocracy.y+actual_cell_height, tech & TECHNOLOGY_AIFARM);
            DrawConnector(mechanised.x+actual_cell_width, mechanised.y+actual_cell_height, technocracy.x, technocracy.y+actual_cell_height, tech & TECHNOLOGY_MECHANISED);
            DrawConnector(research.x+actual_cell_width, research.y+actual_cell_height, infrastructure.x, infrastructure.y+actual_cell_height, tech & TECHNOLOGY_RESEARCH);
            DrawConnector(infrastructure.x+actual_cell_width, infrastructure.y+actual_cell_height, ownership.x, ownership.y+actual_cell_height, tech & TECHNOLOGY_INFRASTRUCTURE);
            DrawConnector(ownership.x+actual_cell_width, ownership.y+actual_cell_height, propaganda.x, propaganda.y+actual_cell_height, tech & TECHNOLOGY_OWNERSHIP);
            DrawConnector(ownership.x+actual_cell_width, ownership.y+actual_cell_height, luxury.x, luxury.y+actual_cell_height, tech & TECHNOLOGY_OWNERSHIP);
            DrawConnector(ownership.x+actual_cell_width, ownership.y+actual_cell_height, refinery.x, refinery.y+actual_cell_height, tech & TECHNOLOGY_OWNERSHIP);
            DrawConnector(gigajoule.x+actual_cell_width, gigajoule.y+actual_cell_height, refinery.x, refinery.y+actual_cell_height, tech & TECHNOLOGY_GIGAJOULE);
            DrawConnector(gigajoule.x+actual_cell_width, gigajoule.y+actual_cell_height, mechanised.x, mechanised.y+actual_cell_height, tech & TECHNOLOGY_GIGAJOULE);
            DrawConnector(gigajoule.x+actual_cell_width, gigajoule.y+actual_cell_height, terraforming.x, terraforming.y+actual_cell_height, tech & TECHNOLOGY_GIGAJOULE);
            DrawConnector(terraforming.x+actual_cell_width, terraforming.y+actual_cell_height, atmosphere.x, atmosphere.y+actual_cell_height, tech & TECHNOLOGY_TERRAFORIMING);



            //UNSTABLE
            DrawConnector(homunculi.x+actual_cell_width, homunculi.y+actual_cell_height, unstable.x, unstable.y+actual_cell_height, tech & TECHNOLOGY_HOMUNCULI);
            DrawConnector(unstable.x+actual_cell_width, unstable.y+actual_cell_height, bioweapon.x, bioweapon.y+actual_cell_height, tech & TECHNOLOGY_UNSTABLE);
            DrawConnector(research.x+actual_cell_width, research.y+actual_cell_height, nuclear.x, nuclear.y+actual_cell_height, tech & TECHNOLOGY_RESEARCH);
            DrawConnector(nuclear.x+actual_cell_width, nuclear.y+actual_cell_height, reactor.x, reactor.y+actual_cell_height, tech & TECHNOLOGY_NUCLEAR);
            DrawConnector(bioweapon.x+actual_cell_width, bioweapon.y+actual_cell_height, evolution.x, evolution.y+actual_cell_height, tech & TECHNOLOGY_BIOWEAPON);
            DrawConnector(bioweapon.x+actual_cell_width, bioweapon.y+actual_cell_height, artificial.x, artificial.y+actual_cell_height, tech & TECHNOLOGY_BIOWEAPON);
            DrawConnector(grit.x+actual_cell_width, grit.y+actual_cell_height, evolution.x, evolution.y+actual_cell_height, tech & TECHNOLOGY_GRIT);
            DrawConnector(reactor.x+actual_cell_width, reactor.y+actual_cell_height, hypermagnet.x, hypermagnet.y+actual_cell_height, tech & TECHNOLOGY_REACTOR);


            // MOBILE FORTRESS
            DrawConnector(driver.x+actual_cell_width, driver.y+actual_cell_height, industry.x, industry.y+actual_cell_height, tech & TECHNOLOGY_DRIVER);
            DrawConnector(mecha.x+actual_cell_width, mecha.y+actual_cell_height, autorepair.x, autorepair.y+actual_cell_height, tech & TECHNOLOGY_MECHA);
            DrawConnector(autorepair.x+actual_cell_width, autorepair.y+actual_cell_height, mobile.x, mobile.y+actual_cell_height, tech & TECHNOLOGY_AUTOREPAIRS);
            DrawConnector(autorepair.x+actual_cell_width, autorepair.y+actual_cell_height, hijack.x, hijack.y+actual_cell_height, tech & TECHNOLOGY_AUTOREPAIRS);
            DrawConnector(industry.x+actual_cell_width, industry.y+actual_cell_height, gigajoule.x, gigajoule.y+actual_cell_height, tech & TECHNOLOGY_INDUSTRY);
            DrawConnector(heroics.x+actual_cell_width, heroics.y+actual_cell_height, propaganda.x, propaganda.y+actual_cell_height, tech & TECHNOLOGY_HEROICS);
            DrawConnector(propaganda.x+actual_cell_width, propaganda.y+actual_cell_height, superiority.x, superiority.y+actual_cell_height, tech & TECHNOLOGY_PROPAGANDA);
            // TRACK
            DrawConnector(track.x+actual_cell_width, track.y+actual_cell_height, sniping.x, sniping.y+actual_cell_height, tech & TECHNOLOGY_TRACK);
            DrawConnector(track.x+actual_cell_width, track.y+actual_cell_height, sniffing.x, sniffing.y+actual_cell_height, tech & TECHNOLOGY_TRACK);


            DrawTechNode(explore.x, explore.y, "CHARTED", "Wide camp & storage sight", tech, TECHNOLOGY_EXPLORE);
            DrawTextureEx(tex::track, {explore.x + ICON_DX, explore.y + ICON_DY}, 0, ICON_SIZE / tex::track.width, WHITE);


            DrawTechNode(hunting.x, hunting.y, "CIVILIZED", "+3 camp & hide industry", tech, TECHNOLOGY_HUNTING);
            DrawTextureEx(tex::gear, {hunting.x + ICON_DX, hunting.y + ICON_DY}, 0, ICON_SIZE / tex::gear.width, WHITE);

            DrawTechNode(nerds.x,   nerds.y,   "NERDS",   "+30\% research, -1 spawn HP", tech, TECHNOLOGY_NERDS);
            DrawTextureEx(tex::research, {nerds.x + ICON_DX, nerds.y + ICON_DY}, 0, ICON_SIZE / tex::research.width, WHITE);


            DrawTechNode(taming.x, taming.y, "TAMER", "-50\% research, 50\% taming", tech, TECHNOLOGY_TAMING);
            DrawTextureEx(tex::bison, {taming.x + ICON_DX, taming.y + ICON_DY}, 0, ICON_SIZE / tex::bison.width, WHITE);


            DrawTechNode(hardcore.x, hardcore.y, "HARDCORE", "-7 camp industry, 2x spawn", tech, TECHNOLOGY_HARDCORE);
            DrawTextureEx(tex::blood, {hardcore.x + ICON_DX, hardcore.y + ICON_DY}, 0, ICON_SIZE / tex::blood.width, WHITE);

            // account for datacenters autonomously adding techs without predecesoors
            if(prev_tech & (TECHNOLOGY_BIOWEAPON | TECHNOLOGY_GRIT | TECHNOLOGY_EVOLUTION)) {
                DrawTechNode(evolution.x, evolution.y, "EVOLUTION", "Some spawn are snowmen", tech, TECHNOLOGY_EVOLUTION);
                DrawTextureEx(tex::snowman, {evolution.x + ICON_DX, evolution.y + ICON_DY}, 0, ICON_SIZE / tex::snowman.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_EXPLORE | TECHNOLOGY_TRACK)) {
                DrawTechNode(track.x, track.y, "TRACKER", "+50\% unit sight", tech, TECHNOLOGY_TRACK);
                DrawTextureEx(tex::track, {track.x + ICON_DX, track.y + ICON_DY}, 0, ICON_SIZE / tex::track.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_AGILE | TECHNOLOGY_TAMING | TECHNOLOGY_WONDER)) {
                DrawTechNode(wonder.x, wonder.y, "WONDER", "Discoveries grant research", tech, TECHNOLOGY_WONDER);
                DrawTextureEx(tex::track, {wonder.x + ICON_DX, wonder.y + ICON_DY}, 0, ICON_SIZE / tex::track.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_EXPLORE | TECHNOLOGY_AGILE)) {
                DrawTechNode(agile.x, agile.y, "AGILE", "Terrain slows less", tech, TECHNOLOGY_AGILE);
                DrawTextureEx(tex::track, {agile.x + ICON_DX, agile.y + ICON_DY}, 0, ICON_SIZE / tex::track.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_HUNTING | TECHNOLOGY_FARMING)) {
                DrawTechNode(farming.x, farming.y, "FARMING", "Fields stay long in bloom", tech, TECHNOLOGY_FARMING);
                DrawTextureEx(tex::gear, {farming.x + ICON_DX, farming.y + ICON_DY}, 0, ICON_SIZE / tex::gear.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_EXPLORE | TECHNOLOGY_DRIVER)) {
                DrawTechNode(driver.x, driver.y, "DRIVER", "Mechas are often faster", tech, TECHNOLOGY_DRIVER);
                DrawTextureEx(tex::tank, {driver.x + ICON_DX, driver.y + ICON_DY}, 0, ICON_SIZE / tex::tank.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_HARDCORE | TECHNOLOGY_FIGHT)) {
                DrawTechNode(fight.x, fight.y, "FIGHT", "Double unit experience", tech, TECHNOLOGY_FIGHT);
                DrawTextureEx(tex::blood, {fight.x + ICON_DX, fight.y + ICON_DY}, 0, ICON_SIZE / tex::blood.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_NERDS | TECHNOLOGY_RESEARCH)) {
                DrawTechNode(research.x, research.y, "RESEARCH", "+30\% research", tech, TECHNOLOGY_RESEARCH);
                DrawTextureEx(tex::research, {research.x + ICON_DX, research.y + ICON_DY}, 0, ICON_SIZE / tex::research.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_NERDS | TECHNOLOGY_HOMUNCULI)) {
                DrawTechNode(homunculi.x, homunculi.y, "HOMUNCULI", "Killed bloos become allies", tech, TECHNOLOGY_HOMUNCULI);
                DrawTextureEx(tex::ghost, {homunculi.x + ICON_DX, homunculi.y + ICON_DY}, 0, ICON_SIZE / tex::ghost.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_NERDS | TECHNOLOGY_MECHA)) {
                DrawTechNode(mecha.x, mecha.y, "MECHA", "+50\% mecha doge", tech, TECHNOLOGY_MECHA);
                DrawTextureEx(tex::tank, {mecha.x + ICON_DX, mecha.y + ICON_DY}, 0, ICON_SIZE / tex::tank.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_MECHA | TECHNOLOGY_AUTOREPAIRS)) {
                DrawTechNode(autorepair.x, autorepair.y, "AUTOREPAIR", "Mecha regen HP", tech, TECHNOLOGY_AUTOREPAIRS);
                DrawTextureEx(tex::tank, {autorepair.x + ICON_DX, autorepair.y + ICON_DY}, 0, ICON_SIZE / tex::tank.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_FIGHT | TECHNOLOGY_TAMING | TECHNOLOGY_HEROICS)) {
                DrawTechNode(heroics.x, heroics.y, "HEROICS", "+30\% vet & heroes dodge", tech, TECHNOLOGY_HEROICS);
                DrawTextureEx(tex::blood, {heroics.x + ICON_DX, heroics.y + ICON_DY}, 0, ICON_SIZE / tex::blood.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_HUNTING | TECHNOLOGY_GRIT)) {
                DrawTechNode(grit.x, grit.y, "GRIT", "+50\% dodge vs lethal", tech, TECHNOLOGY_GRIT);
                DrawTextureEx(tex::blood, {grit.x + ICON_DX, grit.y + ICON_DY}, 0, ICON_SIZE / tex::blood.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_FIGHT | TECHNOLOGY_TOUGH)) {
                DrawTechNode(tough.x, tough.y, "TOUGH", "+1 human HP", tech, TECHNOLOGY_TOUGH);
                DrawTextureEx(tex::blood, {tough.x + ICON_DX, tough.y + ICON_DY}, 0, ICON_SIZE / tex::blood.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_HEROICS | TECHNOLOGY_HELLBRINGER)) {
                DrawTechNode(hellbringer.x, hellbringer.y, "HELLBRINGER", "Rapid hero and veteran fire", tech, TECHNOLOGY_HELLBRINGER);
                DrawTextureEx(tex::blood, {hellbringer.x + ICON_DX, hellbringer.y + ICON_DY}, 0, ICON_SIZE / tex::blood.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_HELLBRINGER | TECHNOLOGY_SPEEDY)) {
                DrawTechNode(speedy.x, speedy.y, "360 ANGLE", "Rotate faster", tech, TECHNOLOGY_SPEEDY);
                DrawTextureEx(tex::track, {speedy.x + ICON_DX, speedy.y + ICON_DY}, 0, ICON_SIZE / tex::track.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_AGILE | TECHNOLOGY_SEAFARERING)) {
                DrawTechNode(seafaring.x, seafaring.y, "SEAFARING", "Water increases speed", tech, TECHNOLOGY_SEAFARERING);
                DrawTextureEx(tex::track, {seafaring.x + ICON_DX, seafaring.y + ICON_DY}, 0, ICON_SIZE / tex::track.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_FARMING | TECHNOLOGY_RESEARCH | TECHNOLOGY_INFRASTRUCTURE)) {
                DrawTechNode(infrastructure.x, infrastructure.y, "MEDIA", "+50\% radio sight", tech, TECHNOLOGY_INFRASTRUCTURE);
                DrawTextureEx(tex::track, {infrastructure.x + ICON_DX, infrastructure.y + ICON_DY}, 0, ICON_SIZE / tex::track.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_SEAFARERING | TECHNOLOGY_INFRASTRUCTURE | TECHNOLOGY_OWNERSHIP)) {
                DrawTechNode(ownership.x, ownership.y, "OWNERSHIP", "Better hold captured assets", tech, TECHNOLOGY_OWNERSHIP);
                DrawTextureEx(tex::blood, {ownership.x + ICON_DX, ownership.y + ICON_DY}, 0, ICON_SIZE / tex::blood.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_HOMUNCULI | TECHNOLOGY_UNSTABLE)) {
                DrawTechNode(unstable.x, unstable.y, "UNSTABLE", "-1 spawn HP, bloo on death", tech, TECHNOLOGY_UNSTABLE);
                DrawTextureEx(tex::ghost, {unstable.x + ICON_DX, unstable.y + ICON_DY}, 0, ICON_SIZE / tex::ghost.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_INFRASTRUCTURE | TECHNOLOGY_AIFARM)) {
                DrawTechNode(aifarm.x, aifarm.y, "AI FARM", "+16 industry from labs", tech, TECHNOLOGY_AIFARM);
                DrawTextureEx(tex::gear, {aifarm.x + ICON_DX, aifarm.y + ICON_DY}, 0, ICON_SIZE / tex::gear.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_AIFARM | TECHNOLOGY_MECHANISED | TECHNOLOGY_TECHNOCRACY)) {
                DrawTechNode(technocracy.x, technocracy.y, "TECHNOCRACY", "+1 utopia per 100 industry", tech, TECHNOLOGY_TECHNOCRACY);
                DrawTextureEx(tex::utopia, {technocracy.x + ICON_DX, technocracy.y + ICON_DY}, 0, ICON_SIZE / tex::utopia.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_DRIVER | TECHNOLOGY_INDUSTRY)) {
                DrawTechNode(industry.x, industry.y, "LOST INDUSTRY", "Mechas gain experience", tech, TECHNOLOGY_INDUSTRY);
                DrawTextureEx(tex::tank, {industry.x + ICON_DX, industry.y + ICON_DY}, 0, ICON_SIZE / tex::tank.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_UNSTABLE | TECHNOLOGY_BIOWEAPON)) {
                DrawTechNode(bioweapon.x, bioweapon.y, "BIOWEAPON", "Kills become bloo", tech, TECHNOLOGY_BIOWEAPON);
                DrawTextureEx(tex::ghost, {bioweapon.x + ICON_DX, bioweapon.y + ICON_DY}, 0, ICON_SIZE / tex::ghost.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_TRACK | TECHNOLOGY_SNIPING)) {
                DrawTechNode(sniping.x, sniping.y, "SNIPING", "x2 accuracy vs dodge", tech, TECHNOLOGY_SNIPING);
                DrawTextureEx(tex::blood, {sniping.x + ICON_DX, sniping.y + ICON_DY}, 0, ICON_SIZE / tex::blood.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_TRACK | TECHNOLOGY_SNIFFING)) {
                DrawTechNode(sniffing.x, sniffing.y, "BOTNET", "Autonomous units", tech, TECHNOLOGY_SNIFFING);
                DrawTextureEx(tex::track, {sniffing.x + ICON_DX, sniffing.y + ICON_DY}, 0, ICON_SIZE / tex::track.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_RESEARCH | TECHNOLOGY_NUCLEAR)) {
                DrawTechNode(nuclear.x, nuclear.y, "NUCLEAR", "x2 damage for no regen", tech, TECHNOLOGY_NUCLEAR);
                DrawTextureEx(tex::blood, {nuclear.x + ICON_DX, nuclear.y + ICON_DY}, 0, ICON_SIZE / tex::blood.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_AUTOREPAIRS | TECHNOLOGY_MOBILE_FORTRESS)) {
                DrawTechNode(mobile.x, mobile.y, "MOBILE FORT", "Railguns may become tanks", tech, TECHNOLOGY_MOBILE_FORTRESS);
                DrawTextureEx(tex::tank, {mobile.x + ICON_DX, mobile.y + ICON_DY}, 0, ICON_SIZE / tex::tank.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_OWNERSHIP | TECHNOLOGY_WONDER | TECHNOLOGY_LUXURY)) {
                DrawTechNode(luxury.x, luxury.y, "PRISTINE", "+50\% dodge vs animal & bloo", tech, TECHNOLOGY_LUXURY);
                DrawTextureEx(tex::research, {luxury.x + ICON_DX, luxury.y + ICON_DY}, 0, ICON_SIZE / tex::research.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_GIGAJOULE | TECHNOLOGY_TERRAFORIMING)) {
                DrawTechNode(terraforming.x, terraforming.y, "TERRAFORMING", "Captures become fields", tech, TECHNOLOGY_TERRAFORIMING);
                DrawTextureEx(tex::field, {terraforming.x + ICON_DX, terraforming.y + ICON_DY}, 0, ICON_SIZE / tex::field.width, WHITE);
            }
            if(prev_tech & TECHNOLOGY_TERRAFORIMING) DrawTechNode(atmosphere.x, atmosphere.y, "ATMOSPHERE v2.0", "Fields delay polution", tech, TECHNOLOGY_ATMOSPHERE);
            if(prev_tech & (TECHNOLOGY_OWNERSHIP | TECHNOLOGY_GIGAJOULE | TECHNOLOGY_REFINERY)) {
                DrawTechNode(refinery.x, refinery.y, "REFINERY", "+25 industry from oil", tech, TECHNOLOGY_REFINERY);
                DrawTextureEx(tex::gear, {refinery.x + ICON_DX, refinery.y + ICON_DY}, 0, ICON_SIZE / tex::gear.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_OWNERSHIP | TECHNOLOGY_HEROICS | TECHNOLOGY_PROPAGANDA)) {
                DrawTechNode(propaganda.x, propaganda.y, "PROPAGANDA", "+1 utopia per 3 radios", tech, TECHNOLOGY_PROPAGANDA);
                DrawTextureEx(tex::utopia, {propaganda.x + ICON_DX, propaganda.y + ICON_DY}, 0, ICON_SIZE / tex::utopia.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_PROPAGANDA | TECHNOLOGY_SUPERIORITY)) {
                DrawTechNode(superiority.x, superiority.y, "SUPERIORITY", "+1 utopia", tech, TECHNOLOGY_SUPERIORITY);
                DrawTextureEx(tex::utopia, {superiority.x + ICON_DX, superiority.y + ICON_DY}, 0, ICON_SIZE / tex::utopia.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_INDUSTRY | TECHNOLOGY_GIGAJOULE)) {
                DrawTechNode(gigajoule.x, gigajoule.y, "GIGAJOULE", "No mecha industry cost", tech, TECHNOLOGY_GIGAJOULE);
                DrawTextureEx(tex::gear, {gigajoule.x + ICON_DX, gigajoule.y + ICON_DY}, 0, ICON_SIZE / tex::gear.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_GIGAJOULE | TECHNOLOGY_MECHANISED)) {
                DrawTechNode(mechanised.x, mechanised.y, "MECHANIZATION", "+1 industry per 5 mecha HP", tech, TECHNOLOGY_MECHANISED);
                DrawTextureEx(tex::gear, {mechanised.x + ICON_DX, mechanised.y + ICON_DY}, 0, ICON_SIZE / tex::gear.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_NUCLEAR | TECHNOLOGY_REACTOR)) {
                DrawTechNode(reactor.x, reactor.y, "REACTOR", "+40 industry", tech, TECHNOLOGY_REACTOR);
                DrawTextureEx(tex::gear, {reactor.x + ICON_DX, reactor.y + ICON_DY}, 0, ICON_SIZE / tex::gear.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_REACTOR | TECHNOLOGY_HYPERMAGNET)) {
                DrawTechNode(hypermagnet.x, hypermagnet.y, "HYPERMAGNET", "x2 industry cost and speed", tech, TECHNOLOGY_HYPERMAGNET);
                DrawTextureEx(tex::track, {hypermagnet.x + ICON_DX, hypermagnet.y + ICON_DY}, 0, ICON_SIZE / tex::track.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_BIOWEAPON | TECHNOLOGY_ARTIFICIAL)) {
                DrawTechNode(artificial.x, artificial.y, "HIVEMIND", "Fast bloo experience", tech, TECHNOLOGY_ARTIFICIAL);
                DrawTextureEx(tex::ghost, {artificial.x + ICON_DX, artificial.y + ICON_DY}, 0, ICON_SIZE / tex::ghost.width, WHITE);
            }
            if(prev_tech & (TECHNOLOGY_AUTOREPAIRS | TECHNOLOGY_HIJACK)) {
                DrawTechNode(hijack.x, hijack.y, "HIJACK", "Often capture mechas", tech, TECHNOLOGY_HIJACK);
                DrawTextureEx(tex::tank, {hijack.x + ICON_DX, hijack.y + ICON_DY}, 0, ICON_SIZE / tex::tank.width, WHITE);
            }

            if(tech!=F.technology && F.technology_progress>=1.f) {
                F.technology_progress -= 1.f;
                F.technology = tech;
                PlaySound(sound::select);
            }
        }


        if (showHelp) {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.85f));
            float px = GetScreenWidth()*0.5f-600;
            float py = GetScreenHeight()*0.5f-340;
            DrawRectangleRounded(Rectangle{px-5, py-3, 260, 60}, 0.2f, 8, Fade(WHITE, 0.5f));
            DrawText("What to do",px,py,52,BLACK);
            py += 80;
            DrawText("Your units capture stuff - explore to find more",px,py,32,WHITE);
            py += 40;
            DrawText("Gather the most utopia points before pollution fills",px,py,32,WHITE);
            py += 40;
            DrawText("Spawning stops if you exceed available industry",px,py,32,WHITE);
            py += 40;
            DrawText("Select techs once research fills",px,py,32,WHITE);
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
            DrawText("Right click to move (units auto-attack and may stop when near target)",px+200,py,32,WHITE);
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
                    DrawText("Pollution progress", bar_bg.x + 20, bar_bg.y + 10, 32, WHITE);
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
        if(!showTechTree && hovered) {
            float panelSize = 320.0f;
            Vector2 mouse = GetMousePosition();
            float px = mouse.x - panelSize * 0.5f;
            float py = mouse.y - panelSize * 0.8f - 64;
            Rectangle src = { 0, 0, (float)tex::info.width, (float)tex::info.height };
            Rectangle dst = { px, py, panelSize, panelSize };
            Vector2 origin = { 0, 0 };
            DrawTexturePro(tex::info, src, dst, origin, 0.0f, hovered->faction?hovered->faction->color:BLACK);
            float textY = py - 26;
            px -= 55;
            if (hovered && hovered->health) {
                DrawText(hovered->name, px + 80, textY + 80, 42, WHITE);
                {
                    Texture2D* t = hovered->texture;
                    float size = 48.0f;
                    float rot = GetTime() * 60.0f; // degrees per second
                    Rectangle src = { 0, 0, (float)t->width, (float)t->height };
                    Rectangle dst = {
                        px + 308,
                        textY + 102,
                        size,
                        size
                    };
                    Vector2 origin = { size * 0.5f, size * 0.5f };
                    DrawTexturePro(*t, src, dst, origin, rot, WHITE);
                }
                textY += 140;
                if(hovered->texture==&tex::camp) {
                    DrawText("Spawns humans if", px + 80, textY, 28, WHITE);
                    DrawText("below industry cap", px + 80, textY+30, 28, WHITE);
                    if(hovered->faction==factions+1)
                        DrawTextSmall("wild", px + 80, textY+94, 20, WHITE);
                    else if(hovered->faction==factions)
                        DrawTextSmall("yours", px + 80, textY+94, 20, WHITE);
                    else
                        DrawTextSmall("AI", px + 80, textY+94, 20, WHITE);
                    for(float f=0;f<hovered->health/2;f+=1.0f)
                        DrawRectangleRounded({px+150.f+7.f*f, textY+99.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                }
                else if(hovered->texture==&tex::lab) {
                    DrawText("+10% research", px + 80, textY, 28, WHITE);
                    if(hovered->faction==factions+1)
                        DrawTextSmall("wild", px + 80, textY+94, 20, WHITE);
                    else if(hovered->faction==factions)
                        DrawTextSmall("yours", px + 80, textY+94, 20, WHITE);
                    else
                        DrawTextSmall("enemy", px + 80, textY+94, 20, WHITE);
                    for(float f=0;f<hovered->health/2;f+=1.0f)
                        DrawRectangleRounded({px+150.f+7.f*f, textY+99.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                }
                else if(hovered->texture==&tex::field) {
                    DrawText("+4 industry (bloom)", px + 80, textY, 28, WHITE);
                    DrawText("Irrational crop cycle", px + 80, textY+30, 28, WHITE);
                    if(hovered->faction==factions+1)
                        DrawTextSmall("wild", px + 80, textY+94, 20, WHITE);
                    else if(hovered->faction==factions)
                        DrawTextSmall("yours", px + 80, textY+94, 20, WHITE);
                    else
                        DrawTextSmall("enemy", px + 80, textY+94, 20, WHITE);
                    for(float f=0;f<hovered->health/2;f+=1.0f)
                        DrawRectangleRounded({px+150.f+7.f*f, textY+99.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                }
                else if(hovered->texture==&tex::field_little) {
                    DrawText("+2 industry (grows)", px + 80, textY, 28, WHITE);
                    DrawText("Irrational crop cycle", px + 80, textY+30, 28, WHITE);
                    if(hovered->faction==factions+1)
                        DrawTextSmall("wild", px + 80, textY+94, 20, WHITE);
                    else if(hovered->faction==factions)
                        DrawTextSmall("yours", px + 80, textY+94, 20, WHITE);
                    else
                        DrawTextSmall("enemy", px + 80, textY+94, 20, WHITE);
                    for(float f=0;f<hovered->health/2;f+=1.0f)
                        DrawRectangleRounded({px+150.f+7.f*f, textY+99.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                }
                else if(hovered->texture==&tex::field_empty) {
                    DrawText("+0 industry (barren)", px + 80, textY, 28, WHITE);
                    DrawText("Irrational crop cycle", px + 80, textY+30, 28, WHITE);
                    if(hovered->faction==factions+1)
                        DrawTextSmall("wild", px + 80, textY+94, 20, WHITE);
                    else if(hovered->faction==factions)
                        DrawTextSmall("yours", px + 80, textY+94, 20, WHITE);
                    else
                        DrawTextSmall("enemy", px + 80, textY+94, 20, WHITE);
                    for(float f=0;f<hovered->health/2;f+=1.0f)
                        DrawRectangleRounded({px+150.f+7.f*f, textY+99.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                }
                else if(hovered->texture==&tex::hide) {
                    DrawText("+4 industry", px + 80, textY, 28, WHITE);
                    DrawText("May become rats", px + 80, textY+30, 28, WHITE);
                    if(hovered->faction==factions+1)
                        DrawTextSmall("wild", px + 80, textY+94, 20, WHITE);
                    else if(hovered->faction==factions)
                        DrawTextSmall("yours", px + 80, textY+94, 20, WHITE);
                    else
                        DrawTextSmall("enemy", px + 80, textY+94, 20, WHITE);
                    for(float f=0;f<hovered->health/2;f+=1.0f)
                        DrawRectangleRounded({px+150.f+7.f*f, textY+99.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                }
                else if(hovered->texture==&tex::mine) {
                    DrawText("+12 industry", px + 80, textY, 28, WHITE);
                    if(hovered->faction==factions+1)
                        DrawTextSmall("wild", px + 80, textY+94, 20, WHITE);
                    else if(hovered->faction==factions)
                        DrawTextSmall("yours", px + 80, textY+94, 20, WHITE);
                    else
                        DrawTextSmall("enemy", px + 80, textY+94, 20, WHITE);
                    for(float f=0;f<hovered->health/2;f+=1.0f)
                        DrawRectangleRounded({px+150.f+7.f*f, textY+99.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                }
                else if(hovered->texture==&tex::oil) {
                    DrawText("+3 utopia", px + 80, textY, 28, WHITE);
                    if(hovered->faction==factions+1)
                        DrawTextSmall("wild", px + 80, textY+94, 20, WHITE);
                    else if(hovered->faction==factions)
                        DrawTextSmall("yours", px + 80, textY+94, 20, WHITE);
                    else
                        DrawTextSmall("enemy", px + 80, textY+94, 20, WHITE);
                    for(float f=0;f<hovered->health/2;f+=1.0f)
                        DrawRectangleRounded({px+150.f+7.f*f, textY+99.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                }
                else if(hovered->texture==&tex::datacenter) {
                    DrawText("Random benefits", px + 80, textY, 28, WHITE);
                    if(hovered->faction==factions+1)
                        DrawTextSmall("wild", px + 80, textY+94, 20, WHITE);
                    else if(hovered->faction==factions)
                        DrawTextSmall("yours", px + 80, textY+94, 20, WHITE);
                    else
                        DrawTextSmall("enemy", px + 80, textY+94, 20, WHITE);
                    for(float f=0;f<hovered->health/2;f+=1.0f)
                        DrawRectangleRounded({px+150.f+7.f*f, textY+99.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                }
                else if(hovered->texture==&tex::warehouse) {
                    DrawText("+2 utopia", px + 80, textY, 28, WHITE);
                    if(hovered->faction==factions+1)
                        DrawTextSmall("wild", px + 80, textY+94, 20, WHITE);
                    else if(hovered->faction==factions)
                        DrawTextSmall("yours", px + 80, textY+94, 20, WHITE);
                    else
                        DrawTextSmall("enemy", px + 80, textY+94, 20, WHITE);
                    for(float f=0;f<hovered->health/2;f+=1.0f)
                        DrawRectangleRounded({px+150.f+7.f*f, textY+99.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                }
                else if(hovered->texture==&tex::railgun) {
                    DrawText("Mecha, stationary", px + 80, textY, 28, WHITE);
                    DrawText("No sight penalty", px + 80, textY+30, 28, WHITE);
                    DrawTextSmall("combat", px + 80, textY+62, 20, WHITE);
                    for(float f=0;f<hovered->damage*hovered->attack_rate;f+=0.5f)
                        DrawRectangleRounded({px+150.f+14.f*f, textY+67.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                    DrawTextSmall("sight", px + 80, textY+78, 20, WHITE);
                    for(float f=0;f<hovered->range/2;f+=0.5f)
                        DrawRectangleRounded({px+150.f+14.f*f, textY+83.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                    if(hovered->faction==factions+1)
                        DrawTextSmall("wild", px + 80, textY+94, 20, WHITE);
                    else
                        DrawTextSmall("health", px + 80, textY+94, 20, WHITE);
                    for(float f=0;f<hovered->health/2;f+=1.0f)
                        DrawRectangleRounded({px+150.f+7.f*f, textY+99.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                }
                else if(hovered->texture==&tex::roomba) {
                    DrawText("Mecha vs animals & bloo", px + 80, textY, 28, WHITE);
                    DrawTextSmall("speed", px + 80, textY+62-16, 20, WHITE);
                    for(float f=0;f<hovered->speed;f+=1.0f)
                        DrawRectangleRounded({px+150.f+7.f*f, textY+67.f-16, 5.f, 10.f}, 0.2f, 8, WHITE);
                    DrawTextSmall("combat", px + 80, textY+62, 20, WHITE);
                    for(float f=0;f<hovered->damage*hovered->attack_rate;f+=0.5f)
                        DrawRectangleRounded({px+150.f+14.f*f, textY+67.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                    DrawTextSmall("sight", px + 80, textY+78, 20, WHITE);
                    for(float f=0;f<hovered->range/2;f+=0.5f)
                        DrawRectangleRounded({px+150.f+14.f*f, textY+83.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                    if(hovered->faction==factions+1)
                        DrawTextSmall("wild", px + 80, textY+94, 20, WHITE);
                    else
                        DrawTextSmall("health", px + 80, textY+94, 20, WHITE);
                    for(float f=0;f<hovered->health/2;f+=1.0f)
                        DrawRectangleRounded({px+150.f+7.f*f, textY+99.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                }
                else if(hovered->texture==&tex::esper) {
                    DrawText("+2 utopia, unruly", px + 80, textY, 28, WHITE);
                    DrawTextSmall("speed", px + 80, textY+62-16, 20, WHITE);
                    for(float f=0;f<hovered->speed;f+=1.0f)
                        DrawRectangleRounded({px+150.f+7.f*f, textY+67.f-16, 5.f, 10.f}, 0.2f, 8, WHITE);
                    DrawTextSmall("combat", px + 80, textY+62, 20, WHITE);
                    for(float f=0;f<hovered->damage*hovered->attack_rate;f+=0.5f)
                        DrawRectangleRounded({px+150.f+14.f*f, textY+67.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                    DrawTextSmall("sight", px + 80, textY+78, 20, WHITE);
                    for(float f=0;f<hovered->range/2;f+=0.5f)
                        DrawRectangleRounded({px+150.f+14.f*f, textY+83.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                    if(hovered->faction==factions+1)
                        DrawTextSmall("wild", px + 80, textY+94, 20, WHITE);
                    else
                        DrawTextSmall("health", px + 80, textY+94, 20, WHITE);
                    for(float f=0;f<hovered->health/2;f+=1.0f)
                        DrawRectangleRounded({px+150.f+7.f*f, textY+99.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                }
                else if(is_mecha((*hovered)) && hovered->speed==0) {
                    DrawText("Mecha", px + 80, textY, 28, WHITE);
                    DrawTextSmall("speed", px + 80, textY+62-16, 20, WHITE);
                    for(float f=0;f<hovered->speed;f+=1.0f)
                        DrawRectangleRounded({px+150.f+7.f*f, textY+67.f-16, 5.f, 10.f}, 0.2f, 8, WHITE);
                    DrawTextSmall("combat", px + 80, textY+62, 20, WHITE);
                    for(float f=0;f<hovered->damage*hovered->attack_rate;f+=0.5f)
                        DrawRectangleRounded({px+150.f+14.f*f, textY+67.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                    DrawTextSmall("sight", px + 80, textY+78, 20, WHITE);
                    for(float f=0;f<hovered->range/2;f+=0.5f)
                        DrawRectangleRounded({px+150.f+14.f*f, textY+83.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                    if(hovered->faction==factions+1)
                        DrawTextSmall("wild", px + 80, textY+94, 20, WHITE);
                    else
                        DrawTextSmall("health", px + 80, textY+94, 20, WHITE);
                    for(float f=0;f<hovered->health/2;f+=1.0f)
                        DrawRectangleRounded({px+150.f+7.f*f, textY+99.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                }
                else if(is_mecha((*hovered))) {
                    DrawText("Mecha, fills industry", px + 80, textY, 28, WHITE);
                    DrawTextSmall("speed", px + 80, textY+62-16, 20, WHITE);
                    for(float f=0;f<hovered->speed;f+=1.0f)
                        DrawRectangleRounded({px+150.f+7.f*f, textY+67.f-16, 5.f, 10.f}, 0.2f, 8, WHITE);
                    DrawTextSmall("combat", px + 80, textY+62, 20, WHITE);
                    for(float f=0;f<hovered->damage*hovered->attack_rate;f+=0.5f)
                        DrawRectangleRounded({px+150.f+14.f*f, textY+67.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                    DrawTextSmall("sight", px + 80, textY+78, 20, WHITE);
                    for(float f=0;f<hovered->range/2;f+=0.5f)
                        DrawRectangleRounded({px+150.f+14.f*f, textY+83.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                    if(hovered->faction==factions+1)
                        DrawTextSmall("wild", px + 80, textY+94, 20, WHITE);
                    else
                        DrawTextSmall("health", px + 80, textY+94, 20, WHITE);
                    for(float f=0;f<hovered->health/2;f+=1.0f)
                        DrawRectangleRounded({px+150.f+7.f*f, textY+99.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                }
                else if(hovered->damage) {
                    if(hovered->range<3.f) {
                        if(hovered->texture==&tex::snowman) DrawText("Animal, fast on hills", px + 80, textY, 28, WHITE);
                        else if(hovered->texture==&tex::rat) DrawText("Animal, proliferates", px + 80, textY, 28, WHITE);
                        else if(hovered->texture==&tex::wolf) DrawText("Animal, 50\% taming", px + 80, textY, 28, WHITE);
                        else DrawText("Animal, drops hide", px + 80, textY, 28, WHITE);
                        DrawTextSmall("speed", px + 80, textY+62-16, 20, WHITE);
                        for(float f=0;f<hovered->speed;f+=1.0f)
                            DrawRectangleRounded({px+150.f+7.f*f, textY+67.f-16, 5.f, 10.f}, 0.2f, 8, WHITE);
                        DrawTextSmall("combat", px + 80, textY+62, 20, WHITE);
                        for(float f=0;f<hovered->damage*hovered->attack_rate;f+=0.5f)
                            DrawRectangleRounded({px+150.f+14.f*f, textY+67.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                        DrawTextSmall("sight", px + 80, textY+78, 20, WHITE);
                        for(float f=0;f<hovered->range/2;f+=0.5f)
                            DrawRectangleRounded({px+150.f+14.f*f, textY+83.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                        DrawTextSmall("health", px + 80, textY+94, 20, WHITE);
                        for(float f=0;f<hovered->health/2;f+=1.0f)
                            DrawRectangleRounded({px+150.f+7.f*f, textY+99.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                    }
                    else {
                        DrawText("Fills industry", px + 80, textY, 28, WHITE);
                        DrawTextSmall("speed", px + 80, textY+62-16, 20, WHITE);
                        for(float f=0;f<hovered->speed;f+=1.0f)
                            DrawRectangleRounded({px+150.f+7.f*f, textY+67.f-16, 5.f, 10.f}, 0.2f, 8, WHITE);
                        DrawTextSmall("combat", px + 80, textY+62, 20, WHITE);
                        for(float f=0;f<hovered->damage*hovered->attack_rate;f+=0.5f)
                            DrawRectangleRounded({px+150.f+14.f*f, textY+67.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                        DrawTextSmall("sight", px + 80, textY+78, 20, WHITE);
                        for(float f=0;f<hovered->range/2;f+=0.5f)
                            DrawRectangleRounded({px+150.f+14.f*f, textY+83.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                        DrawTextSmall("health", px + 80, textY+94, 20, WHITE);
                        for(float f=0;f<hovered->health/2;f+=1.0f)
                            DrawRectangleRounded({px+150.f+7.f*f, textY+99.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                    }
                    // if(hovered->name==veteran_name)
                    //     DrawText("Stronger", px + 80, textY+60, 28, WHITE);
                    // if(hovered->name==hero_name)
                    //     DrawText("Mighty", px + 80, textY+60, 28, WHITE);
                }
                else {
                    DrawText("Grants sight", px + 80, textY, 28, WHITE);
                    DrawText("No sight penalty", px + 80, textY+30, 28, WHITE);
                    if(hovered->faction==factions+1)
                        DrawTextSmall("wild", px + 80, textY+94, 20, WHITE);
                    else if(hovered->faction==factions)
                        DrawTextSmall("yours", px + 80, textY+94, 20, WHITE);
                    else
                        DrawTextSmall("enemy", px + 80, textY+94, 20, WHITE);
                    for(float f=0;f<hovered->health/2;f+=1.0f)
                        DrawRectangleRounded({px+150.f+7.f*f, textY+99.f, 5.f, 10.f}, 0.2f, 8, WHITE);
                }
            }
            else {
                DrawText("No info", px + 80, textY + 80, 42, WHITE);
                DrawText("Mouse over a unit.", px + 80, textY + 150, 28, WHITE);
            }
        }


        // --------------------------------------------------
        // CLUSTERING BUTTON
        // --------------------------------------------------
        if(!showTechTree) {
            Vector2 mouse = GetMousePosition();
            bool hover = CheckCollisionPointRec(mouse, clusteringBtn);

            // background
            DrawRectangleRounded(
                clusteringBtn,
                0.2f,
                8,
                hover ? Fade(DARKGRAY, 0.5f) : Fade(BLACK, 0.5f)
            );
            //DrawRectangleRoundedLines(clusteringBtn, 0.2f, 8, 2, WHITE);

            // icon: three humans with increasing spacing
            float cx = clusteringBtn.x + 40;
            float cy = clusteringBtn.y + 70;
            for (int i = 0; i < 3; i++) {
                float dx = i * 45.0f;
                if(currentMovementMode==MovementMode::Scattered) dx += i*16.f;
                if(currentMovementMode==MovementMode::Explore) dx += i*32.f;
                Rectangle src = {0, 0, (float)tex::human.width, (float)tex::human.height};
                Rectangle dst = {cx + dx, cy, 32, 32};
                DrawTexturePro(tex::human, src, dst, {dst.width / 2, dst.height / 2}, -90.f, WHITE);
            }

            // label
            DrawText(
                //currentMovementMode==MovementMode::Explore?"Explore":currentMovementMode==MovementMode::Tight?"Tight move":"Scattered",
                "Formation",
                clusteringBtn.x + 20,
                clusteringBtn.y + 10,
                32,
                WHITE
            );



            DrawRectangleRounded({clusteringBtn.x+clusteringBtn.width-70, clusteringBtn.y+15, 50, 25},0.2f, 8, WHITE);
            DrawTextSmall("spc",clusteringBtn.x+clusteringBtn.width-65+5,clusteringBtn.y+15,24,BLACK);

        }
        EndDrawing();
    }

    free(terrainBlock);
    free(terrainGrid);
    unload();
    return 0;
}
