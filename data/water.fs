#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform float time;

const float TAU = 6.28318530718;
const float PERIOD = 32.0;

/* Fully periodic wave */
float wave(vec2 uv, vec2 freq, float speed, float phase)
{
    return sin(TAU * (uv.x * freq.x + uv.y * freq.y) + time * speed + phase);
}

/* Periodic flow field (tiles every PERIOD tiles) */
vec2 flow(vec2 uv)
{
    float fx = sin(TAU * uv.y * 2.0 + time * 0.3);
    float fy = sin(TAU * uv.x * 2.0 + time * 0.4);
    return vec2(fx, fy) * 0.15;
}

void main()
{
    /* --- Macro-periodic UV ------------------------------------------ */
    vec2 tileUV = fragTexCoord;
    vec2 uv = fract(tileUV / PERIOD) * PERIOD;

    /* --- Flow distortion -------------------------------------------- */
    vec2 f = flow(uv / PERIOD);

    /* --- Height field ------------------------------------------------ */
    float h = 0.0;

    h += wave(uv, vec2(1.0, 0.0), 1.1, f.x) * 0.035;
    h += wave(uv, vec2(0.0, 1.0), 1.4, f.y) * 0.030;
    h += wave(uv, vec2(1.0, 1.0), 0.8, f.x + f.y) * 0.025;
    h += wave(uv, vec2(2.0, 1.0), 1.9, f.x * 1.3) * 0.015;
    h += wave(uv, vec2(1.0, 2.0), 2.2, f.y * 1.7) * 0.012;

    /* --- Tile-safe normal -------------------------------------------- */
    float eps = PERIOD / 512.0;

    float hx =
        wave(uv + vec2(eps, 0.0), vec2(1.0,0.0), 1.1, f.x) -
        wave(uv - vec2(eps, 0.0), vec2(1.0,0.0), 1.1, f.x) +
        wave(uv + vec2(eps, 0.0), vec2(1.0,1.0), 0.8, f.x + f.y) -
        wave(uv - vec2(eps, 0.0), vec2(1.0,1.0), 0.8, f.x + f.y);

    float hy =
        wave(uv + vec2(0.0, eps), vec2(0.0,1.0), 1.4, f.y) -
        wave(uv - vec2(0.0, eps), vec2(0.0,1.0), 1.4, f.y) +
        wave(uv + vec2(0.0, eps), vec2(1.0,1.0), 0.8, f.x + f.y) -
        wave(uv - vec2(0.0, eps), vec2(1.0,1.0), 0.8, f.x + f.y);

    vec3 normal = normalize(vec3(-hx * 5.0, -hy * 5.0, 1.0));

    /* --- Lighting ---------------------------------------------------- */
    vec3 lightDir = normalize(vec3(0.3, 0.4, 0.85));
    float diffuse = clamp(dot(normal, lightDir), 0.0, 1.0);
    float fresnel = pow(1.0 - normal.z, 3.0);

    /* --- Foam -------------------------------------------------------- */
    float foam = smoothstep(0.03, 0.06, h);

    /* --- Color ------------------------------------------------------- */
    vec3 deep    = vec3(0.05, 0.22, 0.45);
    vec3 shallow = vec3(0.10, 0.40, 0.70);

    vec3 waterColor = mix(deep, shallow, diffuse);
    waterColor += fresnel * vec3(0.20, 0.30, 0.35);
    waterColor += foam * vec3(0.05);

    finalColor = vec4(waterColor, 1.0) * fragColor;
}
