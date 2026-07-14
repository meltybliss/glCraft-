#version 330 core

out vec4 FragColor;


in vec2 TexCoord;
in vec3 vNormal;
in float vBlockLightLevel;
in float vSkyLightLevel;
in float vAO;

uniform sampler2D u_Texture;
uniform float u_skyStrength;//’‹1.0, –é0.1‚Ý‚½‚¢‚È

uniform vec3 sunDirection;


void main() {
	vec4 texColor = texture(u_Texture, TexCoord);
	

	vec3 sunColor = vec3(1.0, 0.95, 0.85);

	
	float sky = vSkyLightLevel / 15.0;

	float B_brightness = vBlockLightLevel / 15.0;
	float S_brightness = sky * u_skyStrength;

	float ambientBrightness  = max(B_brightness, S_brightness);

	float diffuse = 
		max(dot(normalize(vNormal), -sunDirection), 0.0);


	vec3 sunLight = sunColor * diffuse * sky;

	vec3 ambientLight = vec3(ambientBrightness);

	vec3 finalLight =
		ambientLight +
		sunLight;

	FragColor = vec4(
		texColor.rgb * finalLight * vAO,
		texColor.a
	);

}