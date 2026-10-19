#pragma once
#include <raylib.h>
#include "texture.cpp"
#include "faction.cpp"
//#include <cstdint>
//#include <cmath>
#define _Float16 float

// struct Number100 {uint8_t value;};
// float number2float(Number100 x) {return static_cast<float>(x.value) / 100.0f;}
// Number100 float2number(float x) {return Number100{.value = static_cast<uint8_t>(std::round(x * 100.0f))};}

struct Unit {
    Texture2D* texture;
    Faction* faction;
    const char* name;
    float speed;
    float x,y;
    float angle;
    _Float16 attack_rate;
    _Float16 range;
    _Float16 damage;
    _Float16 experience;
    _Float16 size;
    _Float16 health;
    _Float16 max_health;
    _Float16 extra_scale;

    // leave the stuff below zero-initialized (target coords with zero values will just be skipped by convention)
    Faction* capturing; // set this to non-null to indicate that damage captures. in this case, nothing attacks it
    Texture* popup_texture;
    const char* popup;
    float attack_x, attack_y;
    _Float16 stunned; // no issue to lose some slight precision
    _Float16 animation; // no issue to lose some slight precision
    _Float16 ai_decision_countdown;
    _Float16 attack_target_x, attack_target_y;
    _Float16 target_x, target_y;
    int8_t selected; // only to mark selection by player
};

#define CREATE_HUMAN(_faction, _x, _y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = Unit{ \
            .texture = &tex::human, \
            .faction = (_faction), \
            .name = "Human", \
            .speed = GetRandomValue(50,150)*0.05f, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = (float)GetRandomValue(0,360), \
            .attack_rate = static_cast<_Float16>(GetRandomValue(50,150)*0.01f), \
            .range = static_cast<_Float16>(GetRandomValue(50,150)*0.04f), \
            .damage = static_cast<_Float16>(GetRandomValue(50,150)*0.01f), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(0.3), \
            .health = static_cast<_Float16>(5.0), \
            .max_health = static_cast<_Float16>(5.0) \
        };

#define CREATE_CAT(_faction, _x, _y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = Unit{ \
            .texture = &tex::cat, \
            .faction = (_faction), \
            .name = "Cat", \
            .speed = GetRandomValue(50,150)*0.05f+3.f, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = (float)GetRandomValue(0,360), \
            .attack_rate = static_cast<_Float16>(GetRandomValue(50,150)*0.015f), \
            .range = static_cast<_Float16>(GetRandomValue(50,150)*0.04f), \
            .damage = static_cast<_Float16>(GetRandomValue(50,150)*0.01f), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(0.3), \
            .health = static_cast<_Float16>(5.0), \
            .max_health = static_cast<_Float16>(5.0) \
        };

#define CREATE_ESPER(_faction, _x, _y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = Unit{ \
            .texture = &tex::esper, \
            .faction = (_faction), \
            .name = "Esper", \
            .speed = 5.0, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = (float)GetRandomValue(0,360), \
            .attack_rate = static_cast<_Float16>(2.0), \
            .range = static_cast<_Float16>(4.0), \
            .damage = static_cast<_Float16>(3.0), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(0.4), \
            .health = static_cast<_Float16>(15.0), \
            .max_health = static_cast<_Float16>(15.0), \
            .capturing = (_faction) \
        };

#define CREATE_TANK(_faction, _x, _y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = Unit{ \
            .texture = &tex::tank, \
            .faction = (_faction), \
            .name = "Tank", \
            .speed = 3.0, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = (float)GetRandomValue(0,360), \
            .attack_rate = static_cast<_Float16>(0.5), \
            .range = static_cast<_Float16>(6.0), \
            .damage = static_cast<_Float16>(8.0), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(0.8), \
            .health = static_cast<_Float16>(20.0), \
            .max_health = static_cast<_Float16>(20.0), \
            .extra_scale = static_cast<_Float16>(0.4), \
            .capturing = (_faction) \
        };

