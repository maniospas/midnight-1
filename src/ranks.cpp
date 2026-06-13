#pragma once
#include <raylib.h>
#include "texture.cpp"

struct RankDef {
    const char* name;
    double min;
    double max;
    Texture2D* tex;
    Color color;
};

RankDef ranks[] = {
    {"STRUGGLER",  1200.0, 1400.0, &tex::gear, BROWN},
    {"SURVIVOR",   1400.0, 1600.0, &tex::grit, ORANGE},
    {"LEADER",     1600.0, 1800.0, &tex::heroics, YELLOW},
    {"UTOPIC",     1800.0, 2200.0, &tex::utopia, WHITE}
};