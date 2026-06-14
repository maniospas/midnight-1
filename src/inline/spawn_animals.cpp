
// spawn units
if (GetRandomValue(0, 1000000) < (int)(ANIMAL_SPAWN_RATE * dt * 1000000.0f)) {
    for (int k = 0; k < 4; k++) { // small burst
        float x = GetRandomValue(10, GRID_SIZE - 10);
        float y = GetRandomValue(10, GRID_SIZE - 10);
        if (tooCloseToAnyCamp(x, y)) continue;
        Terrain &T = terrainGrid[(int)y][(int)x];
        if(GetRandomValue(0,1000)<10 && time_norm>0.3) {
            CREATE_ESPER(ANIMAL_FACTION, x, y); // 2 every 250 seconds, but only after 33% of progress - they balance the late game by giving opportunities to players to come back
            RevealUnitToAllFactions(num_units - 1);
            last_message_counter = 0.f;
            last_message = "An esper broadcasts their brainwaves!";
        }
        else if(T.texture == &tex::grass) {
            if(GetRandomValue(0,100)<50) {
                CREATE_BISON(ANIMAL_FACTION, x, y);
            }
            else {
                CREATE_WOLF(ANIMAL_FACTION, x, y);
            }
        }
        else if(T.texture == &tex::hill) {
            CREATE_RAT(ANIMAL_FACTION, x-0.2, y+0.2);
            CREATE_RAT(ANIMAL_FACTION, x-0.2, y-0.2);
            CREATE_RAT(ANIMAL_FACTION, x+0.2, y+0.2);
            CREATE_RAT(ANIMAL_FACTION, x+0.2, y-0.2);
            CREATE_RAT(ANIMAL_FACTION, x, y);
        }
        else if(T.texture == &tex::mountain) {
            CREATE_SNOWMAN(ANIMAL_FACTION, x, y);
        }
    }
}