#define CREATE_ROOMBA(_faction, _x, _y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = Unit{ \
            .texture = &tex::roomba, \
            .faction = (_faction), \
            .name = "Roomba", \
            .speed = 7.0, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = (float)GetRandomValue(0,360), \
            .attack_rate = static_cast<_Float16>(1.0), \
            .range = static_cast<_Float16>(3.0), \
            .damage = static_cast<_Float16>(2.0), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(0.4), \
            .health = static_cast<_Float16>(10.0), \
            .max_health = static_cast<_Float16>(10.0), \
            .extra_scale = static_cast<_Float16>(0.2), \
            .capturing = (_faction) \
        };

#define CREATE_VAN(_faction, _x, _y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = Unit{ \
            .texture = &tex::van, \
            .faction = (_faction), \
            .name = "Van", \
            .speed = 9.0, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = (float)GetRandomValue(0,360), \
            .attack_rate = static_cast<_Float16>(1.0), \
            .range = static_cast<_Float16>(6.0), \
            .damage = static_cast<_Float16>(1.0), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(0.6), \
            .health = static_cast<_Float16>(20.0), \
            .max_health = static_cast<_Float16>(20.0), \
            .extra_scale = static_cast<_Float16>(0.2), \
            .capturing = (_faction) \
        };


#define CREATE_HOVERCRAFT(_faction, _x, _y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = Unit{ \
            .texture = &tex::hovercraft, \
            .faction = (_faction), \
            .name = "Hovercraft", \
            .speed = 7.0, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = (float)GetRandomValue(0,360), \
            .attack_rate = static_cast<_Float16>(3.0), \
            .range = static_cast<_Float16>(4.0), \
            .damage = static_cast<_Float16>(2.0), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(0.7), \
            .health = static_cast<_Float16>(8.0), \
            .max_health = static_cast<_Float16>(8.0), \
            .extra_scale = static_cast<_Float16>(0.3), \
            .capturing = (_faction) \
        };


#define CREATE_SNOWMAN(_faction, _x, _y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = Unit{ \
            .texture = &tex::snowman, \
            .faction = (_faction), \
            .name = "Snowman", \
            .speed = 3.0, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = (float)GetRandomValue(0,360), \
            .attack_rate = static_cast<_Float16>(2.0), \
            .range = static_cast<_Float16>(2.5), \
            .damage = static_cast<_Float16>(1.0), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(0.6), \
            .health = static_cast<_Float16>(10.0), \
            .max_health = static_cast<_Float16>(10.0), \
            .extra_scale = static_cast<_Float16>(0.2), \
            .capturing = nullptr \
        };

#define CREATE_BISON(_faction, _x, _y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = Unit{ \
            .texture = &tex::bison, \
            .faction = (_faction), \
            .name = "Bison", \
            .speed = 4.3, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = (float)GetRandomValue(0,360), \
            .attack_rate = static_cast<_Float16>(2.0), \
            .range = static_cast<_Float16>(1.0), \
            .damage = static_cast<_Float16>(3.0), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(0.6), \
            .health = static_cast<_Float16>(15.0), \
            .max_health = static_cast<_Float16>(15.0), \
            .extra_scale = static_cast<_Float16>(0.3), \
            .capturing = nullptr \
        };

#define CREATE_HOG(_faction, _x, _y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = Unit{ \
            .texture = &tex::hog, \
            .faction = (_faction), \
            .name = "Hog", \
            .speed = 8.0, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = (float)GetRandomValue(0,360), \
            .attack_rate = static_cast<_Float16>(2.0), \
            .range = static_cast<_Float16>(1.0), \
            .damage = static_cast<_Float16>(3.0), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(0.6), \
            .health = static_cast<_Float16>(7.0), \
            .max_health = static_cast<_Float16>(7.0), \
            .extra_scale = static_cast<_Float16>(0.3), \
            .capturing = nullptr \
        };

