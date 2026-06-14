
// ============================================================================
// AI FOR FACTIONS >= 3
// ============================================================================
float AI_ORDER_CHANCE = player_rating/1200.f*0.5f*0.1f; // 0.1f is competent but not unbeatable
float AI_ORDER_RADIUS = 10.0f;

for (int i = 0; i < num_units; i++) {
    Unit &u = units[i];
    if (u.health <= 0) continue;
    if (u.speed <= 0) continue;
    if (u.texture==&tex::esper) continue;
    //if (u.target_x != 0 || u.target_y != 0) continue; // already moving → skip
    if (!u.faction) continue;
    if (u.faction==factions && (u.selected || !(u.faction->technology & TECHNOLOGY_SNIFFING))) continue; // disable player AI is sniffing is disabled
    if(u.faction==factions+1) continue;
    if (u.faction == ANIMAL_FACTION) {
        if ((float)GetRandomValue(0, 1000000) / 1000000.0f > AI_ORDER_CHANCE * 20.f * dt) continue;
        Texture2D* baseTex = terrainGrid[(int)u.y][(int)u.x].texture;
        float tx = 0, ty = 0;
        bool found = false;
        float bestDist = 40.f*40.f;
        if(u.max_health<=u.health) bestDist *= 0.25f;
        for (int j = 0; j < num_units; j++) {
            Unit &o = units[j];
            if (!o.faction || o.faction==factions+1 || o.faction==factions+2) continue;
            if (o.faction==ANIMAL_FACTION) continue;
            if (o.capturing) continue;
            if (o.health <= 0) continue;
            //if ((time_norm>0.8f/* || (time_norm>0.35f && time_norm>0.5f)*/) && o.texture!=&tex::oil && o.texture!=&tex::warehouse && o.texture!=&tex::esper && o.texture!=&tex::lighthouse) continue; // at the last stretch attack the victory locations with all means (don't go for curio though because they are a pain to capture)
            float dx = o.x - u.x;
            float dy = o.y - u.y;
            float d2 = dx*dx + dy*dy;
            if (d2 < bestDist) {
                bestDist = d2;
                if(u.health>u.max_health/2) {
                    tx = o.x;
                    ty = o.y;
                }
                else {
                    tx = u.x-(o.x-u.x)+GetRandomValue(-1,1);
                    ty = u.y-(o.y-u.y)+GetRandomValue(-1,1);
                }
                found = true;
            }
        }
        if (!found) {
            tx = GetRandomValue(10, GRID_SIZE - 10);
            ty = GetRandomValue(10, GRID_SIZE - 10);
            if(baseTex==terrainGrid[(int)ty][(int)tx].texture) {
                u.target_x = tx;
                u.target_y = ty;
            }
        }
        else if(bestDist>u.range*u.range) {
            u.target_x = tx;
            u.target_y = ty;
        }
        continue;
    }
    if ((float)GetRandomValue(0, 1000000) / 1000000.0f > AI_ORDER_CHANCE * dt) continue;
    // ---------------------------------------------------------
    // Movement
    // ---------------------------------------------------------
    float bestDist = 999999.0f;
    float tx = 0, ty = 0;
    bool found = false;
    // if we are far from target, stop only for stuff that is close to here
    if(u.target_x && u.target_y && (u.target_x-u.x)*(u.target_x-u.x)+(u.target_y-u.y)*(u.target_y-u.y)>10) bestDist = 40.f;
    if(GetRandomValue(0, 100) > 10)
        for (int j = 0; j < num_units; j++) {
            if(!u.faction->visible_knowledge[j]) continue;
            Unit &o = units[j];
            //if (!o.faction) continue;
            if (o.health <= 0) continue;
            bool isEnemyCapturable =
                (o.capturing != nullptr) &&
                (o.faction != u.faction && o.faction);
            bool isOwnDamagedStructure =
                (o.faction == u.faction) &&
                (o.speed == 0 && o.capturing) &&
                (o.health < o.max_health*0.8f);
            if (!isEnemyCapturable && !isOwnDamagedStructure && o.faction) continue;
            if ((time_norm>0.8f || (time_norm>0.35f && time_norm>0.5f)) && o.texture!=&tex::oil && o.texture!=&tex::warehouse && o.texture!=&tex::esper && o.texture!=&tex::lighthouse) continue; // at the last stretch attack the victory locations with all means
            float dx = o.x - u.x;
            float dy = o.y - u.y;
            float d2 = dx*dx + dy*dy;
            if (d2 < bestDist && d2>=0) {
                bestDist = d2;
                tx = o.x;
                ty = o.y;
                found = true;
            }
        }
    if (!found) {
        if(u.faction && (u.faction->technology & TECHNOLOGY_TRENCHES) && u.texture==&tex::tank && GetRandomValue(0,100)<50) {
            u.texture = &tex::ghost; // prevent explosion
            CREATE_RAILGUN(u.faction, u.x, u.y);
            units[num_units-1].health = units[num_units-1].max_health*u.health/u.max_health;
            units[num_units-1].popup = "entrenched";
            units[num_units-1].capturing = nullptr;
            u.health = 0;
            continue;
        }
        if (u.health>3.1f && (u.faction->technology & TECHNOLOGY_TURTLING) && u.speed && !is_mecha(u)) {
            u.health -= 3;
            u.max_health -= 3;
            if(true) {
                float x = u.x + cos(u.angle*DEG2RAD)*u.size;
                float y = u.y + sin(u.angle*DEG2RAD)*u.size;
                CREATE_ROCK(u.faction, x, y);
                units[num_units-1].popup = "turtling";
                units[num_units-1].capturing = nullptr;
            }
            else {
                u.popup = "harmed";
                u.capturing = nullptr;
            }
        }
        if(GetRandomValue(0, 100)<5 ) {
            tx = GetRandomValue(10, GRID_SIZE - 10);
            ty = GetRandomValue(10, GRID_SIZE - 10);
            u.target_x = tx;
            u.target_y = ty;
        }
        continue;
    }
    u.target_x = tx;
    u.target_y = ty;

    // ---------------------------------------------------------
    // Order nearby units of same faction to follow
    // ---------------------------------------------------------
    for (int j = 0; j < num_units; j++) {
        if (i == j) continue;
        Unit &o = units[j];
        if (!o.faction) continue;
        if (o.faction != u.faction) continue;
        if (o.health <= 0) continue;
        if (o.speed <= 0) continue;
        if (o.texture==&tex::esper) continue;
        float dx = o.x - u.x;
        float dy = o.y - u.y;
        // grab everything if the game has progressed enough
        if (dx*dx + dy*dy <= AI_ORDER_RADIUS * AI_ORDER_RADIUS && (GetRandomValue(0, 100) < 10 || time_norm>0.5f) && !o.selected) {
            // only order units not already moving
            o.target_x = tx;
            o.target_y = ty;
        }
    }
}

