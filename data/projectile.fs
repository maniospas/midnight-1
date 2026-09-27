#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
uniform float time;
out vec4 finalColor;

float hash(float n)
{
    return fract(sin(n) * 43758.5453);
}

void main()
{
    vec2 uv = fragTexCoord;
    float x = uv.x;
    float y = abs(uv.y - 0.5) * 2.0;

    float tailFade = smoothstep(0.02, 0.82, x);
    tailFade *= tailFade;

    float tipFade = 1.0 - smoothstep(0.90, 1.0, x);
    float envelope = tailFade * tipFade;

    float haze = 1.0 - smoothstep(0.02, 0.95, y);
    float beam = 1.0 - smoothstep(0.01, 0.28, y);
    float core = 1.0 - smoothstep(0.0, 0.055, y);

    float cells = floor(x * 20.0 - time * 8.0);
    float particleNoise = hash(cells);
    float particleX = fract(x * 20.0 - time * 8.0);
    float particle = 1.0 - smoothstep(0.0, 0.20, abs(particleX - 0.5));

    particle *= step(0.55, particleNoise);
    particle *= 1.0 - smoothstep(0.06, 0.45, y);

    float flicker = 0.78 + 0.22 * sin(time * 18.0 + x * 27.0);

    float alpha = haze * 0.02;
    alpha += beam * 0.014;
    alpha += core * 0.032;
    alpha += particle * 0.045;
    alpha *= envelope;
    alpha *= flicker;

    vec3 outerColor = vec3(1.0, 0.10, 0.02);
    vec3 beamColor = vec3(1.0, 0.38, 0.08);
    vec3 coreColor = vec3(1.0, 0.86, 0.55);

    vec3 color = mix(outerColor, beamColor, beam);
    color = mix(color, coreColor, core);
    color += coreColor * particle * 0.25;

    finalColor = vec4(color, alpha) * fragColor;
}