#define CREATE_KRAKEN(_faction, _x, _y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = Unit{ \
            .texture = &tex::kraken, \
            .faction = (_faction), \
            .name = "Kraken", \
            .speed = 3.0, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = (float)GetRandomValue(0,360), \
            .attack_rate = static_cast<_Float16>(5.0), \
            .range = static_cast<_Float16>(2.0), \
            .damage = static_cast<_Float16>(2.0), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(1.5f), \
            .health = static_cast<_Float16>(100.0), \
            .max_health = static_cast<_Float16>(100.0), \
            .extra_scale = static_cast<_Float16>(1.5f), \
            .capturing = nullptr \
        };

#define CREATE_RAT(_faction, _x, _y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = Unit{ \
            .texture = &tex::rat, \
            .faction = (_faction), \
            .name = "Rat", \
            .speed = 7.0, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = (float)GetRandomValue(0,360), \
            .attack_rate = static_cast<_Float16>(1.0), \
            .range = static_cast<_Float16>(1.0), \
            .damage = static_cast<_Float16>(1.0), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(0.2), \
            .health = static_cast<_Float16>(2.0), \
            .max_health = static_cast<_Float16>(2.0), \
            .extra_scale = static_cast<_Float16>(0.0), \
            .capturing = nullptr \
        };
#define CREATE_ROCK(_faction, _x, _y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = Unit{ \
            .texture = &tex::rock, \
            .faction = (_faction), \
            .name = "Rock", \
            .speed = 0.0, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = (float)GetRandomValue(0,360), \
            .attack_rate = static_cast<_Float16>(0.0), \
            .range = static_cast<_Float16>(1.0), \
            .damage = static_cast<_Float16>(1.0), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(0.3), \
            .health = static_cast<_Float16>(4.0), \
            .max_health = static_cast<_Float16>(4.0), \
            .extra_scale = static_cast<_Float16>(0.3), \
            .capturing = nullptr \
        };


#define CREATE_WOLF(_faction, _x, _y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = Unit{ \
            .texture = &tex::wolf, \
            .faction = (_faction), \
            .name = "Wolf", \
            .speed = 9.0, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = (float)GetRandomValue(0,360), \
            .attack_rate = static_cast<_Float16>(1.0), \
            .range = static_cast<_Float16>(1.0), \
            .damage = static_cast<_Float16>(3.0), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(0.4), \
            .health = static_cast<_Float16>(5.0), \
            .max_health = static_cast<_Float16>(5.0), \
            .extra_scale = static_cast<_Float16>(0.3), \
            .capturing = nullptr \
        };

#define CREATE_FORT(_faction, _x, _y) \
    if (num_units < MAX_UNITS) \
        units[num_units++] = Unit{ \
            .texture = &tex::fort, \
            .faction = (_faction), \
            .name = "Fort", \
            .speed = 0.0, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = 0.0, \
            .attack_rate = static_cast<_Float16>(2), \
            .range = static_cast<_Float16>(6.0), \
            .damage = static_cast<_Float16>(2.0), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(0.7), \
            .health = static_cast<_Float16>(20.0), \
            .max_health = static_cast<_Float16>(20.0), \
            .extra_scale = static_cast<_Float16>(1.0), \
            .capturing = (_faction) \
        };

#define CREATE_RAILGUN(_faction, _x, _y) \
    if (num_units < MAX_UNITS) {\
        units[num_units++] = Unit{ \
            .texture = &tex::railgun, \
            .faction = (_faction), \
            .name = "Railgun", \
            .speed = 0.0, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = (float)GetRandomValue(0,360), \
            .attack_rate = static_cast<_Float16>(5.0), \
            .range = static_cast<_Float16>(6.0), \
            .damage = static_cast<_Float16>(1.5), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(0.65), \
            .health = static_cast<_Float16>(20.0), \
            .max_health = static_cast<_Float16>(20.0), \
            .extra_scale = static_cast<_Float16>(0.2), \
            .capturing = (_faction) \
        };\
        for(int ppy=-2;ppy<=2;ppy++) \
            for(int ppx=-2;ppx<=2;ppx++) \
                if((ppy*ppy)+(ppx*ppx)<=4) terrainGrid[(int)(_y+0.5f)+ppy][(int)(_x+0.5f)+ppx] = terrainGrid[(int)(_y+0.5f)][(int)(_x+0.5f)]; \
    }

