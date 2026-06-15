
for(int i=0;i<max_factions;i++) {
    factions[i].industry = (factions[i].technology&TECHNOLOGY_REACTOR)?60:20;
    factions[i].count_members = 0;
    factions[i].victory_points = (factions[i].technology&TECHNOLOGY_SUPERIORITY)?1.f:0.f;
    factions[i].technology_progress += dt*0.009f;
    if(factions[i].technology==0 && factions[i].technology_progress<2.f) factions[i].technology_progress += dt*0.009f;
    if(factions[i].technology & TECHNOLOGY_NERDS) factions[i].technology_progress += dt*0.003f;
    if(factions[i].technology & TECHNOLOGY_TAMING) factions[i].technology_progress -= dt*0.0045f;
    if(factions[i].technology & TECHNOLOGY_RESEARCH) factions[i].technology_progress += dt*0.003f;
}
for (int i = 0; i < num_units; i++) {
    Unit &u = units[i];
    if(u.texture==&tex::lab && u.faction) {
        if(u.faction->technology & TECHNOLOGY_AIFARM) {
            u.faction->industry += 16.f;
            game_time += dt*0.08f;
            polution_speedup += 0.08f;
        }
        u.faction->technology_progress += dt*0.0009f;
    }
    if(u.texture==&tex::lighthouse && u.faction) {
        if(u.faction->technology & TECHNOLOGY_AIFARM) {
            u.faction->industry += 16.f;
            game_time += dt*0.08f;
            polution_speedup += 0.08f;
        }
        {
            u.faction->technology_progress += dt*0.0009f;
            u.faction->industry += 5.f;
        }
    }
    if(u.texture==&tex::datacenter &&  u.faction && u.faction!=factions+1 && u.faction!=ANIMAL_FACTION){
        if((float)GetRandomValue(0, 1000000) / 1000000.0f * 300.f < dt) {
            bool applied = false;
            for (int j = 0; j < num_units; j++)
                if (units[j].faction==u.faction && units[j].texture==&tex::blood) {
                    float dx = units[j].x - u.x;
                    float dy = units[j].y - u.y;
                    float d2 = dx*dx + dy*dy;
                    if(d2<25.f*25.f) {
                        units[j] = { \
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
                        units[j].popup = "radiation";
                        applied = true;
                    }
                }
            if(u.faction==factions && applied) {
                last_message = "Databank: released radiation that turned nearby blood to bloo.";
                last_message_counter = 0.f;
            }
        }
        else if((float)GetRandomValue(0, 1000000) / 1000000.0f * 300.f < dt) {
            bool applied = false;
            for (int j = 0; j < num_units; j++)
                if (units[j].faction==u.faction && units[j].texture==&tex::human && units[j].health<units[j].max_health) {
                    units[j].health += 2;
                    units[j].max_health += 2;
                    units[j].popup = "healthcare";
                    units[j].popup_texture = nullptr;
                    applied = true;
                }
            if(u.faction==factions && applied) {
                last_message = "Databank: found healthcare products and increased HP of your humans.";
                last_message_counter = 0.f;
            }
        }
        else if((float)GetRandomValue(0, 1000000) / 1000000.0f * 300.f < dt) {
            bool applied = false;
            for (int j = 0; j < num_units; j++)
                if (units[j].faction==u.faction && is_mecha(units[j]) && units[j].health<units[j].max_health) {
                    units[j].health = units[j].max_health;
                    units[j].popup = "parts";
                    units[j].popup_texture = nullptr;
                    applied = true;
                }
            if(u.faction==factions && applied) {
                last_message = "Databank: found spare parts and fixed your mechas.";
                last_message_counter = 0.f;
            }
        }
        else if((float)GetRandomValue(0, 1000000) / 1000000.0f * 5000.f < dt) {
            unsigned long long candidate = 1ULL << GetRandomValue(0, 62);
            if(!(u.faction->technology & candidate)) {
                u.faction->technology = u.faction->technology | candidate;
                u.popup = "new tech";
                u.popup_texture = nullptr;
                if(u.faction==factions) {
                    last_message = "Databank: recovered a tech";
                    last_message_counter = 0.f;
                }
            }
        }
    }
    if(u.texture==&tex::camp && u.faction && (u.faction->technology & TECHNOLOGY_HUNTING)) u.faction->industry += 3.f;
    if(u.texture==&tex::camp && u.faction && (u.faction->technology & TECHNOLOGY_HARDCORE)) u.faction->industry -= 7.f;
    if(u.texture==&tex::camp || u.speed) u.faction->count_members += 0.00001f;
    if(u.texture==&tex::oil && u.faction && (u.faction->technology & TECHNOLOGY_REFINERY)) u.faction->industry += 25.f;
    if((u.texture==&tex::field
        || (u.texture==&tex::curio && (u.faction->technology & TECHNOLOGY_CENTRAL))
        || (u.texture==&tex::fort && (u.faction->technology & TECHNOLOGY_CENTRAL))
    ) && (float)GetRandomValue(0, 1000000) / 1000000.0f*(u.faction && (u.faction->technology&TECHNOLOGY_TERRAFORIMING)?100.f:200.f)<dt) {
        // stranded fields grow once every 200 seconds, but in truth it's every 400 seconds due to two sides being occupied
        float px = u.x;
        float py = u.y;
        int r = GetRandomValue(0, 4);
        if(r==0) px += 1.4f;
        else if(r==1) px -= 1.4f;
        else if(r==2) py += 1.4f;
        else if(r==3) py -= 1.4f;
        bool allowed = true;
        for (int i = 0; i < num_units; i++) {
            Unit &v = units[i];
            if(v.health && !v.speed && (v.x-px)*(v.x-px)+(v.y-py)*(v.y-py)<1) {
                allowed = false;
                break;
            }
        }
        if(allowed) { CREATE_FIELD(&factions[1], px, py); }
    }
    if(u.texture==&tex::field || u.texture==&tex::field_little || u.texture==&tex::field_empty || u.texture==&tex::mine || u.texture==&tex::hide) {
        if((u.texture==&tex::field || u.texture==&tex::field_little || u.texture==&tex::field_empty) && u.faction && (u.faction->technology & TECHNOLOGY_ATMOSPHERE)) {
            game_time -= dt*0.02f;
            polution_speedup -= 0.02f;
        }
        if(u.texture==&tex::field) u.faction->industry += 4.f;
        if(u.texture==&tex::field_little) u.faction->industry += 2.f;
        if(u.texture==&tex::hide) u.faction->industry += 4.f;
        if(u.texture==&tex::mine) u.faction->industry += 12.f;
        if(u.texture==&tex::hide && u.faction && (u.faction->technology & TECHNOLOGY_HUNTING)) u.faction->industry += 3.f;
        continue;
    }
    if(is_mecha(u) && u.faction) {
        if(u.faction->technology & TECHNOLOGY_MECHANISED) u.faction->industry += u.health/5.f;
        if(u.faction->technology & TECHNOLOGY_GIGAJOULE) continue;
    }
    if(u.texture==&tex::fort) {
        u.faction->count_members += u.max_health/5.f;
        continue;
    }
    if(!u.faction) continue;
    if(u.capturing) continue;
    if(!u.speed) continue;
    if(u.texture!=&tex::bison && u.texture!=&tex::wolf && u.texture!=&tex::rat && u.texture!=&tex::snowman && u.texture!=&tex::roomba) { // don't count animals or roombas for industry needs'
        u.faction->count_members += u.max_health/5.f;
        if(u.faction->technology & TECHNOLOGY_HYPERMAGNET) u.faction->count_members += u.max_health/5.f;
    }
}
if(factions[0].count_members==0) {
    goto GAME_OVER;
}
for(int i=0;i<max_factions;i++) {
    if(factions[i].industry<0) factions[i].industry = 0;
}