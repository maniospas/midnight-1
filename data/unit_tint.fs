#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 factionColor;   // color used to replace white pixels

out vec4 finalColor;

void main() {
    vec4 texColor = texture(texture0, fragTexCoord);

    // Detect white (or near-white) pixels
    bool isWhite = (texColor.r > 0.95 &&
                    texColor.g > 0.95 &&
                    texColor.b > 0.95);

    if (isWhite) {
        // Replace the RGB but keep original alpha
        finalColor = vec4(factionColor.rgb, texColor.a);
    } else {
        finalColor = texColor * fragColor;
    }
}