#define CREATE_CAMP(_faction, _x, _y) \
    if (num_units < MAX_UNITS) { \
        float bbx = _x; \
        float bby = _y; \
        units[num_units++] = Unit{ \
            .texture = &tex::camp, \
            .faction = (_faction), \
            .name = "Camp", \
            .speed = 0.0, \
            .x = (float)(bbx), \
            .y = (float)(bby), \
            .angle = 0.0, \
            .attack_rate = static_cast<_Float16>(4.0), \
            .range = static_cast<_Float16>(1.0), \
            .damage = static_cast<_Float16>(0.0), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(0.8), \
            .health = static_cast<_Float16>(50.0), \
            .max_health = static_cast<_Float16>(50.0), \
            .capturing = (_faction) \
        }; \
        for(int ppy=-2;ppy<=2;ppy++) \
            for(int ppx=-2;ppx<=2;ppx++) \
                if((ppy*ppy)+(ppx*ppx)<=4) terrainGrid[(int)(bby+0.5f)+ppy][(int)(bbx+0.5f)+ppx] = { &tex::grass, 1.0 }; \
    }

#define CREATE_FIELD(_faction, _x, _y) \
        if (num_units < MAX_UNITS) { \
            float bbx = _x; \
            float bby = _y; \
            units[num_units++] = Unit{ \
                .texture = &tex::field_empty, \
                .faction = (_faction), \
                .name = "Field", \
                .speed = 0.0, \
                .x = (float)(bbx), \
                .y = (float)(bby), \
                .angle = 0.0, \
                .attack_rate = static_cast<_Float16>(0.0), \
                .range = static_cast<_Float16>(4.0), \
                .damage = static_cast<_Float16>(0.0), \
                .experience = static_cast<_Float16>(0.0), \
                .size = static_cast<_Float16>(1.4), \
                .health = static_cast<_Float16>(50.0), \
                .max_health = static_cast<_Float16>(50.0), \
                .extra_scale = static_cast<_Float16>(-0.1f), \
                .capturing = (_faction) \
            }; \
            for(int ppy=-2;ppy<=2;ppy++) \
                for(int ppx=-2;ppx<=2;ppx++) \
                    if((ppy*ppy)+(ppx*ppx)<=4) terrainGrid[(int)(bby+0.5f)+ppy][(int)(bbx+0.5f)+ppx] = { &tex::grass, 1.0 }; \
        }


#define CREATE_FLOWERFIELD(_faction, _x, _y) \
        if (num_units < MAX_UNITS) { \
            float bbx = _x; \
            float bby = _y; \
            units[num_units++] = Unit{ \
                .texture = &tex::flowerfield_empty, \
                .faction = (_faction), \
                .name = "Flowerfield", \
                .speed = 0.0, \
                .x = (float)(bbx), \
                .y = (float)(bby), \
                .angle = 0.0, \
                .attack_rate = static_cast<_Float16>(0.0), \
                .range = static_cast<_Float16>(4.0), \
                .damage = static_cast<_Float16>(0.0), \
                .experience = static_cast<_Float16>(0.0), \
                .size = static_cast<_Float16>(1.4), \
                .health = static_cast<_Float16>(50.0), \
                .max_health = static_cast<_Float16>(50.0), \
                .extra_scale = static_cast<_Float16>(-0.1f), \
                .capturing = (_faction) \
            }; \
            for(int ppy=-2;ppy<=2;ppy++) \
                for(int ppx=-2;ppx<=2;ppx++) \
                    if((ppy*ppy)+(ppx*ppx)<=4) terrainGrid[(int)(bby+0.5f)+ppy][(int)(bbx+0.5f)+ppx] = { &tex::grass, 1.0 }; \
        }

