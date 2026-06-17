// AI FOR TECH TREE
for(int i=3;i<max_factions;++i) {
    Faction &F = factions[i];
    if(F.technology_progress < 1.f) continue;
    unsigned long long prev = F.technology;
    unsigned long long chosen = 0;
    int k = GetRandomValue(0, 63);
    unsigned long long candidate = 1ULL << k;
    if(prev & candidate) continue;

    // lower chance of just restarting the tree
    if (candidate == TECHNOLOGY_EXPLORE) chosen = candidate;
    else if (candidate == TECHNOLOGY_HUNTING) chosen = candidate;
    else if (candidate == TECHNOLOGY_NERDS) chosen = candidate;
    else if (candidate == TECHNOLOGY_TAMING) chosen = candidate;
    else if (candidate == TECHNOLOGY_HARDCORE) chosen = candidate;
    else if (candidate == TECHNOLOGY_SCAVENGE) chosen = candidate;
    else if (candidate == TECHNOLOGY_COMMAND) chosen = candidate;
    else if (candidate == TECHNOLOGY_TRENCHES && (prev & TECHNOLOGY_SCAVENGE)) chosen = candidate;
    else if (candidate == TECHNOLOGY_DISMANTLE && (prev & TECHNOLOGY_NERDS)) chosen = candidate;
    else if (candidate == TECHNOLOGY_VROOM && (prev & TECHNOLOGY_DISMANTLE)) chosen = candidate;
    else if (candidate == TECHNOLOGY_TRACK && (prev & TECHNOLOGY_EXPLORE)) chosen = candidate;
    else if (candidate == TECHNOLOGY_AGILE && (prev & TECHNOLOGY_EXPLORE)) chosen = candidate;
    else if (candidate == TECHNOLOGY_DRIVER && (prev & TECHNOLOGY_EXPLORE)) chosen = candidate;
    else if (candidate == TECHNOLOGY_WONDER && (prev & (TECHNOLOGY_AGILE | TECHNOLOGY_TAMING))) chosen = candidate;
    else if (candidate == TECHNOLOGY_FIGHT && (prev & TECHNOLOGY_HARDCORE)) chosen = candidate;
    else if (candidate == TECHNOLOGY_FARMING && (prev & TECHNOLOGY_HUNTING)) chosen = candidate;
    else if (candidate == TECHNOLOGY_FORT && (prev & TECHNOLOGY_HUNTING)) chosen = candidate;
    else if (candidate == TECHNOLOGY_CENTRAL && (prev & TECHNOLOGY_FORT)) chosen = candidate;
    else if (candidate == TECHNOLOGY_RESEARCH && (prev & TECHNOLOGY_NERDS)) chosen = candidate;
    else if (candidate == TECHNOLOGY_HOMUNCULI && (prev & TECHNOLOGY_NERDS)) chosen = candidate;
    else if (candidate == TECHNOLOGY_MECHA && (prev & TECHNOLOGY_NERDS)) chosen = candidate;
    else if (candidate == TECHNOLOGY_SPEEDY && (prev & TECHNOLOGY_HELLBRINGER)) chosen = candidate;
    else if (candidate == TECHNOLOGY_AUTOREPAIRS && (prev & TECHNOLOGY_MECHA)) chosen = candidate;
    else if (candidate == TECHNOLOGY_MOBILE_FORTRESS && (prev & TECHNOLOGY_AUTOREPAIRS)) chosen = candidate;
    else if (candidate == TECHNOLOGY_REVUP && (prev & TECHNOLOGY_MOBILE_FORTRESS)) chosen = candidate;
    else if (candidate == TECHNOLOGY_HIJACK && (prev & (TECHNOLOGY_AUTOREPAIRS | TECHNOLOGY_VROOM))) chosen = candidate;
    else if (candidate == TECHNOLOGY_HEROICS && (prev & (TECHNOLOGY_FIGHT | TECHNOLOGY_TAMING))) chosen = candidate;
    else if (candidate == TECHNOLOGY_GRIT && (prev & (TECHNOLOGY_HUNTING | TECHNOLOGY_SCAVENGE))) chosen = candidate;
    else if (candidate == TECHNOLOGY_ANTIMECHA && (prev & (TECHNOLOGY_GRIT | TECHNOLOGY_TOUGH | TECHNOLOGY_HIJACK))) chosen = candidate;
    else if (candidate == TECHNOLOGY_FLANKING && (prev & (TECHNOLOGY_ANTIMECHA))) chosen = candidate;
    else if (candidate == TECHNOLOGY_TOUGH && (prev & TECHNOLOGY_FIGHT)) chosen = candidate;
    else if (candidate == TECHNOLOGY_HELLBRINGER && (prev & TECHNOLOGY_HEROICS)) chosen = candidate;
    else if (candidate == TECHNOLOGY_SEAFARERING && (prev & TECHNOLOGY_AGILE)) chosen = candidate;
    else if (candidate == TECHNOLOGY_SNIPING && (prev & (TECHNOLOGY_TRACK | TECHNOLOGY_TRENCHES))) chosen = candidate;
    else if (candidate == TECHNOLOGY_TURTLING && (prev & TECHNOLOGY_SNIPING)) chosen = candidate;
    else if (candidate == TECHNOLOGY_SNIFFING && (prev & TECHNOLOGY_TRACK)) chosen = candidate;
    else if (candidate == TECHNOLOGY_INFRASTRUCTURE && (prev & (TECHNOLOGY_FARMING | TECHNOLOGY_RESEARCH))) chosen = candidate;
    else if (candidate == TECHNOLOGY_OWNERSHIP && (prev & (TECHNOLOGY_SEAFARERING | TECHNOLOGY_INFRASTRUCTURE))) chosen = candidate;
    else if (candidate == TECHNOLOGY_LUXURY && (prev & (TECHNOLOGY_OWNERSHIP | TECHNOLOGY_WONDER))) chosen = candidate;
    else if (candidate == TECHNOLOGY_PROPAGANDA && (prev & (TECHNOLOGY_OWNERSHIP | TECHNOLOGY_HEROICS))) chosen = candidate;
    else if (candidate == TECHNOLOGY_SUPERIORITY && (prev & TECHNOLOGY_PROPAGANDA)) chosen = candidate;
    else if (candidate == TECHNOLOGY_INDUSTRY && (prev & TECHNOLOGY_DRIVER)) chosen = candidate;
    else if (candidate == TECHNOLOGY_GIGAJOULE && (prev & TECHNOLOGY_INDUSTRY)) chosen = candidate;
    else if (candidate == TECHNOLOGY_MECHANISED && (prev & TECHNOLOGY_GIGAJOULE)) chosen = candidate;
    else if (candidate == TECHNOLOGY_TERRAFORIMING && (prev & TECHNOLOGY_GIGAJOULE)) chosen = candidate;
    else if (candidate == TECHNOLOGY_ATMOSPHERE && (prev & TECHNOLOGY_TERRAFORIMING)) chosen = candidate;
    else if (candidate == TECHNOLOGY_UNSTABLE && (prev & TECHNOLOGY_HOMUNCULI)) chosen = candidate;
    else if (candidate == TECHNOLOGY_BIOWEAPON && (prev & TECHNOLOGY_UNSTABLE)) chosen = candidate;
    else if (candidate == TECHNOLOGY_EVOLUTION && (prev & (TECHNOLOGY_BIOWEAPON | TECHNOLOGY_GRIT))) chosen = candidate;
    else if (candidate == TECHNOLOGY_ARTIFICIAL && (prev & TECHNOLOGY_BIOWEAPON)) chosen = candidate;
    else if (candidate == TECHNOLOGY_NUCLEAR && (prev & TECHNOLOGY_RESEARCH)) chosen = candidate;
    else if (candidate == TECHNOLOGY_REACTOR && (prev & TECHNOLOGY_NUCLEAR)) chosen = candidate;
    else if (candidate == TECHNOLOGY_HYPERMAGNET && (prev & TECHNOLOGY_NUCLEAR)) chosen = candidate;
    else if (candidate == TECHNOLOGY_AIFARM && (prev & TECHNOLOGY_INFRASTRUCTURE)) chosen = candidate;
    else if (candidate == TECHNOLOGY_REFINERY && (prev & (TECHNOLOGY_OWNERSHIP | TECHNOLOGY_GIGAJOULE))) chosen = candidate;


    if (chosen) {
        int accept_roll = GetRandomValue(0, 99);
        bool rejected = false;
        if (chosen & (TECHNOLOGY_EXPLORE | TECHNOLOGY_HUNTING | TECHNOLOGY_NERDS |
                      TECHNOLOGY_TAMING | TECHNOLOGY_HARDCORE | TECHNOLOGY_SCAVENGE | TECHNOLOGY_COMMAND)) {
            if (accept_roll >= 12) rejected = true;
            if(!(chosen&global_available_starting_techs)) rejected = true;
        }
        else if (chosen & (TECHNOLOGY_TRACK | TECHNOLOGY_AGILE | TECHNOLOGY_DRIVER |
                           TECHNOLOGY_FIGHT | TECHNOLOGY_FARMING | TECHNOLOGY_FORT |
                           TECHNOLOGY_GRIT | TECHNOLOGY_RESEARCH | TECHNOLOGY_HOMUNCULI |
                           TECHNOLOGY_MECHA | TECHNOLOGY_DISMANTLE | TECHNOLOGY_TRENCHES)) {
            if (accept_roll >= 25) rejected = true;
        }
        else if (chosen & (TECHNOLOGY_WONDER | TECHNOLOGY_HEROICS | TECHNOLOGY_TOUGH |
                           TECHNOLOGY_CENTRAL | TECHNOLOGY_AUTOREPAIRS | TECHNOLOGY_UNSTABLE |
                           TECHNOLOGY_NUCLEAR | TECHNOLOGY_INDUSTRY | TECHNOLOGY_SNIPING |
                           TECHNOLOGY_SNIFFING | TECHNOLOGY_SEAFARERING | TECHNOLOGY_INFRASTRUCTURE | 
                           TECHNOLOGY_TURTLING | TECHNOLOGY_VROOM)) {
            if (accept_roll >= 50) rejected = true;
        }
        else if (accept_roll >= 75) rejected = true;
        if (rejected) chosen = 0;
    }


    if (chosen) { F.technology |= candidate; F.technology_progress -= 1.f; }
}