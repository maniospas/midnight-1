for (int i = 0; i < num_units; i++) {
    Unit &u = units[i];
    if (u.attack_target_x == 0 && u.attack_target_y == 0) continue;
    if (u.attack_x == 0 && u.attack_y == 0) continue;
    float ax = u.attack_x;
    float ay = u.attack_y;
    float tx = u.attack_target_x;
    float ty = u.attack_target_y;
    float dx = tx - ax;
    float dy = ty - ay;
    float dist = dx*dx + dy*dy;
    if (dist < 0.3f) {
        for (int j = 0; j < num_units; j++) {
            Unit &o = units[j];
            if (o.faction == u.faction && !o.capturing) continue;
            float ox = o.x;
            float oy = o.y;
            float pdx = ox - tx;
            float pdy = oy - ty;
            if (pdx*pdx + pdy*pdy < 0.3f) { // hit radius approx
                int oxi = (int)(o.x+0.5f);
                int oyi = (int)(o.y+0.5f);
                float skipChance = 0.f;//0.25f;
                float u_damage = u.damage;
                if(u.faction && (u.faction->technology & TECHNOLOGY_NUCLEAR)) {
                    u_damage *= 2.f;
                    if(GetRandomValue(0, 100)<20 && !u.popup) {
                        u.popup = "nuclear";
                        u.popup_texture=&tex::nuclear;
                    }
                }
                if(u.faction && (u.faction->technology & TECHNOLOGY_FLANKING)) {
                    if((u.speed||u.texture==&tex::railgun) && (o.speed||o.texture==&tex::railgun)) {
                        float diff = o.angle - u.angle;
                        while (diff > 180.0f) diff -= 360.0f;
                        while (diff < -180.0f) diff += 360.0f;
                        if(diff<0) diff = -diff;
                        if(diff<90.0) { // the angles should be opposite
                            u_damage *= 2.f;
                            if(GetRandomValue(0, 100)<20 || !u.popup) {
                                u.popup = "flanking";
                                u.popup_texture=&tex::flank;
                            }
                        }
                    }
                }
                if (oxi >= 0 && oyi >= 0 && oxi < GRID_SIZE && oyi < GRID_SIZE) skipChance -= terrainGrid[oyi][oxi].extra_sight/2.f;
                if(o.faction && (o.faction->technology&TECHNOLOGY_MECHA) && is_mecha(o)) skipChance += 0.5f;
                if(o.faction && (o.faction->technology&TECHNOLOGY_HEROICS) && o.name==hero_name) skipChance += 0.3f;
                if(o.faction && (o.faction->technology&TECHNOLOGY_HEROICS) && o.name==veteran_name) skipChance += 0.3f;
                if(o.faction && (o.faction->technology&TECHNOLOGY_LUXURY) && (u.texture==&tex::ghost || u.texture==&tex::bison || u.texture==&tex::wolf || u.texture==&tex::rat || u.texture==&tex::snowman)) skipChance += 0.5f;
                if(u_damage>=o.health && o.faction && (o.faction->technology & TECHNOLOGY_GRIT)) skipChance += 0.5f;
                float has_used_sniping = 0;
                if(u.faction && (u.faction->technology&TECHNOLOGY_SNIPING)) {
                    has_used_sniping = skipChance;
                    skipChance -= 0.5f;
                }
                if(skipChance<0.f) skipChance = 0.f;
                if(skipChance>0.95f) skipChance = 0.95f;

                if ((float)GetRandomValue(0, 1000000) / 1000000.0f >= skipChance) {
                    if(o.capturing && o.faction == u.faction) o.health += 1;
                    else if(o.capturing && o.capturing==factions+1) o.health -= CAPTURE_RATE*u_damage;
                    else if(o.capturing) o.health -= CAPTURE_RATE*u_damage*0.5f;
                    else {
                        if(is_mecha(o)) {
                            u_damage *= 0.33f;
                            if(u.faction && (u.faction->technology&TECHNOLOGY_ANTIMECHA) && GetRandomValue(0, 100)>=25) {
                                u_damage *= 2.0f;
                                if(GetRandomValue(0, 100)>=25 && !o.popup) {
                                    o.popup = "sabotaged";
                                    o.popup_texture = nullptr;
                                }
                            }
                        }
                        else if(o.texture==&tex::fort && u.faction && (u.faction->technology&TECHNOLOGY_ANTIMECHA)) {
                            u_damage *= 2.0f;
                            if(GetRandomValue(0, 100)>=25 && !o.popup) {
                                o.popup = "sabotaged";
                                o.popup_texture = nullptr;
                            }
                        }
                        o.health -= u_damage;
                        if(o.speed) {
                            float dx = o.x-u.x;
                            float dy = o.y-u.y;
                            float r2 = dx*dx+dy*dy;
                            if(r2 && o.max_health) {
                                float mult = u_damage/(o.max_health);
                                if(mult>1) mult = 1;
                                mult = mult/sqrtf(r2*10);
                                o.x += mult*dx;
                                o.y += mult*dy;
                                o.angle += mult*cos(10*t)*30;
                            }
                        }
                    }
                    if(has_used_sniping && GetRandomValue(0, 100)>=50) {
                        o.popup = "sniped";
                        o.popup_texture = &tex::snipe;
                    }
                }
                else if(u_damage>=o.health && o.faction && (o.faction->technology & TECHNOLOGY_GRIT)) {
                    o.popup = "grit";
                    o.popup_texture = &tex::grit;
                }
                else if(o.name==hero_name && (o.faction->technology&TECHNOLOGY_HEROICS)) {
                    o.popup = "heroics";
                    o.popup_texture = &tex::heroics;
                }
                else if(o.name==veteran_name && (o.faction->technology&TECHNOLOGY_HEROICS)) {
                    o.popup = "heroics";
                    o.popup_texture = &tex::heroics;
                }
                else if(o.faction && (o.faction->technology&TECHNOLOGY_MECHA) && is_mecha(o)) {
                    o.popup = "hull";
                    o.popup_texture = &tex::shield;
                }
                else if(o.faction && (o.faction->technology&TECHNOLOGY_LUXURY) && (u.texture==&tex::ghost || u.texture==&tex::bison || u.texture==&tex::wolf || u.texture==&tex::rat || u.texture==&tex::snowman)) {
                    o.popup = "pristine";
                    o.popup_texture = &tex::pristine;
                }
                else {
                    o.popup = "cover";
                    o.popup_texture = &tex::shield;
                }
                if (o.health >= o.max_health) o.health = o.max_health;
                if (o.health < 0) o.health = 0;
                if (o.health <=0 && u.max_health) {
                    float experience_bonus = o.experience/2 + (float)(o.max_health)/(float)(u.max_health);
                    if(u.faction && (u.faction->technology&TECHNOLOGY_FIGHT)) experience_bonus *= 2.f;
                    if(u.texture==&tex::ghost && u.faction && (u.faction->technology&TECHNOLOGY_ARTIFICIAL)) experience_bonus *= 5.f;
                    u.experience += experience_bonus;
                    if(u.experience>=10 && (!is_mecha(u) || (u.faction && (u.faction->technology & TECHNOLOGY_INDUSTRY)))
                        && u.name!=veteran_name && u.name!=hero_name) {
                        u.size *= 1.2;
                        u.name = veteran_name;
                        if(u.texture==&tex::human) u.texture = &tex::scout;
                        u.damage *= 2;
                        u.max_health += 5;
                        u.health += 5;
                        u.popup = "new veteran";
                        u.popup_texture = nullptr;
                    }
                    if(u.experience>=50 && u.name==veteran_name) {
                        u.size *= 1.2;
                        u.name = hero_name;
                        if(u.texture==&tex::scout) u.texture = &tex::hero;
                        u.damage *= 1.5;
                        u.max_health += 5;
                        u.health += 5;
                        u.popup = "new hero";
                        u.popup_texture = nullptr;
                    }
                    if(u.experience>70 && u.name==hero_name) {
                        u.experience -= 20;
                        int r = GetRandomValue(0, 100);
                        if(r<25) {
                            u.attack_rate *= 1.5f;
                            u.popup = "hero: aggression";
                            u.popup_texture = nullptr;
                        }
                        else if(r<50) {
                            u.speed *= 1.2f;
                            u.popup = "hero: faster";
                            u.popup_texture = nullptr;
                        }
                        else if(r<75){
                            u.max_health += 2.f;
                            u.health += 2.f;
                            u.popup = "hero: healthier";
                            u.popup_texture = nullptr;
                        }
                        else {
                            u.range *= 1.2f;
                            u.popup = "hero: farsight";
                            u.popup_texture = nullptr;
                        }
                    }
                }
                if(o.health<=0 && u.faction && (u.faction->technology & TECHNOLOGY_HOMUNCULI) && o.texture==&tex::ghost && GetRandomValue(0, 99) < 50) {
                    o.faction = u.faction;
                    o.health = o.max_health;
                    o.popup = "homunculi";
                    o.popup_texture = nullptr;
                }
                else if(o.health<=0 && u.faction && (u.faction->technology & TECHNOLOGY_HIJACK) && is_mecha(o) && GetRandomValue(0, 99) < 50) {
                    o.faction = u.faction;
                    o.health = o.max_health;
                    o.popup = "hijacked";
                    o.popup_texture = &tex::hijack;
                    o.animation = 0.f;
                }
                else if(o.health<=0 && o.texture==&tex::wolf && GetRandomValue(0, 99) < 50) {
                    o.faction = u.faction;
                    o.health = o.max_health;
                    o.popup = o.faction==ANIMAL_FACTION?"wild":"tamed";
                    o.popup_texture = nullptr;
                }
                else if(o.health<=0 && u.faction && (u.faction->technology & TECHNOLOGY_TAMING) && (o.texture==&tex::bison || o.texture==&tex::rat || o.texture==&tex::snowman) && GetRandomValue(0, 99) < 50) {
                    o.faction = u.faction;
                    o.health = o.max_health;
                    o.popup = "tamed";
                    o.popup_texture = nullptr;
                }
                else if(o.health<=0 && o.faction && (o.faction->technology & TECHNOLOGY_UNSTABLE) && (o.texture==&tex::rat || o.texture==&tex::human || o.texture==&tex::scout || o.texture==&tex::hero)) {
                    units[j] = { \
                        &tex::ghost,  /* texture */
                        "Bloo",       /* name */
                        2.0,          /* speed */
                        o.x,          /* x */
                        o.y,          /* y */
                        2.0,          /* attack_rate */
                        7.0,          /* range */
                        0.5,          /* damage */
                        0.0,          /* experience */
                        o.angle,      /* angle */
                        0.4,          /* size */
                        3.0,          /* health */
                        3.0,          /* max_health */
                        o.faction,   /* faction */
                        nullptr   /* faction */
                    };
                    units[j].popup = "unstable";
                    units[j].popup_texture = nullptr;
                }
                else if(o.health<=0 && u.faction && (u.faction->technology & TECHNOLOGY_BIOWEAPON) && (o.texture==&tex::human || o.texture==&tex::scout || o.texture==&tex::hero)) {
                    units[j] = { \
                        &tex::ghost,  /* texture */
                        "Bloo",       /* name */
                        2.0,          /* speed */
                        o.x,          /* x */
                        o.y,          /* y */
                        2.0,          /* attack_rate */
                        7.0,          /* range */
                        0.5,          /* damage */
                        0.0,          /* experience */
                        o.angle,      /* angle */
                        0.4,          /* size */
                        3.0,          /* health */
                        3.0,          /* max_health */
                        o.faction,   /* faction */
                        nullptr   /* faction */
                    };
                    units[j].popup = "bioweapon";
                    units[j].popup_texture = nullptr;
                }
                if (o.health>0 && o.health<o.max_health && o.capturing) {
                    if(o.capturing==factions+1) o.popup = "capturing";
                    else o.popup = "contested";
                    o.popup_texture = nullptr;
                }
                if (o.health<=0 && o.capturing) {
                    o.animation = 0.f;
                    if(o.faction==factions) {
                        if(o.texture==&tex::camp) {
                            last_message = "Important loss: Camp";
                            last_message_counter = 0.f;
                        }
                        else if(o.texture==&tex::oil) {
                            last_message = "Important loss: Oil";
                            last_message_counter = 0.f;
                        }
                        else if(o.texture==&tex::warehouse) {
                            last_message = "Important loss: Storage";
                            last_message_counter = 0.f;
                        }
                        else if(o.texture==&tex::warehouse) {
                            last_message = "Important loss: Esper";
                            last_message_counter = 0.f;
                        }
                        else if(o.texture==&tex::lighthouse) {
                            last_message = "Important loss: Big bro";
                            last_message_counter = 0.f;
                        }
                        else if(o.texture==&tex::curio) {
                            last_message = "Important loss: Curio";
                            last_message_counter = 0.f;
                        }
                    }
                    if(u.faction==factions) {
                        if(o.texture==&tex::camp) {
                            last_message = "Nice capture: Camp";
                            last_message_counter = 0.f;
                        }
                        else if(o.texture==&tex::oil) {
                            last_message = "Nice capture: Oil";
                            last_message_counter = 0.f;
                        }
                        else if(o.texture==&tex::warehouse) {
                            last_message = "Nice capture: Storage";
                            last_message_counter = 0.f;
                        }
                        else if(o.texture==&tex::esper) {
                            last_message = "Nice capture: Esper";
                            last_message_counter = 0.f;
                        }
                        else if(o.texture==&tex::curio) {
                            last_message = "Nice capture: Curio";
                            last_message_counter = 0.f;
                        }
                        else if(o.texture==&tex::lighthouse) {
                            last_message = "Nice capture: Big bro";
                            last_message_counter = 0.f;
                        }
                    }
                    o.popup = "captured";
                    o.popup_texture = nullptr;
                    o.capturing = u.faction;
                    if(o.capturing==ANIMAL_FACTION) {
                        o.capturing = factions+1; // animals cannot capture
                        o.faction = o.capturing;
                        o.health = o.max_health;
                    }
                    else {
                        o.faction = o.capturing;
                        o.health = o.max_health;
                        if(o.texture==&tex::tank) o.capturing = nullptr; // only capture tanks once
                        if(o.texture==&tex::van) o.capturing = nullptr; // only capture vans once
                        if(o.texture==&tex::railgun) o.capturing = nullptr; // only capture railguns once
                        if(o.texture==&tex::roomba) o.capturing = nullptr; // only capture roombas once
                    }

                }
            }
            else {
                //u.popup = "missed";
            }
        }
        u.attack_target_x = 0;
        u.attack_target_y = 0;
        continue;
    }
    dist = sqrtf(dist);
    float attackSpeed = 3.0f * dt * (u.attack_rate<1.0f?1.0f:u.attack_rate);
    if(u.faction && (u.faction->technology&TECHNOLOGY_HELLBRINGER) && (u.name==veteran_name || u.name==hero_name)) attackSpeed *= 2.f;
    u.attack_x += (dx / dist) * attackSpeed;
    u.attack_y += (dy / dist) * attackSpeed;
}