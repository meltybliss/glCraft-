#version 330 core

layout (location = 0) out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D uSceneTexture;
uniform float uThreshold;

void main() {
    vec3 color = max(texture(uSceneTexture, TexCoord).rgb, vec3(0.0));
    float peak = max(color.r, max(color.g, color.b));
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    // Saturated red/blue lights should bloom without first turning white.
    float brightness = mix(luminance, peak, 0.65);
    float threshold = max(uThreshold, 0.001);
    float knee = threshold * 0.5;
    float soft = clamp(brightness - threshold + knee, 0.0, 2.0 * knee);
    soft = soft * soft / max(4.0 * knee, 0.00001);
    float contribution = max(brightness - threshold, soft) /
        max(brightness, 0.00001);
    FragColor = vec4(color * contribution, 1.0);
}