#define CREATE_DATACENTER(_faction, _x, _y) \
    if (num_units < MAX_UNITS) {\
        units[num_units++] = Unit{ \
            .texture = &tex::datacenter, \
            .faction = (_faction), \
            .name = "Databank", \
            .speed = 0.0, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = 0.0, \
            .attack_rate = static_cast<_Float16>(0.0), \
            .range = static_cast<_Float16>(6.0), \
            .damage = static_cast<_Float16>(0.0), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(1.7), \
            .health = static_cast<_Float16>(50.0), \
            .max_health = static_cast<_Float16>(50.0), \
            .extra_scale = static_cast<_Float16>(0.3f), \
            .capturing = (_faction) \
        }; \
        for(int ppy=-2;ppy<=2;ppy++) \
            for(int ppx=-2;ppx<=2;ppx++) \
                if((ppy*ppy)+(ppx*ppx)<=4) terrainGrid[(int)(_y+0.5f)+ppy][(int)(_x+0.5f)+ppx] = terrainGrid[(int)(_y+0.5f)][(int)(_x+0.5f)]; \
    }


#define CREATE_ENGINE(_faction, _x, _y) \
        if (num_units < MAX_UNITS) { \
            float bbx = _x; \
            float bby = _y; \
            units[num_units++] = Unit{ \
                .texture = &tex::engine, \
                .faction = (_faction), \
                .name = "Engine", \
                .speed = 0.0, \
                .x = (float)(bbx), \
                .y = (float)(bby), \
                .angle = 0.0, \
                .attack_rate = static_cast<_Float16>(0.0), \
                .range = static_cast<_Float16>(4.0), \
                .damage = static_cast<_Float16>(0.0), \
                .experience = static_cast<_Float16>(0.0), \
                .size = static_cast<_Float16>(0.7), \
                .health = static_cast<_Float16>(10.0), \
                .max_health = static_cast<_Float16>(10.0), \
                .extra_scale = static_cast<_Float16>(0.2f), \
                .capturing = (_faction) \
            }; \
        }

#define CREATE_SCRAPYARD(_faction, _x, _y) \
    if (num_units < MAX_UNITS) { \
        units[num_units++] = Unit{ \
            .texture = &tex::scrapyard, \
            .faction = (_faction), \
            .name = "Scrapyard", \
            .speed = 0.0, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = 0.0, \
            .attack_rate = static_cast<_Float16>(12.0), \
            .range = static_cast<_Float16>(4.5), \
            .damage = static_cast<_Float16>(0.0), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(2.0), \
            .health = static_cast<_Float16>(50.0), \
            .max_health = static_cast<_Float16>(50.0), \
            .extra_scale = static_cast<_Float16>(0.5), \
            .capturing = (_faction) \
        };\
        for(int ppy=-2;ppy<=2;ppy++) \
            for(int ppx=-2;ppx<=2;ppx++) \
                if((ppy*ppy)+(ppx*ppx)<=4) terrainGrid[(int)(_y+0.5f)+ppy][(int)(_x+0.5f)+ppx] = terrainGrid[(int)(_y+0.5f)][(int)(_x+0.5f)]; \
    }


