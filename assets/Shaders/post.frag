#version 330 core

layout (location = 0) out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D uSceneTexture;
uniform float uExposure;



void main() {


	vec3 hdrColor = 
		texture(uSceneTexture, TexCoord).rgb;


	vec3 mapped = 
		vec3(1.0) - 
		exp(-hdrColor * uExposure);

	mapped = pow(
		mapped,
		vec3(1.0 / 2.2)
	);


	FragColor = vec4(mapped, 1.0);

}