#pragma once
#include <raylib.h>
#include <bitset>
static const int MAX_UNITS = 80000; // up to 200kb worth of units

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