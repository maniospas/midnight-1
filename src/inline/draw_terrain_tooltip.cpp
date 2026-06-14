float panelSize = 320.0f;
Vector2 mouse = GetMousePosition();
float px = mouse.x - panelSize * 0.5f;
float py = mouse.y - panelSize * 0.8f - 64;
Rectangle src = { 0, 0, (float)tex::info.width, (float)tex::info.height };
Rectangle dst = { px, py, panelSize, panelSize };
Vector2 origin = { 0, 0 };
Color fc = factions[1].color;
DrawTexturePro(tex::info, src, dst, origin, 0.0f, fc);
float textY = py - 26;
px -= 55;
if(hoveredTerrain->texture==&tex::grass || hoveredTerrain->texture==&tex::grass2 || hoveredTerrain->texture==&tex::grass3 || hoveredTerrain->texture==&tex::grass4)
    DrawText("Grass", px + 80, textY + 80, 42, WHITE);
if(hoveredTerrain->texture==&tex::hill || hoveredTerrain->texture==&tex::hill2 || hoveredTerrain->texture==&tex::hill3 || hoveredTerrain->texture==&tex::hill4)
    DrawText("Hill", px + 80, textY + 80, 42, WHITE);
if(hoveredTerrain->texture==&tex::mountain) DrawText("Moutain", px + 80, textY + 80, 42, WHITE);
if(hoveredTerrain->texture==&tex::desert) DrawText("Desert", px + 80, textY + 80, 42, WHITE);
if(hoveredTerrain->texture==&tex::water) DrawText("Water", px + 80, textY + 80, 42, WHITE);
textY += 140;
DrawText("Right click to move", px + 80, textY, DESC_FONT_SIZE, WHITE);
if(hoveredTerrain->speed!=1.f)
    DrawText(TextFormat("Speed %d%%", (int)(hoveredTerrain->speed*100.f+0.5f)), px + 80, textY+DESC_FONT_SIZE+2, DESC_FONT_SIZE, WHITE);
if(hoveredTerrain->extra_sight) {
    if(hoveredTerrain->extra_sight<0) {
        DrawText(TextFormat("Sight %d%%", (int)(100.5f+hoveredTerrain->extra_sight*100)), px + 80, textY+DESC_FONT_SIZE*2+4, DESC_FONT_SIZE, WHITE);
        DrawText(TextFormat("Cover %d%%", (int)(-hoveredTerrain->extra_sight*100+0.5f)), px + 80, textY+DESC_FONT_SIZE*3+6, DESC_FONT_SIZE, WHITE);
    }
    else
        DrawText(TextFormat("Sight %d%%", (int)(100.5f+hoveredTerrain->extra_sight*100)), px + 80, textY+DESC_FONT_SIZE*2+4, DESC_FONT_SIZE, WHITE);
}