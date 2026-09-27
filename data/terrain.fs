#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
uniform float time;
uniform float flowAngle;
uniform vec2 cameraTarget;
uniform vec2 cameraOffset;
uniform float cameraZoom;
uniform vec2 screenSize;
out vec4 finalColor;
const vec3 HILL = vec3(164.0, 107.0, 53.0) / 255.0;
const vec3 MOUNTAIN = vec3(139.0, 69.0, 19.0) / 255.0;
const vec3 HILL_DARK = vec3(82.0, 49.0, 10.0) / 255.0;
// ============================================================
// NOISE
// ============================================================
float hash21(vec2 p)
{
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}
float noise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float a = hash21(i);
    float b = hash21(i + vec2(1.0, 0.0));
    float c = hash21(i + vec2(0.0, 1.0));
    float d = hash21(i + vec2(1.0, 1.0));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}
// ============================================================
// TERRAIN MASKS
// ============================================================
float waterMask(vec3 c)
{
    float blue = c.b - max(c.r, c.g);
    return smoothstep(0.08, 0.25, blue) * smoothstep(0.25, 0.55, c.b);
}
float desertMask(vec3 c)
{
    float yellow = min(c.r, c.g);
    float separation = yellow - c.b;
    return smoothstep(0.30, 0.65, separation)
         * smoothstep(0.55, 0.80, yellow);
}
float grassMask(vec3 c)
{
    float green = c.g - max(c.r, c.b);
    return smoothstep(0.12, 0.38, green);
}
float paletteMask(vec3 c, vec3 target, float radius)
{
    return 1.0 - smoothstep(radius * 0.25, radius, distance(c, target));
}
// ============================================================
// MAIN
// ============================================================
void main()
{
    vec4 texel = texture(texture0, fragTexCoord);
    if (texel.a <= 0.001)
    {
        finalColor = texel;
        return;
    }
    vec3 original = texel.rgb;
    vec3 col = original;
    // ========================================================
    // WORLD POSITION
    // ========================================================
    vec2 screen = gl_FragCoord.xy;
    screen.y = screenSize.y - screen.y;
    vec2 world = (screen - cameraOffset) / cameraZoom + cameraTarget;
    vec2 wp = world / 128.0;
    // ========================================================
    // TERRAIN CLASSIFICATION
    // ========================================================
    float water = waterMask(original);
    float grass = grassMask(original) * (1.0 - water);
    float mountain = paletteMask(original, MOUNTAIN, 0.12);
    mountain *= 1.0 - water;
    float hill = paletteMask(original, HILL, 0.15);
    hill *= 1.0 - water;
    hill *= 1.0 - mountain;
    float hillDark = paletteMask(original, HILL_DARK, 0.11);
    hillDark *= 1.0 - water;
    float desert = desertMask(original);
    desert *= 1.0 - water;
    desert *= 1.0 - grass;
    desert *= 1.0 - hill;
    desert *= 1.0 - mountain;
    desert *= 1.0 - hillDark;
    // ========================================================
    // GRASS
    //
    // Moderately visible broad variation.
    // No fine smudge layer.
    // ========================================================
    float grassLarge = noise(wp * 3.5);
    float grassMedium = noise(wp * 7.0 + vec2(17.0, 31.0));
    float grassNoise = grassLarge * 0.70;
    grassNoise += grassMedium * 0.30;
    vec3 grassDark = col * 0.90;
    vec3 grassLight = min(col * 1.075, vec3(1.0));
    vec3 variedGrass = mix(grassDark, grassLight, grassNoise);
    col = mix(col, variedGrass, grass * 0.34);
    // ========================================================
    // GRASS FLOWERS
    //
    // Very rare, larger flowers.
    // ========================================================
    vec2 flowerGrid = wp * 5.0;
    vec2 flowerCell = floor(flowerGrid);
    vec2 flowerUV = fract(flowerGrid);
    float flowerChance = hash21(flowerCell);
    float flowerType = hash21(flowerCell + vec2(91.7, 37.1));
    vec2 flowerCenter = vec2(hash21(flowerCell + 17.3), hash21(flowerCell + 53.8));
    flowerCenter = mix(vec2(0.25), vec2(0.75), flowerCenter);
    float flowerDist = distance(flowerUV, flowerCenter);
    float flowerExists = step(0.995, flowerChance);
    float flowerShape = 1.0 - smoothstep(0.045, 0.13, flowerDist);
    float flower = flowerExists;
    flower *= flowerShape;
    flower *= grass;
    vec3 flowerColor = vec3(0.98, 0.94, 0.72);
    if (flowerType > 0.55)
        flowerColor = vec3(1.00, 0.80, 0.12);
    if (flowerType > 0.82)
        flowerColor = vec3(0.62, 0.76, 1.00);
    if (flowerType > 0.95)
        flowerColor = vec3(1.00, 0.58, 0.75);
    col = mix(col, flowerColor, flower * 0.92);
    // ========================================================
// DESERT
//
// Strong moving sand streaks + prominent twirling winds.
// ========================================================
vec2 desertWind = normalize(vec2(1.0, 0.20));
vec2 desertSide = vec2(-desertWind.y, desertWind.x);
float alongDesert = dot(wp, desertWind);
float acrossDesert = dot(wp, desertSide);
// --------------------------------------------------------
// MOVING SAND STREAKS
// --------------------------------------------------------
float sandBand = noise(vec2(alongDesert * 1.8 - time * 0.28, acrossDesert * 6.0));
float sandBreak = noise(vec2(alongDesert * 5.0 - time * 0.19, acrossDesert * 2.5) + vec2(43.0, 91.0));
float sandStreak = smoothstep(0.72, 0.91, sandBand);
sandStreak *= smoothstep(0.48, 0.78, sandBreak);
vec3 sandHighlight = vec3(1.00, 0.88, 0.24);
col = mix(col, sandHighlight, sandStreak * desert * 0.48);
// ========================================================
// LARGE TWIRLING DUST WINDS
// ========================================================
float vortexDesert = smoothstep(0.45, 0.75, desert);
// Slowly move the vortex field across the world.
vec2 vortexDrift = desertWind * time * 0.025;
vec2 vortexPos = (wp - vortexDrift) * 0.27;
vec2 vortexCell = floor(vortexPos);
vec2 vortexUV = fract(vortexPos);
    float vortexChance = hash21( vortexCell + vec2(183.7, 41.3) );
// More common than before because the cells themselves
// are now considerably larger.
    float vortexExists = step(0.992, vortexChance);
    vec2 vortexCenter = vec2( hash21(vortexCell + 17.1), hash21(vortexCell + 79.4) );
    vortexCenter = mix( vec2(0.30), vec2(0.70), vortexCenter );
vec2 vortexDelta = vortexUV - vortexCenter;
float vortexRadius = length(vortexDelta);
    float vortexAngle = atan( vortexDelta.y, vortexDelta.x );
// --------------------------------------------------------
// ROTATION
// --------------------------------------------------------
    float spinSeed = hash21( vortexCell + 317.2 );
    float spinDirection = step( 0.5, spinSeed );
    spinDirection = mix( -1.0, 1.0, spinDirection );
// --------------------------------------------------------
// PRIMARY SPIRAL
//
// Fewer turns and thicker arms make the rotation much
// easier to actually see.
// --------------------------------------------------------
float spiralPhase = vortexAngle * 2.0;
spiralPhase += vortexRadius * 18.0;
spiralPhase -= time * 0.75 * spinDirection;
    float spiralWave = sin( spiralPhase );
    float spiral = smoothstep( 0.15, 0.82, spiralWave );
// --------------------------------------------------------
// LARGE RADIAL ENVELOPE
// --------------------------------------------------------
    float vortexInner = smoothstep( 0.025, 0.075, vortexRadius );
    float vortexOuter = 1.0 - smoothstep( 0.28, 0.52, vortexRadius );
float vortexShape = vortexInner * vortexOuter;
// --------------------------------------------------------
// IRREGULARITY
//
// Unlike before, noise doesn't erase most of the spiral.
// It only changes its strength.
// --------------------------------------------------------
    float vortexNoise = noise( vortexCell * 3.0 + vortexDelta * 8.0 + vec2(time * 0.025, -time * 0.016) );
    float vortexBreak = mix( 0.58, 1.0, vortexNoise );
float vortex = spiral;
vortex *= vortexShape;
vortex *= vortexBreak;
vortex *= vortexExists;
vortex *= vortexDesert;
// --------------------------------------------------------
// SECOND SPIRAL ARM
//
// Offset from the main arm so the vortex has a fuller,
// more obvious swirling structure.
// --------------------------------------------------------
float secondPhase = vortexAngle * 2.0;
secondPhase += vortexRadius * 18.0;
secondPhase -= time * 0.75 * spinDirection;
secondPhase += 3.14159265;
    float secondWave = sin( secondPhase );
    float secondSpiral = smoothstep( 0.30, 0.88, secondWave );
secondSpiral *= vortexShape;
secondSpiral *= vortexBreak;
secondSpiral *= vortexExists;
secondSpiral *= vortexDesert;
// --------------------------------------------------------
// BRIGHT INNER CURL
// --------------------------------------------------------
float innerPhase = vortexAngle * 3.0;
innerPhase += vortexRadius * 25.0;
innerPhase -= time * 1.05 * spinDirection;
innerPhase += 1.2;
    float innerSpiral = sin( innerPhase );
    innerSpiral = smoothstep( 0.48, 0.91, innerSpiral );
    float innerShape = 1.0 - smoothstep( 0.10, 0.34, vortexRadius );
innerSpiral *= innerShape;
innerSpiral *= vortexExists;
innerSpiral *= vortexDesert;
// --------------------------------------------------------
// SOFT DUST CLOUD
//
// Makes the whole vortex region visible rather than only
// showing mathematically thin spiral lines.
// --------------------------------------------------------
    float dustCloud = 1.0 - smoothstep( 0.08, 0.48, vortexRadius );
dustCloud *= vortexExists;
dustCloud *= vortexDesert;
    float cloudNoise = noise( vortexDelta * 5.0 + vortexCell * 7.0 + vec2(time * 0.018, 0.0) );
    dustCloud *= mix( 0.35, 0.75, cloudNoise );
// --------------------------------------------------------
// COLORS
// --------------------------------------------------------
    vec3 dustCloudColor = vec3( 0.93, 0.70, 0.18 );
    vec3 vortexDust = vec3( 1.00, 0.84, 0.22 );
    vec3 vortexBright = vec3( 1.00, 0.96, 0.62 );
// --------------------------------------------------------
// COMPOSITE
// --------------------------------------------------------
    col = mix( col, dustCloudColor, dustCloud * 0.24 );
    col = mix( col, vortexDust, vortex * 0.72 );
    col = mix( col, vortexDust, secondSpiral * 0.48 );
    col = mix( col, vortexBright, innerSpiral * 0.62 );
    // ========================================================
    // HILL BASE
    //
    // Almost completely clean.
    // The rocks provide nearly all of the visual detail.
    // ========================================================
    float hillGround = noise(wp * 2.5 + vec2(17.0, 31.0));
    hillGround = (hillGround - 0.5) * 0.025;
    col *= 1.0 + hillGround * hill;
    // ========================================================
    // MOUNTAINS
    //
    // Darker mountain terrain retains its rougher treatment.
    // ========================================================
    float rockLarge = noise( wp * 8.0 + vec2(37.0, 11.0) );
    float rockMedium = noise( wp * 23.0 + vec2(91.0, 53.0) );
    float rockFine = noise( wp * 57.0 + vec2(14.0, 119.0) );
    float mountainRock = (rockLarge - 0.5) * 0.20;
    mountainRock += (rockMedium - 0.5) * 0.15;
    mountainRock += (rockFine - 0.5) * 0.08;
    col *= 1.0 + mountainRock * mountain;
    // --------------------------------------------------------
    // MOUNTAIN CRACKS
    // --------------------------------------------------------
    float crackNoise = noise( wp * vec2(18.0, 45.0) + vec2(43.0, 7.0) );
    float cracks = smoothstep( 0.76, 0.88, crackNoise );
    cracks *= smoothstep( 0.35, 0.70, noise(wp * 9.0 + vec2(81.0, 21.0)) );
    vec3 crackColor = vec3( 91.0, 42.0, 10.0 ) / 255.0;
    col = mix( col, crackColor, cracks * mountain * 0.38 );
    // --------------------------------------------------------
    // MOUNTAIN ROCK HIGHLIGHTS
    // --------------------------------------------------------
    float rockSpecks = noise( wp * 68.0 + vec2(111.0, 47.0) );
    rockSpecks = smoothstep( 0.84, 0.96, rockSpecks );
    vec3 rockHighlight = vec3( 174.0, 100.0, 42.0 ) / 255.0;
    col = mix( col, rockHighlight, rockSpecks * mountain * 0.28 );
    // ========================================================
    // DARK TERRAIN EDGES
    // ========================================================
    float darkEdgeNoise = noise( wp * 22.0 + vec2(19.0, 61.0) );
    float darkVariation = (darkEdgeNoise - 0.5) * 0.13;
    col *= 1.0 + darkVariation * hillDark;
    // ========================================================
    // WATER
    //
    // The complete procedural field moves in flowAngle.
    // ========================================================
    if (water > 0.001)
    {
    vec2 flowDir = vec2( cos(flowAngle), sin(flowAngle) );
    vec2 sideDir = vec2( -flowDir.y, flowDir.x );
        const float WATER_SPEED = 1.45;
        vec2 movingWP = wp;
        movingWP -= flowDir * time * WATER_SPEED;
    float along = dot( movingWP, flowDir );
    float across = dot( movingWP, sideDir );
        // ----------------------------------------------------
        // WATER BODY
        // ----------------------------------------------------
    float body1 = noise( vec2( along * 2.1, across * 3.0 ) );
    float body2 = noise( vec2( along * 4.3, across * 5.4 ) + vec2(37.0, 71.0) );
    float body3 = noise( vec2( along * 7.5, across * 2.1 ) + vec2(91.0, 23.0) );
        float body = body1 * 0.52;
        body += body2 * 0.31;
        body += body3 * 0.17;
    vec3 deepWater = vec3( 0.055, 0.17, 0.70 );
    vec3 lightWater = vec3( 0.075, 0.34, 0.86 );
    vec3 movingWater = mix( deepWater, lightWater, body );
    vec3 waterCol = mix( original, movingWater, 0.52 );
        // ----------------------------------------------------
        // LONG CURRENT STREAKS
        // ----------------------------------------------------
    float streakField = noise( vec2( along * 1.15, across * 11.0 ) + vec2(19.0, 43.0) );
    float streakBreak = noise( vec2( along * 4.5, across * 2.3 ) + vec2(83.0, 17.0) );
    float streak = smoothstep( 0.66, 0.84, streakField );
    streak *= smoothstep( 0.38, 0.72, streakBreak );
    vec3 waterHighlight = vec3( 0.11, 0.49, 0.94 );
    waterCol = mix( waterCol, waterHighlight, streak * 0.38 );
        // ----------------------------------------------------
        // FAST SMALL RIPPLES
        // ----------------------------------------------------
        vec2 fastWP = wp;
        fastWP -= flowDir * time * WATER_SPEED * 1.65;
    float rippleAlong = dot( fastWP, flowDir );
    float rippleAcross = dot( fastWP, sideDir );
    float ripple = noise( vec2( rippleAlong * 7.0, rippleAcross * 18.0 ) + vec2(127.0, 53.0) );
    ripple = smoothstep( 0.78, 0.94, ripple );
    waterCol = mix( waterCol, vec3(0.16, 0.58, 0.98), ripple * 0.21 );
        // ----------------------------------------------------
        // WAVE CRESTS
        // ----------------------------------------------------
        float wavePhase = along * 10.0;
    wavePhase += noise( vec2( along * 1.4, across * 3.0 ) ) * 3.0;
        float wave = sin(wavePhase);
    float crest = smoothstep( 0.82, 1.0, wave );
    crest *= smoothstep( 0.30, 0.72, noise( vec2( along * 4.0, across * 5.0 ) + vec2(47.0, 113.0) ) );
    waterCol = mix( waterCol, vec3(0.20, 0.62, 1.00), crest * 0.12 );
    col = mix( col, waterCol, water );
    }
    // ========================================================
    // OUTPUT
    // ========================================================
    finalColor = vec4( clamp(col, 0.0, 1.0), texel.a ) * fragColor;
}
