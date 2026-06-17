#pragma once
#include <raylib.h>
#include "texture.cpp"
#include "faction.cpp"

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
#define TECHNOLOGY_DRIVER     33554432ULL // mechas turn faster
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
#define TECHNOLOGY_MECHANISED   137438953472ULL // 1 industry per 10 mecha health
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
#define TECHNOLOGY_TURTLING  2251799813685248ULL // humans can lose 3 health to create rocks
#define TECHNOLOGY_TRENCHES  4503599627370496ULL // can turn tanks and vans to railguns
#define TECHNOLOGY_DISMANTLE 9007199254740992ULL // can dismantle tanks and vans to engines
#define TECHNOLOGY_CATS      18014398509481984ULL // can dismantle tanks and vans to engines
#define TECHNOLOGY_VROOM     36028797018963968ULL // double engine effect
#define TECHNOLOGY_REVUP     72057594037927936ULL // x1.5 speed and instruct cost by mechas
#define TECHNOLOGY_SCAVENGE 144115188075855872ULL // can dismantle tanks and vans to engines
#define TECHNOLOGY_COMMAND  288230376151711744ULL // fixed spawn speed
//#define TECHNOLOGY_INFORMATION  576460752303423488LL // see other faction industry and 

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
    "starting vets",
    "starting cats",
    "+8 start spawn",
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
    &tex::scout,
    &tex::cat,
    &tex::human,
    &tex::roomba,
    &tex::bison,
    &tex::esper,
    &tex::curio
};

unsigned long long global_available_starting_techs = TECHNOLOGY_SCAVENGE|TECHNOLOGY_COMMAND|TECHNOLOGY_NERDS|TECHNOLOGY_HUNTING|TECHNOLOGY_TAMING|TECHNOLOGY_HARDCORE|TECHNOLOGY_EXPLORE;
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


static void DrawConnector(float x1, float y1, float x2, float y2, bool active) {
    if(!active) return;
    Color c = active ? Fade(GREEN, 0.8f) : Fade(GRAY, 0.4f);
    DrawLineBezier({ x1, y1 }, { x2, y2 }, 3.0f, c);
}