#define CREATE_MINE(_faction, _x, _y) \
    if (num_units < MAX_UNITS) { \
        units[num_units++] = Unit{ \
            .texture = &tex::mine, \
            .faction = (_faction), \
            .name = "Mine", \
            .speed = 0.0, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = 0.0, \
            .attack_rate = static_cast<_Float16>(12.0), \
            .range = static_cast<_Float16>(4.5), \
            .damage = static_cast<_Float16>(0.0), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(2.0), \
            .health = static_cast<_Float16>(50.0), \
            .max_health = static_cast<_Float16>(50.0), \
            .extra_scale = static_cast<_Float16>(0.5), \
            .capturing = (_faction) \
        };\
        for(int ppy=-2;ppy<=2;ppy++) \
            for(int ppx=-2;ppx<=2;ppx++) \
                if((ppy*ppy)+(ppx*ppx)<=4) terrainGrid[(int)(_y+0.5f)+ppy][(int)(_x+0.5f)+ppx] = terrainGrid[(int)(_y+0.5f)][(int)(_x+0.5f)]; \
    }

#define CREATE_RADIO(_faction, _x, _y) \
    if (num_units < MAX_UNITS) {\
        units[num_units++] = Unit{ \
            .texture = &tex::radio, \
            .faction = (_faction), \
            .name = "Radio", \
            .speed = 0.0, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = 0.0, \
            .attack_rate = static_cast<_Float16>(0.0), \
            .range = static_cast<_Float16>(16.0), \
            .damage = static_cast<_Float16>(0.0), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(1.5), \
            .health = static_cast<_Float16>(50.0), \
            .max_health = static_cast<_Float16>(50.0), \
            .extra_scale = static_cast<_Float16>(0.5), \
            .capturing = (_faction) \
        };\
        for(int ppy=-2;ppy<=2;ppy++) \
            for(int ppx=-2;ppx<=2;ppx++) \
                if((ppy*ppy)+(ppx*ppx)<=4) terrainGrid[(int)(_y+0.5f)+ppy][(int)(_x+0.5f)+ppx] = terrainGrid[(int)(_y+0.5f)][(int)(_x+0.5f)]; \
    }


#define CREATE_HOUSE(_faction, _x, _y) \
    if (num_units < MAX_UNITS && terrainGrid[(int)(_y+0.5f)][(int)(_x+0.5f)].texture==&tex::grass) {\
        Texture* selected_texture = GetRandomValue(0,4)?&tex::house:&tex::house2;\
        units[num_units++] = Unit{ \
            .texture = selected_texture, \
            .faction = (_faction), \
            .name = "House", \
            .speed = 0.0, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = GetRandomValue(0,3)*90.f, \
            .attack_rate = static_cast<_Float16>(0.0), \
            .range = static_cast<_Float16>(3.0), \
            .damage = static_cast<_Float16>(0.0), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(selected_texture==&tex::house?1.0f:1.5f), \
            .health = static_cast<_Float16>(4.0), \
            .max_health = static_cast<_Float16>(4.0), \
            .extra_scale = static_cast<_Float16>(0.25), \
            .capturing = (_faction) \
        };\
        for(int ppy=-2;ppy<=2;ppy++) \
            for(int ppx=-2;ppx<=2;ppx++) \
                if((ppy*ppy)+(ppx*ppx)<=4) terrainGrid[(int)(_y+0.5f)+ppy][(int)(_x+0.5f)+ppx] = terrainGrid[(int)(_y+0.5f)][(int)(_x+0.5f)]; \
    }


#define CREATE_RETREAT(_faction, _x, _y) \
    if (num_units < MAX_UNITS && terrainGrid[(int)(_y+0.5f)][(int)(_x+0.5f)].texture==&tex::grass) {\
        units[num_units++] = Unit{ \
            .texture = &tex::retreat, \
            .faction = (_faction), \
            .name = "Retreat", \
            .speed = 0.0, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = GetRandomValue(0,3)*90.f, \
            .attack_rate = static_cast<_Float16>(0.0), \
            .range = static_cast<_Float16>(3.0), \
            .damage = static_cast<_Float16>(0.0), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(1.5f), \
            .health = static_cast<_Float16>(4.0), \
            .max_health = static_cast<_Float16>(4.0), \
            .extra_scale = static_cast<_Float16>(0.25), \
            .capturing = (_faction) \
        };\
    }


