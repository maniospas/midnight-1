#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform float hillSeed;
uniform float hillHeight;

const vec3 lightDir = normalize(vec3(-0.35, -0.45, 0.82));

float hash(vec2 p) {
    p = fract(p * vec2(443.897, 441.423));
    p += dot(p, p + 19.19);
    return fract(p.x * p.y);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(
        mix(hash(i), hash(i + vec2(1, 0)), u.x),
        mix(hash(i), hash(i + vec2(0, 1)), u.x),
        u.y
    );
}

/* Fractal noise for strong terrain breakup */
float fbm(vec2 p) {
    float v = 0.0;
    float a = 0.55;
    for (int i = 0; i < 4; i++) {
        v += noise(p) * a;
        p *= 2.1;
        a *= 0.5;
    }
    return v;
}

void main() {
    vec2 uv = fragTexCoord;
    vec4 base = texture(texture0, uv);

    float h = clamp(hillHeight, 0.0, 1.0);
    if (h == 0.0) {
        finalColor = base * fragColor;
        return;
    }

    /* Edge preservation */
    float edge =
        min(min(uv.x, 1.0 - uv.x),
            min(uv.y, 1.0 - uv.y));
    float edgeMask = smoothstep(0.02, 0.12, edge);

    /* Strong height field */
    vec2 p = uv * 6.0 + hillSeed * 12.0;
    float n = fbm(p);

    /* Nonlinear amplification */
    float heightAmp = pow(h, 0.6);
    float heightField = n * heightAmp;

    /* Much steeper normal reconstruction */
    float eps = 0.0015;
    float hx = fbm(p + vec2(eps, 0.0));
    float hy = fbm(p + vec2(0.0, eps));

    vec3 normal = normalize(vec3(
        (hx - n) * heightAmp * 18.0,
        (hy - n) * heightAmp * 18.0,
        1.0
    ));

    float light = clamp(dot(normal, lightDir), 0.0, 1.0);

    /* Stronger height-driven color shift */
    vec3 low  = base.rgb * vec3(0.80, 0.88, 0.80);
    vec3 high = base.rgb * vec3(1.25, 1.20, 1.05);
    vec3 heightColor = mix(low, high, heightField);

    /* Harder contrast */
    vec3 shaded = heightColor * (0.45 + 0.75 * light);

    float influence = edgeMask * heightAmp;
    vec3 finalRGB = mix(base.rgb, shaded, influence);

    finalColor = vec4(finalRGB, base.a) * fragColor;
}
