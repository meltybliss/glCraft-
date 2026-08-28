#version 330 core

layout (location = 0) out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D uImage;
uniform bool uHorizontal;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(uImage, 0));
    vec2 axis = uHorizontal ? vec2(texelSize.x, 0.0) : vec2(0.0, texelSize.y);
    // Bilinear paired taps: the same normalized 9-tap kernel with only 5 fetches.
    // The existing C++ blur pass count stays unchanged.
    vec3 result = texture(uImage, TexCoord).rgb * 0.2270270270;
    result += texture(uImage, TexCoord + axis * 1.3846153846).rgb * 0.3162162162;
    result += texture(uImage, TexCoord - axis * 1.3846153846).rgb * 0.3162162162;
    result += texture(uImage, TexCoord + axis * 3.2307692308).rgb * 0.0702702703;
    result += texture(uImage, TexCoord - axis * 3.2307692308).rgb * 0.0702702703;
    FragColor = vec4(result, 1.0);
}
