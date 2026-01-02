#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform float time;

const float TAU = 6.28318530718;

void main()
{
    // Wrap UV explicitly (defensive, ensures tiling)
    vec2 uv = fract(fragTexCoord);

    // Periodic wave functions (tile-safe)
    float wx = sin(TAU * uv.x + time * 1.2);
    float wy = sin(TAU * uv.y + time * 1.5);
    float wxy = sin(TAU * (uv.x + uv.y) + time * 0.8);

    float wave = (wx + wy + wxy) * 0.04;

    // Secondary ripple (also periodic)
    float ripple = sin(TAU * uv.x * 2.0 + time * 2.0) *
                   sin(TAU * uv.y * 2.0 + time * 1.7) * 0.02;

    float h = wave + ripple;

    // Procedural water color
    vec3 waterColor = vec3(
        0.10 + h,
        0.35 + h * 0.6,
        0.70 + h
    );

    finalColor = vec4(waterColor, 1.0) * fragColor;
}
