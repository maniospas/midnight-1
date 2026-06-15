#pragma once
#include <raylib.h>

Font uiFont;
Font smallerFont;
#define DrawText(txt, x, y, size, col) DrawTextEx(uiFont, txt, Vector2{ (float)(x), (float)(y) }, (float)(size), 2.0f, col)
#define DrawTextSmall(txt, x, y, size, col) DrawTextEx(smallerFont, txt, Vector2{ (float)(x), (float)(y) }, (float)(size), 1.0f, col)
#define DrawTextSmallOutlined(txt, x, y, size, col) {DrawTextSmall(txt, x-1, y+1, size, BLACK);DrawTextSmall(txt, x-1, y-1, size, BLACK);DrawTextSmall(txt, x+1, y+1, size, BLACK);DrawTextSmall(txt, x+1, y-1, size, BLACK);DrawTextSmall(txt, x, y, size, col);}
#define DrawTextOutlined(txt, x, y, size, col) {DrawText(txt, x-1, y+1, size, BLACK);DrawText(txt, x-1, y-1, size, BLACK);DrawText(txt, x+1, y+1, size, BLACK);DrawText(txt, x+1, y-1, size, BLACK);DrawText(txt, x, y, size, col);}

void load_fonts() {
    SetTextureFilter(uiFont.texture, TEXTURE_FILTER_BILINEAR);
    uiFont = LoadFontEx("data/Beholden-Regular.ttf", 96, nullptr, 0);
    SetTextureFilter(uiFont.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(smallerFont.texture, TEXTURE_FILTER_BILINEAR);
    smallerFont = LoadFontEx("data/Beholden-Regular.ttf", 32, nullptr, 0);
    SetTextureFilter(smallerFont.texture, TEXTURE_FILTER_BILINEAR);
}