#define CREATE_LIGHTHOUSE(_faction, _x, _y) \
    if (num_units < MAX_UNITS) {\
        units[num_units++] = Unit{ \
            .texture = &tex::lighthouse, \
            .faction = (_faction), \
            .name = "Big bro", \
            .speed = 0.0, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = 0.0, \
            .attack_rate = static_cast<_Float16>(0.0), \
            .range = static_cast<_Float16>(2.0), \
            .damage = static_cast<_Float16>(0.0), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(2.0), \
            .health = static_cast<_Float16>(50.0), \
            .max_health = static_cast<_Float16>(50.0), \
            .extra_scale = static_cast<_Float16>(0.5), \
            .capturing = (_faction) \
        };\
        for(int ppy=-2;ppy<=2;ppy++) \
            for(int ppx=-2;ppx<=2;ppx++) \
                if((ppy*ppy)+(ppx*ppx)<=4) terrainGrid[(int)(_y+0.5f)+ppy][(int)(_x+0.5f)+ppx] = terrainGrid[(int)(_y+0.5f)][(int)(_x+0.5f)]; \
    }


#define CREATE_LAB(_faction, _x, _y) \
    if (num_units < MAX_UNITS) { \
        units[num_units++] = Unit{ \
            .texture = &tex::lab, \
            .faction = (_faction), \
            .name = "Lab", \
            .speed = 0.0, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = 0.0, \
            .attack_rate = static_cast<_Float16>(0.0), \
            .range = static_cast<_Float16>(6.0), \
            .damage = static_cast<_Float16>(0.0), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(2.0), \
            .health = static_cast<_Float16>(50.0), \
            .max_health = static_cast<_Float16>(50.0), \
            .extra_scale = static_cast<_Float16>(0.3), \
            .capturing = (_faction) \
        };\
        for(int ppy=-2;ppy<=2;ppy++) \
            for(int ppx=-2;ppx<=2;ppx++) \
                if((ppy*ppy)+(ppx*ppx)<=4) terrainGrid[(int)(_y+0.5f)+ppy][(int)(_x+0.5f)+ppx] = terrainGrid[(int)(_y+0.5f)][(int)(_x+0.5f)]; \
    }

#define CREATE_OIL(_faction, _x, _y) \
    if (num_units < MAX_UNITS) { \
        units[num_units++] = Unit{ \
            .texture = &tex::oil, \
            .faction = (_faction), \
            .name = "Oil", \
            .speed = 0.0, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = 0.0, \
            .attack_rate = static_cast<_Float16>(0.0), \
            .range = static_cast<_Float16>(1.0), \
            .damage = static_cast<_Float16>(0.0), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(1.2), \
            .health = static_cast<_Float16>(25.0), \
            .max_health = static_cast<_Float16>(25.0), \
            .capturing = (_faction) \
        };\
        for(int ppy=-2;ppy<=2;ppy++) \
            for(int ppx=-2;ppx<=2;ppx++) \
                if((ppy*ppy)+(ppx*ppx)<=4) terrainGrid[(int)(_y+0.5f)+ppy][(int)(_x+0.5f)+ppx] = terrainGrid[(int)(_y+0.5f)][(int)(_x+0.5f)]; \
    }

#define CREATE_WAREHOUSE(_faction, _x, _y) \
    if (num_units < MAX_UNITS) {\
        units[num_units++] = Unit{ \
            .texture = &tex::warehouse, \
            .faction = (_faction), \
            .name = "Storage", \
            .speed = 0.0, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = 0.0, \
            .attack_rate = static_cast<_Float16>(0.0), \
            .range = static_cast<_Float16>(4.5), \
            .damage = static_cast<_Float16>(0.0), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(2.5), \
            .health = static_cast<_Float16>(50.0), \
            .max_health = static_cast<_Float16>(50.0), \
            .extra_scale = static_cast<_Float16>(0.5), \
            .capturing = (_faction) \
        };\
        for(int ppy=-2;ppy<=2;ppy++) \
            for(int ppx=-2;ppx<=2;ppx++) \
                if((ppy*ppy)+(ppx*ppx)<=4) terrainGrid[(int)(_y+0.5f)+ppy][(int)(_x+0.5f)+ppx] = terrainGrid[(int)(_y+0.5f)][(int)(_x+0.5f)]; \
    }

