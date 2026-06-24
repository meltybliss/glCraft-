#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in float vLightLevel;

uniform sampler2D u_Texture;

void main() {
	vec4 texColor = texture(u_Texture, TexCoord);

	float brightness = vLightLevel / 15.0f;

	FragColor = vec4(
		texColor.rgb * brightness,
		texColor.a
	);

}