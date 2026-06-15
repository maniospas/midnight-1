#pragma once
#include <raylib.h>
#include "texture.cpp"
#include "faction.cpp"

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
    Texture* popup_texture;
    float animation;
};

#define CREATE_HUMAN(faction, x, y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = { \
            &tex::human,  /* texture */ \
            "Human",      /* name */ \
            GetRandomValue(50,150)*0.05f,         /* speed */ \
            (float)(x),   /* x */ \
            (float)(y),   /* y */ \
            GetRandomValue(50,150)*0.01f,          /* attack_rate */ \
            GetRandomValue(50,150)*0.04f,         /* range */ \
            GetRandomValue(50,150)*0.01f,          /* damage */ \
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
            0.6,          /* size */ \
            20.0,         /* health */ \
            20.0,         /* max_health */ \
            (faction),    /* faction */ \
            (faction),    /* faction */ \
            0.2           /* extra scale*/\
        };


#define CREATE_HOVERCRAFT(faction, x, y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = { \
            &tex::hovercraft,   /* texture */ \
            "Hovercraft",       /* name */ \
            7.0,          /* speed */ \
            (float)(x),   /* x */ \
            (float)(y),   /* y */ \
            3.0,          /* attack_rate */ \
            4.0,          /* range */ \
            2.0,          /* damage */ \
            0.0,          /* experience */ \
            (float)GetRandomValue(0,360),          /* angle */ \
            0.7,          /* size */ \
            8.0,          /* health */ \
            8.0,          /* max_health */ \
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
#define CREATE_ROCK(faction, x, y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = { \
            &tex::rock,   /* texture */ \
            "Rock",       /* name */ \
            0.0,          /* speed */ \
            (float)(x),   /* x */ \
            (float)(y),   /* y */ \
            0.0,          /* attack_rate */ \
            1.0,          /* range */ \
            1.0,          /* damage */ \
            0.0,          /* experience */ \
            (float)GetRandomValue(0,360),          /* angle */ \
            0.3,          /* size */ \
            7.0,         /* health */ \
            7.0,         /* max_health */ \
            (faction),    /* faction */ \
            nullptr,      /* faction */ \
            0.3           /* extra scale*/\
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

#define CREATE_FORT(faction, x, y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = { \
            &tex::fort,   /* texture */ \
            "Fort",       /* name */ \
            0.0,          /* speed */ \
            (float)(x),   /* x */ \
            (float)(y),   /* y */ \
            2,          /* attack_rate */ \
            6.0,          /* range */ \
            2.0,          /* damage */ \
            0.0,          /* experience */ \
            0.0,          /* angle */ \
            0.7,          /* size */ \
            20.0,         /* health */ \
            20.0,         /* max_health */ \
            (faction),    /* faction */ \
            (faction),    /* faction */ \
            0.0           /* extra scale*/\
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
                &tex::field_empty,   /* texture */ \
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

#define CREATE_LIGHTHOUSE(faction, x, y) \
    if (num_units < MAX_UNITS) {\
        units[num_units++] = { \
            &tex::lighthouse,   /* texture */ \
            "Big bro",       /* name */ \
            0.0,          /* speed */ \
            (float)(x),   /* x */ \
            (float)(y),   /* y */ \
            0.0,          /* attack_rate */ \
            2.0,          /* range */ \
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


#define CREATE_CURIO(faction, x, y) \
    if (num_units < MAX_UNITS) {\
        units[num_units++] = { \
            &tex::curio,   /* texture */ \
            "Curio",  /* name */ \
            0.0,          /* speed */ \
            (float)(x),   /* x */ \
            (float)(y),   /* y */ \
            0.0,          /* attack_rate */ \
            5,          /* range */ \
            0.0,          /* damage */ \
            0.0,          /* experience */ \
            0.0,          /* angle */ \
            1.0,          /* size */ \
            300.0,         /* health */ \
            300.0,         /* max_health */ \
            (faction),    /* faction */ \
            (faction)     /* can only be captured */ \
        };\
        for(int ppy=-2;ppy<=2;ppy++) \
            for(int ppx=-2;ppx<=2;ppx++) \
                if((ppy*ppy)+(ppx*ppx)<=4) terrainGrid[(int)(y+0.5f)+ppy][(int)(x+0.5f)+ppx] = terrainGrid[(int)(y+0.5f)][(int)(x+0.5f)]; \
    }
