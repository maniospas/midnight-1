if(!showTechTree && (factions->technology&TECHNOLOGY_DISMANTLE) && !mouseCapturedByUI && (CheckCollisionPointRec(GetMousePosition(), dismantleBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) && engine_creation_num) {
    mouseCapturedByUI = true;
    for (int i = 0; i < num_units; i++) {
        Unit &u = units[i];
        if (u.selected && u.health && u.speed && u.health>=u.max_health-0.5 && is_mecha(u)) {
            u.texture = &tex::ghost; // prevent explosion
            CREATE_ENGINE(factions, u.x, u.y);
            units[num_units-1].health = units[num_units-1].max_health*u.health/u.max_health;
            units[num_units-1].popup = "dismantle";
            units[num_units-1].capturing = nullptr;
            u.health = 0;
        }
    }
    PlaySound(sound::select2);
    engine_creation_num = 0;
}

if(!showTechTree && (factions->technology&TECHNOLOGY_TRENCHES) && !mouseCapturedByUI && (CheckCollisionPointRec(GetMousePosition(), trenchBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) && trench_creation_num) {
    mouseCapturedByUI = true;
    for (int i = 0; i < num_units; i++) {
        Unit &u = units[i];
        if (u.selected && u.health && (u.texture==&tex::tank || u.texture==&tex::van)) {
            u.texture = &tex::ghost; // prevent explosion
            CREATE_RAILGUN(factions, u.x, u.y);
            units[num_units-1].health = units[num_units-1].max_health*u.health/u.max_health;
            units[num_units-1].popup = "entrenched";
            units[num_units-1].capturing = nullptr;
            u.health = 0;
        }
    }
    PlaySound(sound::select2);
    trench_creation_num = 0;
}

if(!showTechTree && (factions->technology&TECHNOLOGY_TURTLING) && !mouseCapturedByUI && (CheckCollisionPointRec(GetMousePosition(), turtleBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) && turtle_creation_num) {
    mouseCapturedByUI = true;
    for (int i = 0; i < num_units; i++) {
        Unit &u = units[i];
        if (u.selected && u.health>3.1f && u.speed && !is_mecha(u)) {
            u.health -= 3;
            u.max_health -= 3;
            if(true) {
                float x = u.x + cos(u.angle*DEG2RAD)*u.size;
                float y = u.y + sin(u.angle*DEG2RAD)*u.size;
                CREATE_ROCK(u.faction, x, y);
                // units[num_units-1].popup = "turtling";
                // units[num_units-1].capturing = nullptr;
            }
            else {
                u.popup = "harmed";
                u.capturing = nullptr;
            }
        }
    }
    PlaySound(sound::select2);
    turtle_creation_num = 0;
}


if(!showTechTree && (factions->technology&TECHNOLOGY_COMMAND) && !mouseCapturedByUI && (CheckCollisionPointRec(GetMousePosition(), vanBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) && fort_creation_num) {
    mouseCapturedByUI = true;
    if(fort_creation_total_health<100) {
        last_message = "Vans need human builders worth at least 20 industry";
        last_message_counter = 0.f;
    }
    else {
        for (int i = 0; i < num_units; i++) {
            Unit &u = units[i];
            if (u.selected && u.health && u.speed && (u.texture==&tex::cat || u.texture==&tex::human || u.texture==&tex::scout || u.texture==&tex::hero)) {
                u.health = 0;
                u.texture = &tex::ghost;
            }
        }
        CREATE_VAN(factions, fort_creation_px, fort_creation_py);
        fort_creation_total_health = fort_creation_total_health/100.f;
        units[num_units-1].max_health *= fort_creation_total_health;
        units[num_units-1].health *= fort_creation_total_health;
        units[num_units-1].size *= sqrtf(fort_creation_total_health);
        units[num_units-1].capturing = nullptr;
        PlaySound(sound::select2);
        last_message = "Constructed a van";
        last_message_counter = 0.f;
        fort_creation_num = 0;
    }
}

if(!showTechTree && (factions->technology&TECHNOLOGY_FORT) && !mouseCapturedByUI && (CheckCollisionPointRec(GetMousePosition(), fortBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) && fort_creation_num) {
    mouseCapturedByUI = true;
    if(fort_creation_has_nearby) {
        last_message = "Cannot build fort close to other structures";
        last_message_counter = 0.f;
    }
    else if(fort_creation_total_health<100) {
        last_message = "Forts need stationed humans worth at least 20 industry";
        last_message_counter = 0.f;
    }
    else {
        for (int i = 0; i < num_units; i++) {
            Unit &u = units[i];
            if (u.selected && u.health && u.speed && (u.texture==&tex::cat || u.texture==&tex::human || u.texture==&tex::scout || u.texture==&tex::hero)) {
                u.health = 0;
                u.texture = &tex::ghost;
            }
        }
        CREATE_FORT(factions, fort_creation_px, fort_creation_py);
        fort_creation_total_health = fort_creation_total_health/5;
        units[num_units-1].max_health = fort_creation_total_health;
        units[num_units-1].health = fort_creation_total_health;
        units[num_units-1].size = sqrtf(fort_creation_total_health/10);
        PlaySound(sound::select2);
        last_message = "Built a fort";
        last_message_counter = 0.f;
        fort_creation_num = 0;
    }
}