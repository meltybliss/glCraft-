#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in float vBlockLightLevel;
in float vSkyLightLevel;
in float vAO;

uniform sampler2D u_Texture;
uniform float u_skyStrength;//’‹1.0, –é0.1‚Ý‚½‚¢‚È

void main() {
	vec4 texColor = texture(u_Texture, TexCoord);

	float B_brightness = vBlockLightLevel / 15.0f;
	float S_brightness = 
		(vSkyLightLevel / 15.0f) * u_skyStrength;

	float brightness = max(B_brightness, S_brightness);

	FragColor = vec4(
		texColor.rgb * brightness * vAO,
		texColor.a
	);

}