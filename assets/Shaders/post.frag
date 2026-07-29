#version 330 core

layout (location = 0) out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D uSceneTexture;
uniform sampler2D uBloomTexture;


uniform float uExposure;
uniform float uBloomStrength;



void main() {


	vec3 scene = texture(uSceneTexture, TexCoord).rgb;

	vec3 bloom = texture(uBloomTexture, TexCoord).rgb;

	vec3 hdrColor =
        scene + bloom * uBloomStrength;


	vec3 mapped = 
		vec3(1.0) - 
		exp(-hdrColor * uExposure);

	mapped = pow(
		mapped,
		vec3(1.0 / 2.2)
	);


	FragColor = vec4(mapped, 1.0);

}