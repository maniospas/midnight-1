#pragma once
#include <raylib.h>
#include "texture.cpp"

static Color ColorForTile(Texture* texture) {
    if (texture == &tex::water) return (Color){ 40, 120, 160, 255 };
    if (texture == &tex::grass || texture == &tex::grass2 || texture == &tex::grass3 || texture == &tex::grass4) return (Color){ 60, 196, 40, 255 };
    if (texture == &tex::hill || texture == &tex::hill2 || texture == &tex::hill3 || texture == &tex::hill4) return (Color){ 140, 80, 40, 255 };
    if (texture == &tex::desert) return (Color){ 196, 196, 64, 255 };
    if (texture == &tex::mountain) return (Color){ 90, 52, 24, 255 };
    return (Color){ 80, 80, 80, 255 };
}

struct Terrain {
    Texture2D* texture;
    float speed;
    float extra_sight;
};


inline void DrawRot(Texture2D tex, int px, int py, float rot){
    Rectangle src = {0.0f,0.0f,(float)tex.width,(float)tex.height};
    Rectangle dst = {(float)px + tex.width  * 0.5f,(float)py + tex.height * 0.5f,(float)tex.width,(float)tex.height};
    Vector2 origin = {tex.width  * 0.5f, tex.height * 0.5f};
    DrawTexturePro(tex, src, dst, origin, rot, WHITE);
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