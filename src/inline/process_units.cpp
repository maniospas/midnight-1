const float TURN_RATE = 36.0f;
const float AIM_THRESHOLD = 5.0f;
for (int i = 0; i < num_units; i++) {
    Unit &u = units[i];
    if(u.popup) {
        u.animation += dt*0.3f;
        if(u.animation>1.0f) {
            u.animation = 0.f;
            u.popup = nullptr;
            u.popup_texture = nullptr;
        }
    }
    if(u.x<2) u.x = 2;
    if(u.y<2) u.y = 2;
    if(u.x>=GRID_SIZE-2) u.x = GRID_SIZE-2;
    if(u.y>=GRID_SIZE-2) u.y = GRID_SIZE-2;
    if (u.texture == &tex::blood) {
        if ((float)GetRandomValue(0, 1000000) / 1000000.0f < 0.005f * dt) {
            num_units--;
            u = units[num_units];
            i--;
            continue;
        }
        else if ((float)GetRandomValue(0, 1000000) / 1000000.0f < 0.005f * dt) {
            u = { \
                &tex::ghost,  /* texture */
                "Bloo",       /* name */
                3.0,          /* speed */
                u.x,          /* x */
                u.y,          /* y */
                2.0,          /* attack_rate */
                7.0,          /* range */
                0.5,          /* damage */
                0.0,          /* experience */
                u.angle,      /* angle */
                0.4,          /* size */
                3.0,          /* health */
                3.0,          /* max_health */
                factions+2,   /* faction */
                nullptr   /* faction */
            };
        }
        continue; // blood does nothing else
    }
    if(!u.faction)
        continue;
    if (u.capturing && u.faction && (float)GetRandomValue(0, 1000000) / 1000000.0f < dt
            *((u.faction->technology&TECHNOLOGY_OWNERSHIP)?1.0f:0.5f)
            *(u.faction->count_members<=u.faction->industry?1.0f:OVER_CAP_REGEN_RATE)
        ) {
        u.health += CAPTURE_RATE;
        if (u.health > u.max_health)
            u.health = u.max_health;
    }
    if(u.health<=0 && u.max_health) {
        if(!u.speed || !u.faction) {
            u.health = u.max_health;
        }
        if(terrainGrid[(int)u.y][(int)u.x].texture==&tex::water){
            num_units--;
            u = units[num_units];
            i--;
        }
        else if(u.texture==&tex::bison){// || u.texture==&tex::wolf) {
            terrainGrid[(int)u.y][(int)u.x].speed /= 2; // terrain becomes uneven
            u = { \
                &tex::hide,  /* texture */
                "Hide",       /* name */
                0.0,          /* speed */
                u.x,          /* x */
                u.y,          /* y */
                4.0,          /* attack_rate */
                2.0,          /* range */
                0.0,          /* damage */
                0.0,          /* experience */
                u.angle,      /* angle */
                0.8,          /* size */
                15.0,          /* health */
                15.0,          /* max_health */
                factions+1,       /* faction */
                factions+1       /* faction */
            };
        }
        else if(is_mecha(u)) {
            int ux = (int)(u.x+0.5f);
            int uy = (int)(u.y+0.5f);
            if(visible[uy][ux]) {
                int range = (int)u.size + 1;
                if (ux >= xMin-range && ux < xMax+range && uy >= yMin-range && uy < yMax+range)
                    sound::explosion.Play(camera.zoom*0.2f);
            }
            terrainGrid[uy][ux].speed /= 2; // terrain becomes uneven
            if(terrainGrid[uy][ux].extra_sight>-0.5f)
                terrainGrid[uy][ux].extra_sight = -0.5f; // extra dodge
            u = { \
                &tex::crater,  /* texture */
                "Crater",      /* name */
                0.0,          /* speed */
                u.x,          /* x */
                u.y,          /* y */
                0.0,          /* attack_rate */
                0.0,          /* range */
                0.0,          /* damage */
                0.0,          /* experience */
                u.angle,      /* angle */
                1.5,          /* size */
                0.0,          /* health */
                0.0,          /* max_health */
                nullptr       /* faction */
            };
        }
        else if(u.texture==&tex::ghost || u.texture==&tex::rock) { // use bloos to clean up units
            num_units--;
            u = units[num_units];
        }
        else {
            if(visible[(int)u.y][(int)u.x]) {
                int ux = (int)u.x;
                int uy = (int)u.y;
                int range = (int)u.size + 1;
                if (ux >= xMin-range && ux < xMax+range && uy >= yMin-range && uy < yMax+range) {
                    if(u.texture==&tex::human) sound::dead.Play(camera.zoom*1.0f);
                    else sound::damage.Play(camera.zoom*0.2f);
                }
            }
            u = { \
                &tex::blood,  /* texture */
                "Blood",      /* name */
                0.0,          /* speed */
                u.x,          /* x */
                u.y,          /* y */
                0.0,          /* attack_rate */
                0.0,          /* range */
                0.0,          /* damage */
                0.0,          /* experience */
                u.angle,      /* angle */
                0.5,          /* size */
                0.0,          /* health */
                0.0,          /* max_health */
                nullptr       /* faction */
            };
        }
        continue;
    }
    if(u.texture==&tex::oil)
        u.faction->victory_points += 3.f;
    if(u.texture==&tex::esper)
        u.faction->victory_points += 2.f;
    if(u.texture==&tex::warehouse)
        u.faction->victory_points += 2.f;
    if(u.texture==&tex::curio)
        u.faction->victory_points += 1.f;
    // if(u.texture==&tex::fort)
    //     u.faction->victory_points -= 0.334f;
    if(u.texture==&tex::radio && u.faction && (u.faction->technology & TECHNOLOGY_PROPAGANDA) ) {
        u.faction->victory_points += 0.5f;
    }
    if(u.texture==&tex::lighthouse && u.faction) {
        u.faction->victory_points += 0.5f;
        if(u.faction->technology & TECHNOLOGY_DISCOURSE) {
            u.faction->victory_points += 0.5f;
            if(u.faction->technology & TECHNOLOGY_AIFARM) u.faction->victory_points += 1.6f;
        }
    }
    if(u.texture==&tex::lab)
        continue;
    if(u.texture==&tex::field) {
        if(u.faction->technology & TECHNOLOGY_FARMING) {
            if((float)GetRandomValue(0, 1000000) / 1000000.0f <dt*0.01) {u.texture = &tex::field_empty;u.popup = "barren";u.popup_texture=nullptr;} // once every 100 sec
        }
        else if((float)GetRandomValue(0, 1000000) / 1000000.0f <dt*0.05) {u.texture = &tex::field_empty;u.popup = "barren";u.popup_texture=nullptr;} // once every 20 sec
        continue;
    }
    if(u.texture==&tex::field_little) {
        if((float)GetRandomValue(0, 1000000) / 1000000.0f <dt*0.05) {u.texture = &tex::field;u.popup = "bloom";u.popup_texture=nullptr;} // once every 20 sec
        continue;
    }
    if(u.texture==&tex::field_empty) {
        if(u.faction->technology & TECHNOLOGY_FARMING) {
            if((float)GetRandomValue(0, 1000000) / 1000000.0f <dt*0.1) {u.texture = &tex::field_little;u.popup = "grows";u.popup_texture=nullptr;} // once every 10 sec
        }
        else if((float)GetRandomValue(0, 1000000) / 1000000.0f <dt*0.05) {u.texture = &tex::field_little;u.popup = "grows";u.popup_texture=nullptr;} // once every 20 sec
        continue;
    }
    if(u.texture==&tex::hide) {
        if((float)GetRandomValue(0, 1000000) / 1000000.0f <dt*0.005) { // once every 10 mins it may become rats
            CREATE_RAT(ANIMAL_FACTION, u.x-0.2, u.y+0.2);
            CREATE_RAT(ANIMAL_FACTION, u.x-0.2, u.y-0.2);
            CREATE_RAT(ANIMAL_FACTION, u.x+0.2, u.y+0.2);
            CREATE_RAT(ANIMAL_FACTION, u.x+0.2, u.y-0.2);
            CREATE_RAT(ANIMAL_FACTION, u.x, u.y);
            u = { \
                &tex::blood,  /* texture */
                "Blood",      /* name */
                0.0,          /* speed */
                u.x,          /* x */
                u.y,          /* y */
                0.0,          /* attack_rate */
                0.0,          /* range */
                0.0,          /* damage */
                0.0,          /* experience */
                u.angle,      /* angle */
                0.5,          /* size */
                0.0,          /* health */
                0.0,          /* max_health */
                nullptr       /* faction */
            };
        }
        continue;
    }
    if(u.texture==&tex::mine)
        continue;
    if (u.texture == &tex::rat) {
        if((float)GetRandomValue(0, 1000000) / 1000000.0f * 30.f<dt*u.attack_rate*(1-time_norm)*(1-time_norm)*(1-time_norm)*(1-time_norm)) {
            int canMake = (int)u.faction->industry-(int)u.faction->count_members;
            float sx = u.x + (GetRandomValue(-5000, 5000) * 0.0002f);
            float sy = u.y + (GetRandomValue(-5000, 5000) * 0.0002f);
            if(canMake>0) {
                CREATE_RAT(u.faction, sx, sy);
            }
        }
    }
    if (u.texture == &tex::curio) {
        if(u.faction && (float)GetRandomValue(0, 1000000) / 1000000.0f * 30.f<CAMP_SPAWN_RATE*dt*3) {
            int canMake = 1;
            if (canMake > 0) {
                const int can_make_limit = 1;//(u.faction && u.faction->technology & TECHNOLOGY_HARDCORE)?4:2;
                if (canMake > can_make_limit) canMake = can_make_limit;
                for (int k = 0; k < canMake; k++) {
                    if (num_units >= MAX_UNITS) break;
                    float sx = u.x + (GetRandomValue(-5000, 5000) * 0.0002f);
                    float sy = u.y + (GetRandomValue(-5000, 5000) * 0.0002f);
                    CREATE_RAT(ANIMAL_FACTION, sx, sy);
                    if(u.faction && (u.faction->technology & TECHNOLOGY_NERDS)) {
                        units[num_units-1].max_health -= 1;
                        units[num_units-1].health -= 1;
                    }
                    if(u.faction && (u.faction->technology & TECHNOLOGY_UNSTABLE)) {
                        units[num_units-1].max_health -= 1;
                        units[num_units-1].health -= 1;
                    }
                    if(u.faction && (u.faction->technology & TECHNOLOGY_TOUGH)) {
                        units[num_units-1].max_health += 1;
                        units[num_units-1].health += 1;
                    }
                }
            }
        }
        continue;
    }
    if (u.texture == &tex::camp) {
        if(u.faction && (float)GetRandomValue(0, 1000000) / 1000000.0f * 30.f<CAMP_SPAWN_RATE*dt*u.attack_rate*(1.1f-u.faction->count_members/(float)(1+u.faction->industry)) && u.faction!=factions+1) {
            int canMake = (int)u.faction->industry-(int)u.faction->count_members;
            if (canMake > 0) {
                const int can_make_limit = (u.faction->technology & TECHNOLOGY_HARDCORE)?4:2;
                if (canMake > can_make_limit) canMake = can_make_limit;
                for (int k = 0; k < canMake; k++) {
                    if (num_units >= MAX_UNITS) break;
                    float sx = u.x + (GetRandomValue(-5000, 5000) * 0.0002f);
                    float sy = u.y + (GetRandomValue(-5000, 5000) * 0.0002f);
                    if(u.faction&&(u.faction->technology&TECHNOLOGY_EVOLUTION)&&GetRandomValue(0, 100)<10) {
                        CREATE_SNOWMAN(u.faction, sx, sy);
                    }
                    else {
                        CREATE_HUMAN(u.faction, sx, sy);
                    }
                    // superiority has 25% chance of spawning something hostile
                    /*if(u.faction&&(u.faction->technology&TECHNOLOGY_SUPERIORITY)&&GetRandomValue(0, 100)<25)
                        units[num_units-1].faction = factions+2;
                    else*/
                    {
                        if(u.faction && (u.faction->technology & TECHNOLOGY_NERDS)) {
                            units[num_units-1].max_health -= 1;
                            units[num_units-1].health -= 1;
                        }
                        if(u.faction && (u.faction->technology & TECHNOLOGY_UNSTABLE)) {
                            units[num_units-1].max_health -= 1;
                            units[num_units-1].health -= 1;
                        }
                        if(u.faction && (u.faction->technology & TECHNOLOGY_TOUGH)) {
                            units[num_units-1].max_health += 1;
                            units[num_units-1].health += 1;
                        }
                    }
                }
            }
        }
        continue;
    }
    if(u.faction && (u.faction->technology & TECHNOLOGY_MOBILE_FORTRESS) && u.texture==&tex::railgun && (float)GetRandomValue(0, 1000000) / 1000000.0f<dt*0.015) {
        u = { \
            &tex::tank,   /* texture */ \
            "Tank",       /* name */ \
            3.0,          /* speed */ \
            (float)(u.x),   /* x */ \
            (float)(u.y),   /* y */ \
            0.5,          /* attack_rate */ \
            6.0,         /* range */ \
            8.0,          /* damage */ \
            0.0,          /* experience */ \
            0.0,          /* angle */ \
            0.8,          /* size */ \
            20.0,         /* health */ \
            20.0,         /* max_health */ \
            (u.faction),    /* faction */ \
            nullptr,    /* faction */ \
            0.3           /* extra scale*/\
        };
        u.popup = "mobile fort";
        u.popup_texture=nullptr;
        continue;
    }
    if(u.texture==&tex::esper && (float)GetRandomValue(0, 1000000) / 1000000.0f<0.03*dt) { // once every 30 seconds the esper changes their mind
        u.target_x = u.x+GetRandomValue(-40,40);
        u.target_y = u.y+GetRandomValue(-40,40);
        if(u.target_x<1) u.target_x = 1;
        if(u.target_y<1) u.target_y = 1;
        if(u.target_x>=GRID_SIZE-2) u.target_x = GRID_SIZE-2;
        if(u.target_y>=GRID_SIZE-2) u.target_y = GRID_SIZE-2;
    }

    int ux = (int)(u.x+0.5f);
    int uy = (int)(u.y+0.5f);
    float u_speed = terrainGrid[uy][ux].speed;
    if(u_speed<1.f && u.texture==&tex::snowman && (terrainGrid[uy][ux].texture==&tex::mountain || terrainGrid[uy][ux].texture==&tex::hill)) u_speed = 2.f;
    if(u_speed<1.2f && is_mecha(u) && u.faction && (u.faction->technology && TECHNOLOGY_DRIVER)) u_speed = 1.2f;
    if(u_speed<1.f && u.faction && (u.faction->technology && TECHNOLOGY_AGILE)) u_speed = (1.f+u_speed)*0.5f;
    if(u_speed<1.f && u.faction && (u.faction->technology & TECHNOLOGY_SEAFARERING) && terrainGrid[uy][ux].texture==&tex::water) u_speed = 1.5f;
    u_speed *= u.speed;
    float extra_sight = terrainGrid[(int)u.y][(int)u.x].extra_sight;
    if((u.texture==&tex::railgun || u.texture==&tex::radio || u.texture==&tex::lighthouse) && extra_sight<0.f) extra_sight = 0.f;
    float u_base_range = u.range*(1+extra_sight);
    if(u.faction->technology&TECHNOLOGY_TRACK) extra_sight += 0.7f;
    float u_range = u.range*(1+extra_sight);
    if((u.texture==&tex::camp || u.texture==&tex::warehouse) && (u.faction->technology & TECHNOLOGY_EXPLORE)) u_range = 25.f;
    if(u.faction && (u.faction->technology & TECHNOLOGY_INFRASTRUCTURE) && (u.texture==&tex::radio || u.texture==&tex::fort)) u_range *= 2.0f;
    // attack (interrupt movement to attack)
    if (u.attack_target_x == 0 && u.attack_target_y == 0 && u.capturing!=factions+1) {
        float r = (float)GetRandomValue(0, 1000000) / 1000000.0f;
        float mul = 1.f;
        //if(u.target_x==0 && u.target_y==0) mul *= 40.f;
        float u_attack_rate = u.attack_rate;
        if(u.faction && (u.faction->technology&TECHNOLOGY_HELLBRINGER) && (u.name==veteran_name || u.name==hero_name)) u_attack_rate *= 3.f;
        if (r < dt * mul * u_attack_rate) {
            u.attack_x = 0;
            u.attack_y = 0;
            float bestDist = 999999.0f;
            float bestCaptureDist = bestDist;
            Unit* best = nullptr;
            Unit* bestCapture = nullptr;
            bool is_roomba = u.texture==&tex::roomba;
            bool best_found_via_tracking = false;
            // find closest enemy in range
            for (int j = 0; j < num_units; j++) {
                if(i == j) continue;
                Unit &o = units[j];
                //if (o.capturing) continue;
                //if (o.faction == u.faction && (!o.capturing || o.health>=o.max_health)) continue;
                if(o.health <=0) continue;
                float dx = o.x - u.x;
                float dy = o.y - u.y;
                float d2 = dx*dx + dy*dy;
                float effective_range = (u_range+1+o.size);
                effective_range *= effective_range;
                float effective_base_range = u_base_range+1+o.size;
                effective_base_range *= effective_base_range;
                bool in_range = d2 < effective_range;
                bool in_effective_base_range = d2 < effective_base_range;
                if(in_range && o.capturing) {
                    if(u.faction && !u.faction->visible_knowledge[j] && (u.faction->technology & TECHNOLOGY_WONDER)) {
                        u.faction->technology_progress += 0.05f;
                        if(u.faction==factions) {
                            o.popup = "wonder";
                            o.popup_texture = &tex::wonder;
                        }
                    }
                    u.faction->visible_knowledge.set(j);
                }
                if(is_roomba && o.texture!=&tex::ghost && o.texture!=&tex::wolf && o.texture!=&tex::rat && o.texture!=&tex::bison) continue;
                if(o.faction == u.faction) continue;
                if (u.faction==ANIMAL_FACTION && (o.faction==factions+1 || o.faction==factions+2)) continue;
                if(in_range) {
                    if(o.capturing) {
                        if(d2 < bestCaptureDist) {
                            bestCaptureDist = d2;
                            bestCapture = &o;
                            best_found_via_tracking = !in_effective_base_range;
                        }
                    }
                    else {
                        if(d2 < bestDist) {
                            bestDist = d2;
                            best = &o;
                            best_found_via_tracking = !in_effective_base_range;
                        }
                    }
                }
            }
            // we re going to try to capture only if there's no enemy
            if(!best) best = bestCapture;
            if(best) {
                u.attack_target_x = best->x;
                u.attack_target_y = best->y;
                if((u.attack_target_x-u.target_x)*(u.attack_target_x-u.target_x)+(u.attack_target_y-u.target_y)*(u.attack_target_y-u.target_y)<u_range*u_range/16
                    && u.faction!=factions && u.faction /*&& (u.faction->technology & TECHNOLOGY_TRACK)*/) {
                    u.stunned = 1.0f/u.attack_rate; // only stun non-player units; players are expected to actually manage swarm vs distance
                }
                else
                    u.stunned = 0;
                if(best_found_via_tracking && (!u.popup && GetRandomValue(0, 100)<20)) {
                    u.popup = "tracker";
                    u.popup_texture = &tex::track;
                }
            }
        }
    }
    // if we are going to attack but are still rotating
    else if(u.attack_x==0 && u.attack_y==0 && u.attack_target_x && u.attack_target_y) {
        float dx = u.attack_target_x - u.x;
        float dy = u.attack_target_y - u.y;
        float targetAngle = atan2f(dy, dx) * RAD2DEG;

        // normalize angles into [-180, +180]
        float diff = targetAngle - u.angle;
        if(u.texture==&tex::fort) diff = 0;
        while (diff > 180.0f) diff -= 360.0f;
        while (diff < -180.0f) diff += 360.0f;


        // if not facing target, rotate toward it
        if (fabs(diff) > AIM_THRESHOLD) {
            float rot = TURN_RATE * dt * (u_speed?u_speed:1.f);
            if(u.texture==&tex::human) rot *= 3.f; // humans turn very fast
            if(is_mecha(u) && (u.faction->technology&TECHNOLOGY_DRIVER)) rot *= 2.f;
            if(u.faction && (u.faction->technology&TECHNOLOGY_SPEEDY)) rot *= 3.f; // even faster turning for speedy
            if (diff > 0) {
                u.angle += rot;
                if(diff-rot<0) u.angle = targetAngle;
            }
            else {
                u.angle -= rot;
                if(diff+rot>0) u.angle = targetAngle;
            }
            continue;
        }
        float rad = u.size*TILE_SIZE;//(u.size+u.extra_scale) * TILE_SIZE;   // same radius as the drawn circle, NOT the image that may be larger

        float ang = (u.texture==&tex::fort?targetAngle:u.angle) * DEG2RAD;
        u.attack_x = u.x + cosf(ang) * (rad / TILE_SIZE);
        u.attack_y = u.y + sinf(ang) * (rad / TILE_SIZE);
        u.stunned = 0.1/u.attack_rate;
        int ux = (int)(u.x+0.5f);
        int uy = (int)(u.y+0.5f);
        if(visible[uy][ux]) {
            int range = (int)u.size + 1;
            if (ux >= xMin-range && ux < xMax+range && uy >= yMin-range && uy < yMax+range) {
                if(u.texture==&tex::bison) sound::moo.Play(camera.zoom*0.5f);
                else if(u.texture==&tex::wolf || u.texture==&tex::snowman) sound::damage.Play(camera.zoom*0.4f);
                else if(u.texture==&tex::tank) sound::boom.Play(camera.zoom*0.9f);
                else if(u.texture==&tex::esper) sound::ohm.Play(camera.zoom*0.9f);
                else sound::gun.Play(camera.zoom*0.2f);
            }
        }
        continue;
    }
    // TODO: perhaps heal only if not moving, but for now this is hard to properly check
    if (u.health < u.max_health && (!is_mecha(u) || (u.faction && (u.faction->technology & TECHNOLOGY_AUTOREPAIRS)))) {
        if ((float)GetRandomValue(0, 1000000) / 1000000.0f < dt*0.5f && (is_mecha(u) || u.faction==nullptr || !(u.faction->technology & TECHNOLOGY_NUCLEAR))) {
            u.health += 1.0f;
            if (u.health > u.max_health)
                u.health = u.max_health;
        }
    }
    if((u.attack_x || u.attack_y) && (u.x-u.target_x)*(u.x-u.target_x)+(u.y-u.target_y)*(u.y-u.target_y) < u_range*u_range/3)
        continue;

    if(u.target_x==0 && u.target_y==0)
        continue;
    // --- ROTATE TOWARD MOVEMENT DIRECTION BEFORE MOVING ---
    float dx = u.target_x - u.x;
    float dy = u.target_y - u.y;
    float dist2 = dx*dx + dy*dy;

    // reached destination?
    if (dist2 < 0.1f) {
        u.target_x = 0;
        u.target_y = 0;
        continue;
    }

    float dist = sqrtf(dist2);
    float desiredAngle = atan2f(dy, dx) * RAD2DEG;

    if(u.texture==&tex::human && !u.stunned) {
        desiredAngle += cos(t*10+u.speed*3.14159)*20;
    }

    // compute smallest signed difference
    float diff = desiredAngle - u.angle;
    while (diff > 180.0f) diff -= 360.0f;
    while (diff < -180.0f) diff += 360.0f;

    // rotate until close enough
    if (fabs(diff) > AIM_THRESHOLD) {
        float out_of_threshold = fabs(diff) > 20;
        float rot = TURN_RATE * dt * u_speed * 2;
        if(u.texture==&tex::human) rot *= 3.f; // humans turn very fast
        if(is_mecha(u) && (u.faction->technology&TECHNOLOGY_DRIVER)) rot *= 2.f;
        if(u.faction && (u.faction->technology&TECHNOLOGY_SPEEDY)) rot *= 3.f; // even faster turning for speedy
        if (diff>0) {
            u.angle += rot;
            if(diff-rot<0) u.angle = desiredAngle;
        }
        else {
            u.angle -= rot;
            if(diff+rot>0) u.angle = desiredAngle;
        }
        if(out_of_threshold) continue;
    }
    if(u.stunned>0) {
        if(u.attack_target_x!=0 || u.attack_target_y!=0) continue;
        u.stunned -= dt;
        if(u.stunned<0)
            u.stunned = 0;
        continue;
        if(u.faction!=factions) continue; // non-player factions stay and fight
    }

    float step = u_speed * dt * movement_speed_multiplier;
    if(u.faction && (u.faction->technology & TECHNOLOGY_HYPERMAGNET)) step *= 2;
    u.x += (dx / dist) * step;
    u.y += (dy / dist) * step;
}

