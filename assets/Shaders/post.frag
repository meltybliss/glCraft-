#version 330 core

layout (location = 0) out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D uSceneTexture;
uniform sampler2D uBloomTexture;


uniform float uExposure;
uniform float uBloomStrength;


vec3 AcesFilm(vec3 color) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

vec3 AcesFilmColorPreserving(vec3 color) {
    float peak = max(color.r, max(color.g, color.b));

    if (peak <= 0.00001) {
        return vec3(0.0);
    }

    float mappedPeak = AcesFilm(vec3(peak)).r;
    return color * (mappedPeak / peak);
}



const float BLOOM_AMOUNT = 0.28;
const float COLOR_PRESERVATION = 0.55;

vec3 LinearToSRGB(vec3 color) {
    vec3 low = 12.92 * color;
    vec3 high = 1.055 * pow(color, vec3(1.0 / 2.4)) - 0.055;
    return mix(low, high, step(vec3(0.0031308), color));
}

void main() {
    vec3 scene = max(texture(uSceneTexture, TexCoord).rgb, vec3(0.0));
    vec3 bloom = max(texture(uBloomTexture, TexCoord).rgb, vec3(0.0));
    vec3 hdrColor = scene + bloom * max(uBloomStrength, 0.0) * BLOOM_AMOUNT;
    vec3 exposedColor = hdrColor * max(uExposure, 0.0);
    float peak = max(exposedColor.r, max(exposedColor.g, exposedColor.b));
    // Ordinary surfaces use the filmic shoulder; bright emitters retain more hue.
    // No global blue-shadow / orange-highlight tint: color comes from the lights.
    float preserve = mix(0.22, COLOR_PRESERVATION, smoothstep(0.8, 3.0, peak));
    vec3 mapped = mix(AcesFilm(exposedColor), AcesFilmColorPreserving(exposedColor), preserve);
    mapped = LinearToSRGB(clamp(mapped, 0.0, 1.0));

    // Static sub-LSB dither after tone mapping: avoids banding in the night sky.
    float dither = fract(52.9829189 * fract(dot(gl_FragCoord.xy,
        vec2(0.06711056, 0.00583715)))) - 0.5;
    FragColor = vec4(clamp(mapped + dither / 255.0, 0.0, 1.0), 1.0);
}
