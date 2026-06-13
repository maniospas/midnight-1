#pragma once
#include <raylib.h>
#include "texture.cpp"


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
#define TECHNOLOGY_FORT        70368744177664ULL // allows fortbuilding
#define TECHNOLOGY_ANTIMECHA  140737488355328ULL // extra damage vs mecha and forts
#define TECHNOLOGY_FLANKING   281474976710656ULL // extra flanking damage
#define TECHNOLOGY_DISCOURSE  562949953421312ULL // +1 utopia per 10 big bro industry
#define TECHNOLOGY_CENTRAL   1125899906842624ULL // fields grow around forts and radios

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
#define PREFERENCE_CURIO      12
#define PREFERENCE_COUNT      13

const char* preference_desc[PREFERENCE_COUNT] = {
    "near railguns",
    "near vehicle",
    "near fields",
    "near old tech",
    "near storage",
    "+camp spacing",
    "+starting vets",
    "+starting speed",
    "+8 humans",
    "near roombas",
    "+tech, near animals",
    "near esper",
    "near curio",
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
    &tex::esper,
    &tex::curio
};

typedef unsigned long long PrefMask;
#define PREF_BIT(p) (1ULL << (p))

static void DrawTechProgressBar(float x, float y, float w, float h, float progress) {
    if (progress < 0.f) progress = 0.f;
    if (progress > 1.f) progress = 1.f;
    DrawRectangleRounded({x, y, w, h}, 0.3f, 8, Fade(DARKGRAY, 0.6f));
    DrawRectangleRounded({x + 2, y + 2, (w - 4) * progress, h - 4},0.3f, 8,Fade(GREEN, 0.85f));
    DrawRectangleRoundedLines({x, y, w, h}, 0.3f, 8, GRAY);
}


static bool DrawTechNode(
    float x, float y,
    const char* title,
    const char* desc,
    unsigned long long &tech,
    unsigned long long bit,
    bool enabled
) {
    const int W = GetScreenWidth()/6-60;
    const int H = GetScreenHeight()/14-20;
    Rectangle rect = { x, y, (float)W, (float)H };
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, rect);
    bool owned   = (tech & bit) != 0;
    Color bg = owned   ? Fade(GREEN, 0.15f) : hovered ? Fade(DARKGRAY, 0.75f) : Fade(GRAY, 0.55f);
    Color edge = owned ? GREEN : hovered ? WHITE :GRAY;
    const float title_height = W/10;
    const float desc_height = (H-W/10)/2.2f;
    if (hovered) {
        rect = { x, y, (float)W, (float)(H*1.4) };
        bg =  owned   ? ColorBrightness(GREEN, -0.65f) : hovered ? ColorBrightness(DARKGRAY, -0.25f) : ColorBrightness(GRAY, -0.45f);
    }
    DrawRectangleRounded(rect, 0.2f, title_height, bg);
    DrawRectangleRoundedLines(rect, 0.2f, title_height, edge);

    DrawText(title, x + W/4+20, y + H/2-W/20, title_height, WHITE);
    //DrawTextSmall(desc,  x + 12, y + W/7, (H-W/10)/1.9f, Fade(WHITE, 0.85f));

    if (hovered) {
        DrawText(
            desc,
            x + 12,
            y + H,
            desc_height,
            Fade(WHITE, 0.85f)
        );
    }
    if (hovered && enabled && !owned && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        tech |= bit;
        return true;
    }
    return false;
}