#define CREATE_PYRAMIS(_faction, _x, _y) \
    if (num_units < MAX_UNITS) {\
        units[num_units++] = Unit{ \
            .texture = &tex::pyramis, \
            .faction = (_faction), \
            .name = "Pyramis", \
            .speed = 0.0, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = 0.0, \
            .attack_rate = static_cast<_Float16>(0.0), \
            .range = static_cast<_Float16>(9.0), \
            .damage = static_cast<_Float16>(0.0), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(2.5), \
            .health = static_cast<_Float16>(50.0), \
            .max_health = static_cast<_Float16>(50.0), \
            .extra_scale = static_cast<_Float16>(0.5), \
            .capturing = (_faction) \
        };\
        for(int ppy=-2;ppy<=2;ppy++) \
            for(int ppx=-2;ppx<=2;ppx++) \
                if((ppy*ppy)+(ppx*ppx)<=4) terrainGrid[(int)(_y+0.5f)+ppy][(int)(_x+0.5f)+ppx] = terrainGrid[(int)(_y+0.5f)][(int)(_x+0.5f)]; \
    }

#define CREATE_TOWNHALL(_faction, _x, _y) \
    if (num_units < MAX_UNITS) {\
        units[num_units++] = Unit{ \
            .texture = &tex::villa, \
            .faction = (_faction), \
            .name = "Townhall", \
            .speed = 0.0, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = 0.0, \
            .attack_rate = static_cast<_Float16>(0.0), \
            .range = static_cast<_Float16>(4.5), \
            .damage = static_cast<_Float16>(0.0), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(1.7), \
            .health = static_cast<_Float16>(30.0), \
            .max_health = static_cast<_Float16>(30.0), \
            .extra_scale = static_cast<_Float16>(0.3), \
            .capturing = (_faction) \
        };\
        for(int ppy=-2;ppy<=2;ppy++) \
            for(int ppx=-2;ppx<=2;ppx++) \
                if((ppy*ppy)+(ppx*ppx)<=4) terrainGrid[(int)(_y+0.5f)+ppy][(int)(_x+0.5f)+ppx] = terrainGrid[(int)(_y+0.5f)][(int)(_x+0.5f)]; \
    }


#define CREATE_CURIO(_faction, _x, _y) \
    if (num_units < MAX_UNITS) {\
        units[num_units++] = Unit{ \
            .texture = &tex::curio, \
            .faction = (_faction), \
            .name = "Curio", \
            .speed = 0.0, \
            .x = (float)(_x), \
            .y = (float)(_y), \
            .angle = 0.0, \
            .attack_rate = static_cast<_Float16>(0.0), \
            .range = static_cast<_Float16>(5), \
            .damage = static_cast<_Float16>(0.0), \
            .experience = static_cast<_Float16>(0.0), \
            .size = static_cast<_Float16>(2.5), \
            .health = static_cast<_Float16>(300.0), \
            .max_health = static_cast<_Float16>(300.0), \
            .extra_scale = static_cast<_Float16>(0.5), \
            .capturing = (_faction) \
        };\
        for(int ppy=-2;ppy<=2;ppy++) \
            for(int ppx=-2;ppx<=2;ppx++) \
                if((ppy*ppy)+(ppx*ppx)<=4) terrainGrid[(int)(_y+0.5f)+ppy][(int)(_x+0.5f)+ppx] = terrainGrid[(int)(_y+0.5f)][(int)(_x+0.5f)]; \
    }
