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
if(hoveredTerrain->texture==&tex::water) {
    DrawText("Water", px + 80, textY + 80, 42, WHITE);
    float arrowAngle = water_angle + PI / 4.0f;
    Vector2 arrowCenter = { px + 300, textY + 80 + 21 }; // centered on the "Water" text
    float arrowLen = 28.0f;
    Vector2 arrowTip = {
        arrowCenter.x + cosf(arrowAngle) * arrowLen,
        arrowCenter.y + sinf(arrowAngle) * arrowLen
    };
    Vector2 arrowTail = {
        arrowCenter.x - cosf(arrowAngle) * arrowLen,
        arrowCenter.y - sinf(arrowAngle) * arrowLen
    };
    DrawLineEx(arrowTail, arrowTip, 3.0f, SKYBLUE);
    float headLen = 10.0f;
    float headAngle = 2.5f; // ~143 degrees spread
    Vector2 head1 = {
        arrowTip.x + cosf(arrowAngle + headAngle) * headLen,
        arrowTip.y + sinf(arrowAngle + headAngle) * headLen
    };
    Vector2 head2 = {
        arrowTip.x + cosf(arrowAngle - headAngle) * headLen,
        arrowTip.y + sinf(arrowAngle - headAngle) * headLen
    };
    DrawLineEx(arrowTip, head1, 3.0f, SKYBLUE);
    DrawLineEx(arrowTip, head2, 3.0f, SKYBLUE);
}
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