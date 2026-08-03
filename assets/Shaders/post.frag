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



void main() {


	vec3 scene = texture(uSceneTexture, TexCoord).rgb;

	vec3 bloom = texture(uBloomTexture, TexCoord).rgb;

	vec3 hdrColor =
        scene + bloom * uBloomStrength;


	// HDR Å® 0Å`1Ç÷é©ëRÇ…à≥èk
    vec3 mapped =
        AcesFilm(hdrColor * uExposure);

    // à√ïîÇè≠Çµê¬Ç≠ÅAñæïîÇè≠ÇµígÇ©Ç≠Ç∑ÇÈ
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