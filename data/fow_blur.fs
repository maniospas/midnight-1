#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;
uniform vec2 resolution;
uniform float radius;

void main() {
    vec2 texel = 1.0 / resolution;
    vec4 sum = vec4(0.0);

    // small 9-tap gaussian-like blur
    sum += texture(texture0, fragTexCoord + texel * vec2(-1, -1)) * 0.06;
    sum += texture(texture0, fragTexCoord + texel * vec2( 0, -1)) * 0.09;
    sum += texture(texture0, fragTexCoord + texel * vec2( 1, -1)) * 0.06;

    sum += texture(texture0, fragTexCoord + texel * vec2(-1,  0)) * 0.09;
    sum += texture(texture0, fragTexCoord)                     * 0.40;
    sum += texture(texture0, fragTexCoord + texel * vec2( 1,  0)) * 0.09;

    sum += texture(texture0, fragTexCoord + texel * vec2(-1,  1)) * 0.06;
    sum += texture(texture0, fragTexCoord + texel * vec2( 0,  1)) * 0.09;
    sum += texture(texture0, fragTexCoord + texel * vec2( 1,  1)) * 0.06;

    finalColor = sum;
}
