#version 330 core

layout (location = 0) out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D uSceneTexture;
uniform float uThreshold;


void main() {

	
	vec3 color = texture(uSceneTexture, TexCoord).rgb;

	float brightness =
		dot(
			color,
			vec3(0.2126, 0.7152, 0.0722)
		);

	float threshold = uThreshold;
	float knee = 0.5;

	float contribution = smoothstep(
		threshold - knee,
		threshold + knee,
		brightness
	);

	FragColor = vec4(
		color * contribution,
		1.0
	);

	
}