void DrawTechs(Faction& F, bool showing_preview = false) {
    unsigned long long tech = F.technology;
    unsigned long long prev_tech = F.technology;

    //F.technology_progress += 1; // for debug

    const float actual_cell_width = GetScreenWidth()/6-60;
    const float actual_cell_height = (GetScreenHeight()/14-20)/2;
    // const float ICON_SIZE = actual_cell_height;
    // const float ICON_DX = actual_cell_width-ICON_SIZE-3.f;
    // const float ICON_DY = 3.f;
    const float ICON_SIZE = actual_cell_height*1.8f;
    const float ICON_DX = 12;
    const float ICON_DY = 3.f;

    float top = actual_cell_height;

    const float DX = GetScreenWidth()/6.0f;
    const float DY = GetScreenHeight()/15.0f;
    float cx = GetScreenWidth() * 0.5f-DX+20.f;

    // ROOTS
    Vector2 scavenge  = { cx - 2*DX, top };
    Vector2 explore   = { cx - 2*DX, top+2*DY };
    Vector2 hunting   = { cx - 2*DX, top+8*DY };
    Vector2 nerds     = { cx - 2*DX, top+11*DY };
    Vector2 taming    = { cx - 2*DX, top+4*DY };
    Vector2 command   = { cx - 2*DX, top+13*DY };
    Vector2 dismantle = { cx - DX, top+13*DY };

    // SECOND TIER
    Vector2 trenches  = { scavenge.x+DX, scavenge.y };
    Vector2 track     = { explore.x+DX, explore.y - DY };
    Vector2 agile     = { explore.x+DX, explore.y + DY};
    Vector2 driver    = { explore.x+DX, explore.y };
    Vector2 wonder    = { agile.x+DX, agile.y+DY};
    Vector2 fight     = { hunting.x+DX, hunting.y-2*DY};
    Vector2 farming   = { hunting.x+DX, hunting.y+DY};
    Vector2 research  = { nerds.x + DX, nerds.y-DY };
    Vector2 homunculi = { nerds.x + DX, nerds.y };
    Vector2 mecha     = { nerds.x + DX, nerds.y+DY };
    Vector2 vroom    =  { dismantle.x+DX, dismantle.y };

    // THIRD TIER
    Vector2 heroics    = { fight.x+DX,      fight.y-DY };
    Vector2 grit       = { fight.x,      fight.y+DY };
    Vector2 antimech   = { grit.x+3*DX,  grit.y};
    Vector2 flanking   = { antimech.x+DX,  antimech.y};
    Vector2 fort       = { fight.x,      fight.y+2*DY };
    Vector2 central    = { fort.x+DX,      fort.y };
    Vector2 tough      = { fight.x+DX,      fight.y };
    Vector2 infrastructure = { research.x + DX, research.y-DY };
    Vector2 aifarm     = { infrastructure.x + 2*DX, infrastructure.y };
    Vector2 technocracy= { aifarm.x + DX, aifarm.y };
    Vector2 unstable   = { homunculi.x + DX, homunculi.y };

    Vector2 seafaring  = { agile.x+DX,      agile.y };
    Vector2 hellbringer= { heroics.x+DX*2,  heroics.y+DY };
    Vector2 speedy     = { hellbringer.x+DX,hellbringer.y };

    Vector2 sniping    = { track.x+DX,      track.y-DY };
    Vector2 sniffing   = { track.x+DX,      track.y };
    Vector2 hardcore   = { hunting.x,       hunting.y-2*DY };
    Vector2 ownership  = { seafaring.x + DX,seafaring.y };
    Vector2 discourse  = { seafaring.x+DX,  seafaring.y-2*DY };
    Vector2 turtling   = { sniping.x+DX,    sniping.y };

    Vector2 luxury     = { ownership.x+DX,   ownership.y+DY };
    Vector2 refinery   = { ownership.x+DX,   ownership.y };
    Vector2 propaganda = { ownership.x+DX,   ownership.y+DY*2 };
    Vector2 superiority= { propaganda.x+DX,  propaganda.y };

    Vector2 autorepair  = { mecha.x + DX, mecha.y };
    Vector2 mobile      = { autorepair.x + DX, autorepair.y };
    Vector2 revup       = { mobile.x + DX, mobile.y };
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
    Vector2 cats        = { evolution.x + DX, evolution.y};
    Vector2 artificial  = { bioweapon.x + DX, bioweapon.y};
    Vector2 atmosphere  = { terraforming.x+DX, terraforming.y};
    if(showing_preview) tech = -1;

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
    DrawConnector(hunting.x+actual_cell_width, hunting.y+actual_cell_height, fort.x, fort.y+actual_cell_height, tech & TECHNOLOGY_HUNTING);
    DrawConnector(fort.x+actual_cell_width, fort.y+actual_cell_height, central.x, central.y+actual_cell_height, tech & TECHNOLOGY_FORT);
    DrawConnector(fight.x+actual_cell_width, fight.y+actual_cell_height, tough.x, tough.y+actual_cell_height, tech & TECHNOLOGY_FIGHT);
    DrawConnector(taming.x+actual_cell_width, taming.y+actual_cell_height, wonder.x, wonder.y+actual_cell_height, tech & TECHNOLOGY_TAMING);
    DrawConnector(wonder.x+actual_cell_width, wonder.y+actual_cell_height, luxury.x, luxury.y+actual_cell_height, tech & TECHNOLOGY_WONDER);
    DrawConnector(grit.x+actual_cell_width, grit.y+actual_cell_height, antimech.x, antimech.y+actual_cell_height, tech & TECHNOLOGY_GRIT);
    DrawConnector(tough.x+actual_cell_width, tough.y+actual_cell_height, antimech.x, antimech.y+actual_cell_height, tech & TECHNOLOGY_TOUGH);
    DrawConnector(antimech.x+actual_cell_width, antimech.y+actual_cell_height, flanking.x, flanking.y+actual_cell_height, tech & TECHNOLOGY_ANTIMECHA);
    DrawConnector(scavenge.x+actual_cell_width, scavenge.y+actual_cell_height, grit.x, grit.y+actual_cell_height, tech & TECHNOLOGY_SCAVENGE);
    
    // AGILE
    DrawConnector(agile.x+actual_cell_width, agile.y+actual_cell_height, seafaring.x, seafaring.y+actual_cell_height, tech & TECHNOLOGY_AGILE);
    DrawConnector(seafaring.x+actual_cell_width, seafaring.y+actual_cell_height, discourse.x, discourse.y+actual_cell_height, tech & TECHNOLOGY_SEAFARERING);
    DrawConnector(sniffing.x+actual_cell_width, sniffing.y+actual_cell_height, discourse.x, discourse.y+actual_cell_height, tech & TECHNOLOGY_SNIFFING);
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
    DrawConnector(evolution.x+actual_cell_width, evolution.y+actual_cell_height, cats.x, cats.y+actual_cell_height, tech & TECHNOLOGY_EVOLUTION);
    

    // MOBILE FORTRESS
    DrawConnector(nerds.x+actual_cell_width, nerds.y+actual_cell_height, dismantle.x, dismantle.y+actual_cell_height, tech & TECHNOLOGY_NERDS);
    DrawConnector(driver.x+actual_cell_width, driver.y+actual_cell_height, industry.x, industry.y+actual_cell_height, tech & TECHNOLOGY_DRIVER);
    DrawConnector(mecha.x+actual_cell_width, mecha.y+actual_cell_height, autorepair.x, autorepair.y+actual_cell_height, tech & TECHNOLOGY_MECHA);
    DrawConnector(autorepair.x+actual_cell_width, autorepair.y+actual_cell_height, mobile.x, mobile.y+actual_cell_height, tech & TECHNOLOGY_AUTOREPAIRS);
    DrawConnector(autorepair.x+actual_cell_width, autorepair.y+actual_cell_height, hijack.x, hijack.y+actual_cell_height, tech & TECHNOLOGY_AUTOREPAIRS);
    DrawConnector(industry.x+actual_cell_width, industry.y+actual_cell_height, gigajoule.x, gigajoule.y+actual_cell_height, tech & TECHNOLOGY_INDUSTRY);
    DrawConnector(heroics.x+actual_cell_width, heroics.y+actual_cell_height, propaganda.x, propaganda.y+actual_cell_height, tech & TECHNOLOGY_HEROICS);
    DrawConnector(propaganda.x+actual_cell_width, propaganda.y+actual_cell_height, superiority.x, superiority.y+actual_cell_height, tech & TECHNOLOGY_PROPAGANDA);
    DrawConnector(vroom.x+actual_cell_width, vroom.y+actual_cell_height, hijack.x, hijack.y+actual_cell_height, tech & TECHNOLOGY_VROOM);
    DrawConnector(hijack.x+actual_cell_width, hijack.y+actual_cell_height, antimech.x, antimech.y+actual_cell_height, tech & TECHNOLOGY_HIJACK);
    DrawConnector(dismantle.x+actual_cell_width, dismantle.y+actual_cell_height, vroom.x, vroom.y+actual_cell_height, tech & TECHNOLOGY_DISMANTLE);
    DrawConnector(mobile.x+actual_cell_width, mobile.y+actual_cell_height, revup.x, revup.y+actual_cell_height, tech & TECHNOLOGY_MOBILE_FORTRESS);
    
    // TRACK
    DrawConnector(track.x+actual_cell_width, track.y+actual_cell_height, sniping.x, sniping.y+actual_cell_height, tech & TECHNOLOGY_TRACK);
    DrawConnector(scavenge.x+actual_cell_width, scavenge.y+actual_cell_height, trenches.x, trenches.y+actual_cell_height, tech & TECHNOLOGY_SCAVENGE);
    DrawConnector(trenches.x+actual_cell_width, trenches.y+actual_cell_height, sniping.x, sniping.y+actual_cell_height, tech & TECHNOLOGY_TRENCHES);
    DrawConnector(track.x+actual_cell_width, track.y+actual_cell_height, sniffing.x, sniffing.y+actual_cell_height, tech & TECHNOLOGY_TRACK);
    DrawConnector(sniping.x+actual_cell_width, sniping.y+actual_cell_height, turtling.x, turtling.y+actual_cell_height, tech & TECHNOLOGY_SNIPING);
    DrawConnector(turtling.x+actual_cell_width, turtling.y+actual_cell_height, terraforming.x, terraforming.y+actual_cell_height, tech & TECHNOLOGY_TURTLING);

    // COMMAND
    DrawConnector(scavenge.x+actual_cell_width, scavenge.y+actual_cell_height, trenches.x, trenches.y+actual_cell_height, tech & TECHNOLOGY_SCAVENGE);
    DrawConnector(command.x+actual_cell_width, command.y+actual_cell_height, trenches.x, trenches.y+actual_cell_height, tech & TECHNOLOGY_COMMAND);
    DrawConnector(command.x+actual_cell_width, command.y+actual_cell_height, fort.x, fort.y+actual_cell_height, tech & TECHNOLOGY_COMMAND);
    DrawConnector(command.x+actual_cell_width, command.y+actual_cell_height, dismantle.x, dismantle.y+actual_cell_height, tech & TECHNOLOGY_COMMAND);
    

    if(showing_preview) {
        tech = F.technology;
        prev_tech = -1;
    }

    if(global_available_starting_techs&TECHNOLOGY_COMMAND) {
        DrawTechNode(command.x, command.y, "COMMAND", "Fixed speed of spawns", tech, TECHNOLOGY_COMMAND, !showing_preview);
        DrawTextureEx(tex::command, {command.x + ICON_DX, command.y + ICON_DY}, 0, ICON_SIZE / tex::command.width, WHITE);
    }

    if(global_available_starting_techs&TECHNOLOGY_SCAVENGE) {
        DrawTechNode(scavenge.x, scavenge.y, "SCAVENGE", "Fast unclaimed capture", tech, TECHNOLOGY_SCAVENGE, !showing_preview);
        DrawTextureEx(tex::warehouse, {scavenge.x + ICON_DX, scavenge.y + ICON_DY}, 0, ICON_SIZE / tex::warehouse.width, WHITE);
    }

    if(global_available_starting_techs&TECHNOLOGY_EXPLORE) {
        DrawTechNode(explore.x, explore.y, "CHARTED", "Wide camp & storage sight", tech, TECHNOLOGY_EXPLORE, !showing_preview);
        DrawTextureEx(tex::chart, {explore.x + ICON_DX, explore.y + ICON_DY}, 0, ICON_SIZE / tex::chart.width, WHITE);
    }

    if(global_available_starting_techs&TECHNOLOGY_HUNTING) {
        DrawTechNode(hunting.x, hunting.y, "CIVILIZED", "+3 camp & hide industry", tech, TECHNOLOGY_HUNTING, !showing_preview);
        DrawTextureEx(tex::hide, {hunting.x + ICON_DX, hunting.y + ICON_DY}, 0, ICON_SIZE / tex::hide.width, WHITE);
    }

    if(global_available_starting_techs&TECHNOLOGY_NERDS) {
        DrawTechNode(nerds.x,   nerds.y,   "NERDS",   "+30% research, -1 spawn HP", tech, TECHNOLOGY_NERDS, !showing_preview);
        DrawTextureEx(tex::nerds, {nerds.x + ICON_DX, nerds.y + ICON_DY}, 0, ICON_SIZE / tex::nerds.width, WHITE);
    }

    if(global_available_starting_techs&TECHNOLOGY_TAMING) {
        DrawTechNode(taming.x, taming.y, "TAMER", "-50% research, 50% taming", tech, TECHNOLOGY_TAMING, !showing_preview);
        DrawTextureEx(tex::bison, {taming.x + ICON_DX, taming.y + ICON_DY}, 0, ICON_SIZE / tex::bison.width, WHITE);
    }

    if(global_available_starting_techs&TECHNOLOGY_HARDCORE) {
        DrawTechNode(hardcore.x, hardcore.y, "HARDCORE", "-7 camp industry, 2x spawn", tech, TECHNOLOGY_HARDCORE, !showing_preview);
        DrawTextureEx(tex::blood, {hardcore.x + ICON_DX, hardcore.y + ICON_DY}, 0, ICON_SIZE / tex::blood.width, WHITE);
    }

    if(prev_tech & (TECHNOLOGY_EXPLORE | TECHNOLOGY_AGILE)) {
        DrawTechNode(agile.x, agile.y, "AGILE", "Terrain slows less", tech, TECHNOLOGY_AGILE, !showing_preview);
        DrawTextureEx(tex::agile, {agile.x + ICON_DX, agile.y + ICON_DY}, 0, ICON_SIZE / tex::agile.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_EXPLORE | TECHNOLOGY_DRIVER)) {
        DrawTechNode(driver.x, driver.y, "DRIVER", "Mechas turn faster", tech, TECHNOLOGY_DRIVER, !showing_preview);
        DrawTextureEx(tex::driver, {driver.x + ICON_DX, driver.y + ICON_DY}, 0, ICON_SIZE / tex::driver.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_EXPLORE | TECHNOLOGY_TRACK)) {
        DrawTechNode(track.x, track.y, "TRACKER", "+70% unit sight", tech, TECHNOLOGY_TRACK, !showing_preview);
        DrawTextureEx(tex::track, {track.x + ICON_DX, track.y + ICON_DY}, 0, ICON_SIZE / tex::track.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_SCAVENGE | TECHNOLOGY_COMMAND | TECHNOLOGY_TRENCHES)) {
        DrawTechNode(trenches.x, trenches.y, "ENTRENCH", "Vans & tanks to railguns", tech, TECHNOLOGY_TRENCHES, !showing_preview);
        DrawTextureEx(tex::railgun, {trenches.x + ICON_DX, trenches.y + ICON_DY}, 0, ICON_SIZE / tex::railgun.width, WHITE);
    }

    if(prev_tech & (TECHNOLOGY_NERDS | TECHNOLOGY_COMMAND | TECHNOLOGY_DISMANTLE)) {
        DrawTechNode(dismantle.x,   dismantle.y,   "DIMANTLERS",   "Vans & tanks to engines", tech, TECHNOLOGY_DISMANTLE, !showing_preview);
        DrawTextureEx(tex::engine, {dismantle.x + ICON_DX, dismantle.y + ICON_DY}, 0, ICON_SIZE / tex::engine.width, WHITE);
    }

    if(prev_tech & (TECHNOLOGY_MOBILE_FORTRESS | TECHNOLOGY_REVUP)) {
        DrawTechNode(revup.x, revup.y, "REVUP", "x1.5 mecha speed & industry", tech, TECHNOLOGY_REVUP, !showing_preview);
        DrawTextureEx(tex::revup, {revup.x + ICON_DX, revup.y + ICON_DY}, 0, ICON_SIZE / tex::revup.width, WHITE);
    }

    if(prev_tech & (TECHNOLOGY_DISMANTLE | TECHNOLOGY_VROOM)) {
        DrawTechNode(vroom.x, vroom.y, "VROOOM", "Double engine effects", tech, TECHNOLOGY_VROOM, !showing_preview);
        DrawTextureEx(tex::vroom, {vroom.x + ICON_DX, vroom.y + ICON_DY}, 0, ICON_SIZE / tex::vroom.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_NERDS | TECHNOLOGY_MECHA)) {
        DrawTechNode(mecha.x, mecha.y, "HULL", "+50% mecha dodge", tech, TECHNOLOGY_MECHA, !showing_preview);
        DrawTextureEx(tex::shield, {mecha.x + ICON_DX, mecha.y + ICON_DY}, 0, ICON_SIZE / tex::shield.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_NERDS | TECHNOLOGY_HOMUNCULI)) {
        DrawTechNode(homunculi.x, homunculi.y, "HOMUNCULI", "50% to control killed bloo", tech, TECHNOLOGY_HOMUNCULI, !showing_preview);
        DrawTextureEx(tex::ghost, {homunculi.x + ICON_DX, homunculi.y + ICON_DY}, 0, ICON_SIZE / tex::ghost.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_NERDS | TECHNOLOGY_RESEARCH)) {
        DrawTechNode(research.x, research.y, "RESEARCH", "+30% research", tech, TECHNOLOGY_RESEARCH, !showing_preview);
        DrawTextureEx(tex::research, {research.x + ICON_DX, research.y + ICON_DY}, 0, ICON_SIZE / tex::research.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_HUNTING | TECHNOLOGY_FARMING)) {
        DrawTechNode(farming.x, farming.y, "FARMING", "Fields stay long in bloom", tech, TECHNOLOGY_FARMING, !showing_preview);
        DrawTextureEx(tex::field, {farming.x + ICON_DX, farming.y + ICON_DY}, 0, ICON_SIZE / tex::field.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_HUNTING | TECHNOLOGY_COMMAND | TECHNOLOGY_FORT)) {
        DrawTechNode(fort.x, fort.y, "FORT", "Humans to forts", tech, TECHNOLOGY_FORT, !showing_preview);
        DrawTextureEx(tex::fort, {fort.x + ICON_DX, fort.y + ICON_DY}, 0, ICON_SIZE / tex::fort.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_HUNTING | TECHNOLOGY_SCAVENGE | TECHNOLOGY_GRIT)) {
        DrawTechNode(grit.x, grit.y, "GRIT", "+50% dodge vs lethal", tech, TECHNOLOGY_GRIT, !showing_preview);
        DrawTextureEx(tex::grit, {grit.x + ICON_DX, grit.y + ICON_DY}, 0, ICON_SIZE / tex::grit.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_HARDCORE | TECHNOLOGY_FIGHT)) {
        DrawTechNode(fight.x, fight.y, "FIGHT", "Double unit experience", tech, TECHNOLOGY_FIGHT, !showing_preview);
        DrawTextureEx(tex::fight, {fight.x + ICON_DX, fight.y + ICON_DY}, 0, ICON_SIZE / tex::fight.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_MECHA | TECHNOLOGY_AUTOREPAIRS)) {
        DrawTechNode(autorepair.x, autorepair.y, "AUTOREPAIR", "Mecha regen HP", tech, TECHNOLOGY_AUTOREPAIRS, !showing_preview);
        DrawTextureEx(tex::autorepair, {autorepair.x + ICON_DX, autorepair.y + ICON_DY}, 0, ICON_SIZE / tex::autorepair.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_FIGHT | TECHNOLOGY_TOUGH)) {
        DrawTechNode(tough.x, tough.y, "TOUGH", "+1 spawn HP", tech, TECHNOLOGY_TOUGH, !showing_preview);
        DrawTextureEx(tex::tough, {tough.x + ICON_DX, tough.y + ICON_DY}, 0, ICON_SIZE / tex::tough.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_FIGHT | TECHNOLOGY_TAMING | TECHNOLOGY_HEROICS)) {
        DrawTechNode(heroics.x, heroics.y, "HEROICS", "+30% vet & hero dodge", tech, TECHNOLOGY_HEROICS, !showing_preview);
        DrawTextureEx(tex::heroics, {heroics.x + ICON_DX, heroics.y + ICON_DY}, 0, ICON_SIZE / tex::heroics.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_AGILE | TECHNOLOGY_TAMING | TECHNOLOGY_WONDER)) {
        DrawTechNode(wonder.x, wonder.y, "WONDER", "Discoveries grant research", tech, TECHNOLOGY_WONDER, !showing_preview);
        DrawTextureEx(tex::wonder, {wonder.x + ICON_DX, wonder.y + ICON_DY}, 0, ICON_SIZE / tex::wonder.width, WHITE);
    }

    if(prev_tech & (TECHNOLOGY_BIOWEAPON | TECHNOLOGY_ARTIFICIAL)) {
        DrawTechNode(artificial.x, artificial.y, "HIVEMIND", "Fast bloo experience", tech, TECHNOLOGY_ARTIFICIAL, !showing_preview);
        DrawTextureEx(tex::mind, {artificial.x + ICON_DX, artificial.y + ICON_DY}, 0, ICON_SIZE / tex::mind.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_REACTOR | TECHNOLOGY_HYPERMAGNET)) {
        DrawTechNode(hypermagnet.x, hypermagnet.y, "HYPERMAGNET", "x2 industry cost and speed", tech, TECHNOLOGY_HYPERMAGNET, !showing_preview);
        DrawTextureEx(tex::magnet, {hypermagnet.x + ICON_DX, hypermagnet.y + ICON_DY}, 0, ICON_SIZE / tex::magnet.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_INFRASTRUCTURE | TECHNOLOGY_AIFARM)) {
        DrawTechNode(aifarm.x, aifarm.y, "AI FARM", "+16 lab & big bro industry", tech, TECHNOLOGY_AIFARM, !showing_preview);
        DrawTextureEx(tex::lab, {aifarm.x + ICON_DX, aifarm.y + ICON_DY}, 0, ICON_SIZE / tex::lab.width, WHITE);
    }
    // account for datacenters autonomously adding techs without predecesoors
    if(prev_tech & (TECHNOLOGY_BIOWEAPON | TECHNOLOGY_GRIT | TECHNOLOGY_EVOLUTION)) {
        DrawTechNode(evolution.x, evolution.y, "EVOLUTION", "Some spawn are snowmen", tech, TECHNOLOGY_EVOLUTION, !showing_preview);
        DrawTextureEx(tex::snowman, {evolution.x + ICON_DX, evolution.y + ICON_DY}, 0, ICON_SIZE / tex::snowman.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_ANTIMECHA | TECHNOLOGY_GRIT | TECHNOLOGY_HIJACK | TECHNOLOGY_TOUGH)) {
        DrawTechNode(antimech.x, antimech.y, "SABOTAGE", "x2 damage vs fort & mecha", tech, TECHNOLOGY_ANTIMECHA, !showing_preview);
        DrawTextureEx(tex::human, {antimech.x + ICON_DX, antimech.y + ICON_DY}, 0, ICON_SIZE / tex::human.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_HEROICS | TECHNOLOGY_HELLBRINGER)) {
        DrawTechNode(hellbringer.x, hellbringer.y, "HELLBRINGER", "Rapid vet & hero fire", tech, TECHNOLOGY_HELLBRINGER, !showing_preview);
        DrawTextureEx(tex::hero, {hellbringer.x + ICON_DX, hellbringer.y + ICON_DY}, 0, ICON_SIZE / tex::hero.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_AGILE | TECHNOLOGY_SEAFARERING)) {
        DrawTechNode(seafaring.x, seafaring.y, "SEAFARING", "Water increases speed", tech, TECHNOLOGY_SEAFARERING, !showing_preview);
        DrawTextureEx(tex::seafaring, {seafaring.x + ICON_DX, seafaring.y + ICON_DY}, 0, ICON_SIZE / tex::seafaring.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_SEAFARERING | TECHNOLOGY_INFRASTRUCTURE | TECHNOLOGY_OWNERSHIP)) {
        DrawTechNode(ownership.x, ownership.y, "OWNERSHIP", "Better hold captured assets", tech, TECHNOLOGY_OWNERSHIP, !showing_preview);
        DrawTextureEx(tex::ownership, {ownership.x + ICON_DX, ownership.y + ICON_DY}, 0, ICON_SIZE / tex::ownership.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_HOMUNCULI | TECHNOLOGY_UNSTABLE)) {
        DrawTechNode(unstable.x, unstable.y, "UNSTABLE", "-1 spawn HP, bloo on death", tech, TECHNOLOGY_UNSTABLE, !showing_preview);
        DrawTextureEx(tex::unstable, {unstable.x + ICON_DX, unstable.y + ICON_DY}, 0, ICON_SIZE / tex::unstable.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_AIFARM | TECHNOLOGY_MECHANISED | TECHNOLOGY_TECHNOCRACY)) {
        DrawTechNode(technocracy.x, technocracy.y, "TECHNOCRACY", "+1 utopia per 100 industry", tech, TECHNOLOGY_TECHNOCRACY, !showing_preview);
        DrawTextureEx(tex::utopia, {technocracy.x + ICON_DX, technocracy.y + ICON_DY}, 0, ICON_SIZE / tex::utopia.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_EVOLUTION | TECHNOLOGY_CATS)) {
        DrawTechNode(cats.x, cats.y, "APEX", "Spawned humans are cats", tech, TECHNOLOGY_CATS, !showing_preview);
        DrawTextureEx(tex::cat, {cats.x + ICON_DX, cats.y + ICON_DY}, 0, ICON_SIZE / tex::cat.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_ANTIMECHA | TECHNOLOGY_FLANKING)) {
        DrawTechNode(flanking.x, flanking.y, "FLANKING", "x2 damage from behind", tech, TECHNOLOGY_FLANKING, !showing_preview);
        DrawTextureEx(tex::flank, {flanking.x + ICON_DX, flanking.y + ICON_DY}, 0, ICON_SIZE / tex::flank.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_HELLBRINGER | TECHNOLOGY_SPEEDY)) {
        DrawTechNode(speedy.x, speedy.y, "360 ANGLE", "Rotate faster", tech, TECHNOLOGY_SPEEDY, !showing_preview);
        DrawTextureEx(tex::rotate, {speedy.x + ICON_DX, speedy.y + ICON_DY}, 0, ICON_SIZE / tex::rotate.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_PROPAGANDA | TECHNOLOGY_SUPERIORITY)) {
        DrawTechNode(superiority.x, superiority.y, "SUPERIORITY", "+1 utopia", tech, TECHNOLOGY_SUPERIORITY, !showing_preview);
        DrawTextureEx(tex::superiority, {superiority.x + ICON_DX, superiority.y + ICON_DY}, 0, ICON_SIZE / tex::superiority.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_DRIVER | TECHNOLOGY_INDUSTRY)) {
        DrawTechNode(industry.x, industry.y, "LOST .INC", "Mechas gain experience", tech, TECHNOLOGY_INDUSTRY, !showing_preview);
        DrawTextureEx(tex::gear, {industry.x + ICON_DX, industry.y + ICON_DY}, 0, ICON_SIZE / tex::gear.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_AUTOREPAIRS | TECHNOLOGY_VROOM | TECHNOLOGY_HIJACK)) {
        DrawTechNode(hijack.x, hijack.y, "HIJACK", "50% capture mechas", tech, TECHNOLOGY_HIJACK, !showing_preview);
        DrawTextureEx(tex::hijack, {hijack.x + ICON_DX, hijack.y + ICON_DY}, 0, ICON_SIZE / tex::hijack.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_AUTOREPAIRS | TECHNOLOGY_MOBILE_FORTRESS)) {
        DrawTechNode(mobile.x, mobile.y, "MOBILE FORT", "Railguns may become tanks", tech, TECHNOLOGY_MOBILE_FORTRESS, !showing_preview);
        DrawTextureEx(tex::tank, {mobile.x + ICON_DX, mobile.y + ICON_DY}, 0, ICON_SIZE / tex::tank.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_UNSTABLE | TECHNOLOGY_BIOWEAPON)) {
        DrawTechNode(bioweapon.x, bioweapon.y, "BIOWEAPON", "Kills become bloo", tech, TECHNOLOGY_BIOWEAPON, !showing_preview);
        DrawTextureEx(tex::bioweapon, {bioweapon.x + ICON_DX, bioweapon.y + ICON_DY}, 0, ICON_SIZE / tex::bioweapon.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_TRACK | TECHNOLOGY_SNIFFING)) {
        DrawTechNode(sniffing.x, sniffing.y, "FREE THINKING", "Unit autonomy, +1 utopia", tech, TECHNOLOGY_SNIFFING, !showing_preview);
        DrawTextureEx(tex::human, {sniffing.x + ICON_DX, sniffing.y + ICON_DY}, 0, ICON_SIZE / tex::human.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_TRACK | TECHNOLOGY_TRENCHES | TECHNOLOGY_SNIPING)) {
        DrawTechNode(sniping.x, sniping.y, "SNIPING", "x2 accuracy vs dodge", tech, TECHNOLOGY_SNIPING, !showing_preview);
        DrawTextureEx(tex::snipe, {sniping.x + ICON_DX, sniping.y + ICON_DY}, 0, ICON_SIZE / tex::snipe.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_RESEARCH | TECHNOLOGY_NUCLEAR)) {
        DrawTechNode(nuclear.x, nuclear.y, "NUCLEAR", "x2 damage for no regen", tech, TECHNOLOGY_NUCLEAR, !showing_preview);
        DrawTextureEx(tex::nuclear, {nuclear.x + ICON_DX, nuclear.y + ICON_DY}, 0, ICON_SIZE / tex::nuclear.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_FARMING | TECHNOLOGY_RESEARCH | TECHNOLOGY_INFRASTRUCTURE)) {
        DrawTechNode(infrastructure.x, infrastructure.y, "MEDIA", "x2 radio & fort sight", tech, TECHNOLOGY_INFRASTRUCTURE, !showing_preview);
        DrawTextureEx(tex::radio, {infrastructure.x + ICON_DX, infrastructure.y + ICON_DY}, 0, ICON_SIZE / tex::radio.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_FORT | TECHNOLOGY_CENTRAL)) {
        DrawTechNode(central.x, central.y, "CENTRAL", "Fort & curio spread fields", tech, TECHNOLOGY_CENTRAL, !showing_preview);
        DrawTextureEx(tex::central, {central.x + ICON_DX, central.y + ICON_DY}, 0, ICON_SIZE / tex::central.width, WHITE);
    }
    if(prev_tech & TECHNOLOGY_TERRAFORIMING) {
        DrawTechNode(atmosphere.x, atmosphere.y, "AIR v2.0", "Fields delay polution", tech, TECHNOLOGY_ATMOSPHERE, !showing_preview);
        DrawTextureEx(tex::earth, {atmosphere.x + ICON_DX, atmosphere.y + ICON_DY + actual_cell_height/4}, 0, ICON_SIZE / tex::earth.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_OWNERSHIP | TECHNOLOGY_HEROICS | TECHNOLOGY_PROPAGANDA)) {
        DrawTechNode(propaganda.x, propaganda.y, "PROPAGANDA", "+1 utopia per 2 radios", tech, TECHNOLOGY_PROPAGANDA, !showing_preview);
        DrawTextureEx(tex::propaganda, {propaganda.x + ICON_DX, propaganda.y + ICON_DY}, 0, ICON_SIZE / tex::propaganda.width, WHITE);
    }

    if(prev_tech & (TECHNOLOGY_OWNERSHIP | TECHNOLOGY_WONDER | TECHNOLOGY_LUXURY)) {
        DrawTechNode(luxury.x, luxury.y, "PRISTINE", "+50% dodge vs animal & bloo", tech, TECHNOLOGY_LUXURY, !showing_preview);
        DrawTextureEx(tex::pristine, {luxury.x + ICON_DX, luxury.y + ICON_DY}, 0, ICON_SIZE / tex::pristine.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_OWNERSHIP | TECHNOLOGY_GIGAJOULE | TECHNOLOGY_REFINERY)) {
        DrawTechNode(refinery.x, refinery.y, "REFINERY", "+25 industry from oil", tech, TECHNOLOGY_REFINERY, !showing_preview);
        DrawTextureEx(tex::oil, {refinery.x + ICON_DX, refinery.y + ICON_DY}, 0, ICON_SIZE / tex::oil.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_GIGAJOULE | TECHNOLOGY_MECHANISED)) {
        DrawTechNode(mechanised.x, mechanised.y, "MECHANIZED", "+1 industry per 10 mecha HP", tech, TECHNOLOGY_MECHANISED, !showing_preview);
        DrawTextureEx(tex::gear, {mechanised.x + ICON_DX, mechanised.y + ICON_DY}, 0, ICON_SIZE / tex::gear.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_GIGAJOULE | TECHNOLOGY_TURTLING | TECHNOLOGY_TERRAFORIMING)) {
        DrawTechNode(terraforming.x, terraforming.y, "TERRAFORM", "Double field expansion", tech, TECHNOLOGY_TERRAFORIMING, !showing_preview);
        DrawTextureEx(tex::field, {terraforming.x + ICON_DX, terraforming.y + ICON_DY}, 0, ICON_SIZE / tex::field.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_INDUSTRY | TECHNOLOGY_GIGAJOULE)) {
        DrawTechNode(gigajoule.x, gigajoule.y, "GIGAJOULE", "No mecha industry cost", tech, TECHNOLOGY_GIGAJOULE, !showing_preview);
        DrawTextureEx(tex::gigajoule, {gigajoule.x + ICON_DX, gigajoule.y + ICON_DY}, 0, ICON_SIZE / tex::gigajoule.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_SEAFARERING | TECHNOLOGY_SNIFFING | TECHNOLOGY_DISCOURSE)) {
        DrawTechNode(discourse.x, discourse.y, "DISCOURSE", "1 utopia per 10 big bro industry", tech, TECHNOLOGY_DISCOURSE, !showing_preview);
        DrawTextureEx(tex::discourse, {discourse.x + ICON_DX, discourse.y + ICON_DY}, 0, ICON_SIZE / tex::discourse.width, WHITE);
    }

    if(prev_tech & (TECHNOLOGY_SNIPING | TECHNOLOGY_TURTLING)) {
        DrawTechNode(turtling.x, turtling.y, "TURTLING", "-3 max health to stack rocks", tech, TECHNOLOGY_TURTLING, !showing_preview);
        DrawTextureEx(tex::rock, {turtling.x + ICON_DX, turtling.y + ICON_DY}, 0, ICON_SIZE / tex::rock.width, WHITE);
    }
    if(prev_tech & (TECHNOLOGY_NUCLEAR | TECHNOLOGY_REACTOR)) {
        DrawTechNode(reactor.x, reactor.y, "REACTOR", "+40 industry", tech, TECHNOLOGY_REACTOR, !showing_preview);
        DrawTextureEx(tex::nuclear, {reactor.x + ICON_DX, reactor.y + ICON_DY}, 0, ICON_SIZE / tex::nuclear.width, WHITE);
    }

    if(tech!=F.technology && F.technology_progress>=1.f && !showing_preview) {
        F.technology_progress -= 1.f;
        F.technology = tech;
        PlaySound(sound::select);
    }
}