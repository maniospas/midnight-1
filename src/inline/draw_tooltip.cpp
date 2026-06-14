float panelSize = 320.0f;
Vector2 mouse = GetMousePosition();
float px = mouse.x - panelSize * 0.5f;
float py = mouse.y - panelSize * 0.8f - 64;
Rectangle src = { 0, 0, (float)tex::info.width, (float)tex::info.height };
Rectangle dst = { px, py, panelSize, panelSize };
Vector2 origin = { 0, 0 };
Color fc = hovered->faction ? hovered->faction->color : BLACK;
Color inv = { (unsigned char)(255 - fc.r), (unsigned char)(255 - fc.g), (unsigned char)(255 - fc.b), fc.a };
DrawTexturePro(tex::info, src, dst, origin, 0.0f, fc);
auto WHITECOL = Fade(WHITE,1.f);
float textY = py - 26;
px -= 55;
if (hovered && hovered->health) {
    DrawText(hovered->name, px + 80, textY + 80, 42, WHITECOL);
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
        DrawTexturePro(*t, src, dst, origin, rot, WHITECOL);
    }
    textY += 140;
    if(hovered->texture==&tex::camp) {
        DrawText("Spawns humans if", px + 80, textY, DESC_FONT_SIZE, WHITECOL);
        DrawText("below industry cap", px + 80, textY+DESC_FONT_SIZE+2, DESC_FONT_SIZE, WHITECOL);
        //DrawTextSmall("capturable", px + 255, textY+125, DESC_FONT_SIZE, inv);
    }
    else if(hovered->texture==&tex::lab) {
        DrawText("+10% research", px + 80, textY, DESC_FONT_SIZE, WHITECOL);
        //DrawTextSmall("capturable", px + 255, textY+125, DESC_FONT_SIZE, inv);
    }
    else if(hovered->texture==&tex::field) {
        DrawText("+4 industry (bloom)", px + 80, textY, DESC_FONT_SIZE, WHITECOL);
        DrawText("Eratic crop cycle", px + 80, textY+DESC_FONT_SIZE+2, DESC_FONT_SIZE, WHITECOL);
        DrawText("Spreads if in bloom", px + 80, textY+(DESC_FONT_SIZE+2)*2, DESC_FONT_SIZE, WHITECOL);
        //DrawTextSmall("capturable", px + 255, textY+125, 22, inv);
    }
    else if(hovered->texture==&tex::field_little) {
        DrawText("+2 industry (grows)", px + 80, textY, DESC_FONT_SIZE, WHITECOL);
        DrawText("Eratic crop cycle", px + 80, textY+DESC_FONT_SIZE+2, DESC_FONT_SIZE, WHITECOL);
        DrawText("Spreads if in bloom", px + 80, textY+(DESC_FONT_SIZE+2)*2, DESC_FONT_SIZE, WHITECOL);
        //DrawTextSmall("capturable", px + 255, textY+125, 22, inv);
    }
    else if(hovered->texture==&tex::field_empty) {
        DrawText("+0 industry (barren)", px + 80, textY, DESC_FONT_SIZE, WHITECOL);
        DrawText("Eratic crop cycle", px + 80, textY+DESC_FONT_SIZE+2, DESC_FONT_SIZE, WHITECOL);
        DrawText("Spreads if in bloom", px + 80, textY+(DESC_FONT_SIZE+2)*2, DESC_FONT_SIZE, WHITECOL);
        //DrawTextSmall("capturable", px + 255, textY+125, 22, inv);
    }
    else if(hovered->texture==&tex::hide) {
        DrawText("+4 industry", px + 80, textY, DESC_FONT_SIZE, WHITECOL);
        DrawText("May become rats", px + 80, textY+DESC_FONT_SIZE+2, DESC_FONT_SIZE, WHITECOL);
        //DrawTextSmall("capturable", px + 255, textY+125, 22, inv);
    }
    else if(hovered->texture==&tex::mine) {
        DrawText("+12 industry", px + 80, textY, DESC_FONT_SIZE, WHITECOL);
        //DrawTextSmall("capturable", px + 255, textY+125, 22, inv);
    }
    else if(hovered->texture==&tex::oil) {
        DrawText("+3 utopia", px + 80, textY, DESC_FONT_SIZE, WHITECOL);
        //DrawTextSmall("capturable", px + 255, textY+125, 22, inv);
    }
    else if(hovered->texture==&tex::datacenter) {
        DrawText("Random benefits", px + 80, textY, DESC_FONT_SIZE, WHITECOL);
        //DrawTextSmall("capturable", px + 255, textY+125, 22, inv);
    }
    else if(hovered->texture==&tex::warehouse) {
        DrawText("+2 utopia", px + 80, textY, DESC_FONT_SIZE, WHITECOL);
        //DrawTextSmall("capturable", px + 255, textY+125, 22, inv);
    }
    else if(hovered->texture==&tex::lighthouse) {
        DrawText("+half utopia", px + 80, textY, DESC_FONT_SIZE, WHITECOL);
        DrawText("+5 industry", px + 80, textY+DESC_FONT_SIZE+2, DESC_FONT_SIZE, WHITECOL);
        //DrawTextSmall("capturable", px + 255, textY+125, 22, inv);
    }
    else if(hovered->texture==&tex::fort) {
        DrawText(TextFormat("%d industry cost", (int)(hovered->max_health/5+0.5)), px + 80, textY, DESC_FONT_SIZE, WHITECOL);
        DrawUnitStatCircle(hovered, px + 120, textY + 40);
        //DrawTextSmall("capturable", px + 255, textY+125, 22, inv);
    }
    else if(hovered->texture==&tex::railgun) {
        DrawText("Mecha", px + 80, textY, DESC_FONT_SIZE, WHITECOL);
        DrawUnitStatCircle(hovered, px + 120, textY + 40);
        //if(!hovered->faction) DrawTextSmall("capturable", px + 255, textY+125, 22, inv);
    }
    else if(hovered->texture==&tex::roomba) {
        DrawText("Mecha, only attacks", px + 80, textY, DESC_FONT_SIZE, WHITECOL);
        DrawText("animal & bloo", px + 80, textY+DESC_FONT_SIZE+2, DESC_FONT_SIZE, WHITECOL);
        DrawUnitStatCircle(hovered, px + 120, textY + 40);
    }
    else if(hovered->texture==&tex::curio) {
        DrawText("+1 utopia", px + 80, textY, DESC_FONT_SIZE, WHITECOL);
        DrawText("Produces wild rats", px + 80, textY+DESC_FONT_SIZE+2, DESC_FONT_SIZE, WHITECOL);
        //DrawUnitStatCircle(hovered, px + 120, textY + 40);
    }
    else if(hovered->texture==&tex::rock) {
        DrawText("Obstacle", px + 80, textY, DESC_FONT_SIZE, WHITECOL);
        DrawUnitStatCircle(hovered, px + 120, textY + 40);
    }
    else if(hovered->texture==&tex::esper) {
        DrawText("+2 utopia, unruly", px + 80, textY, DESC_FONT_SIZE, WHITECOL);
        DrawUnitStatCircle(hovered, px + 120, textY + 40);
        //DrawTextSmall("capturable", px + 255, textY+125, 22, inv);
    }
    else if(is_mecha((*hovered)) && hovered->speed==0) {
        DrawText("Mecha", px + 80, textY, DESC_FONT_SIZE, WHITECOL);
        DrawUnitStatCircle(hovered, px + 120, textY + 40);
        if(!hovered->faction) DrawTextSmall("capturable", px + 255, textY+125, 22, inv);
    }
    else if(is_mecha((*hovered))) {
        DrawText(TextFormat("Mecha, %d industry cost", (int)(hovered->max_health/5+0.5)), px + 80, textY, DESC_FONT_SIZE, WHITECOL);
        DrawUnitStatCircle(hovered, px + 120, textY + 40);
        //if(!hovered->faction) DrawTextSmall("capturable", px + 255, textY+125, 22, inv);
    }
    else if(hovered->damage) {
        if(hovered->range<3.f) {
            if(hovered->texture==&tex::snowman) DrawText("Animal, fast on hills", px + 80, textY, DESC_FONT_SIZE, WHITECOL);
            else if(hovered->texture==&tex::rat) DrawText("Animal, proliferates", px + 80, textY, DESC_FONT_SIZE, WHITECOL);
            else if(hovered->texture==&tex::wolf) DrawText("Animal, 50\% taming", px + 80, textY, DESC_FONT_SIZE, WHITECOL);
            else DrawText("Animal, drops hide", px + 80, textY, DESC_FONT_SIZE, WHITECOL);
            DrawUnitStatCircle(hovered, px + 120, textY + 40);
        }
        else {
            DrawText(TextFormat("%d industry cost", (int)(hovered->max_health/5+0.5)), px + 80, textY, DESC_FONT_SIZE, WHITECOL);
            DrawUnitStatCircle(hovered, px + 120, textY + 40);
        }
    }
    else {
        DrawText("Far sight", px + 80, textY, DESC_FONT_SIZE, WHITECOL);
        //DrawTextSmall("capturable", px + 255, textY+125, 22, inv);
    }
}
else {
    DrawText("No info", px + 80, textY + 80, 42, WHITECOL);
    DrawText("Mouse over a unit.", px + 80, textY + 150, DESC_FONT_SIZE, WHITECOL);
}