//
for(int i=0;i<max_factions;i++) {
    if(factions[i].technology & TECHNOLOGY_TECHNOCRACY) {
        factions[i].victory_points += factions[i].industry*0.01f;
        //factions[i].industry *= 0.5f;
    }
    if(factions[i].technology & TECHNOLOGY_SNIFFING) factions[i].victory_points += 1;
    if(factions[i].victory_points<0) factions[i].victory_points = 0;
    game_time += dt*0.08f*factions[i].industry/30.f/max_factions;
    polution_speedup += 0.08f*factions[i].industry/30.f/max_factions;
    game_time += dt*0.08f*factions[i].count_members/30.f/max_factions;
    polution_speedup += 0.08f*factions[i].count_members/30.f/max_factions;
}

// repulse units (Collisions)
const float stiffness = 1.0f;   // tune this
const float radiusFactor = 0.7f;
for (int i = 0; i < num_units; i++) {
    Unit &u = units[i];
    if (!u.max_health) continue;
    if (u.texture==&tex::field) continue;
    if (u.texture==&tex::field_empty) continue;
    if (u.texture==&tex::field_little) continue;
    if (u.texture==&tex::hide) continue;

    for (int j = i + 1; j < num_units; j++) {
        Unit &o = units[j];
        if (!o.max_health) continue;
        if (o.texture==&tex::field) continue;
        if (o.texture==&tex::field_little) continue;
        if (o.texture==&tex::field_empty) continue;
        if (o.texture==&tex::hide) continue;

        float dx = u.x - o.x;
        float dy = u.y - o.y;
        float d2 = dx*dx + dy*dy;

        float minDist = (u.size + o.size) * radiusFactor;
        float minDist2 = minDist * minDist;

        if (d2 < minDist2 && d2 > 0.0001f) {
            float d = sqrtf(d2);
            float overlap = minDist - d;
            float force = overlap * stiffness;
            float nx = dx / d;
            float ny = dy / d;

            bool u_movable = (u.speed > 0.0f) || u.texture==&tex::rock;
            bool o_movable = (o.speed > 0.0f) || o.texture==&tex::rock;
            float u_force = u.texture==&tex::rock?0.02f:0.5f;
            float o_force = o.texture==&tex::rock?0.02f:0.5f;

            if (u_movable && o_movable) {
                // Both move equally
                u.x += nx * force * u_force;
                u.y += ny * force * u_force;
                o.x -= nx * force * o_force;
                o.y -= ny * force * o_force;
            }
            else if (u_movable && !o_movable) {
                // Only u moves
                u.x += nx * force;
                u.y += ny * force;
            }
            else if (!u_movable && o_movable) {
                // Only o moves
                o.x -= nx * force;
                o.y -= ny * force;
            }
            // else: both immovable → do nothing
        }
    }
}