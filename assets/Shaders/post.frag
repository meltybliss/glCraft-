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



void main() {


	vec3 scene = texture(uSceneTexture, TexCoord).rgb;

	vec3 bloom = texture(uBloomTexture, TexCoord).rgb;

	vec3 hdrColor =
        scene + bloom * uBloomStrength;


	// HDR → 0～1へ自然に圧縮
    vec3 exposedColor = hdrColor * uExposure;
    vec3 mappedPerChannel = AcesFilm(exposedColor);
    vec3 mappedColorPreserving =
        AcesFilmColorPreserving(exposedColor);

    vec3 mapped = mix(
        mappedPerChannel,
        mappedColorPreserving,
        0.45
    );

    // 暗部を少し青く、明部を少し暖かくする
    float luminance =
        dot(
            mapped,
            vec3(0.2126, 0.7152, 0.0722)
        );

    float gradeAmount =
        smoothstep(
            0.18,
            0.82,
            luminance
        );

    mapped *= mix(
        vec3(0.95, 0.98, 1.07),
        vec3(1.035, 1.005, 0.965),
        gradeAmount
    );

	mapped = pow(
		mapped,
		vec3(1.0 / 2.2)
	);


	FragColor = vec4(mapped, 1.0);

}
