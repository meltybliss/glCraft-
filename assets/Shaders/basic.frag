#version 330 core

out vec4 FragColor;


in vec2 TexCoord;
in vec3 vNormal;
in float vBlockLightLevel;
in float vSkyLightLevel;
in float vAO;
in vec4 FragPosLightSpace;

uniform sampler2D u_Texture;
uniform float u_skyStrength;//’‹1.0, –é0.1‚Ý‚½‚¢‚È

uniform vec3 sunDirection;

uniform sampler2D shadowMap;


float CalculateShadow(vec4 fragPosLightSpace)
{

	//“§Ž‹œŽZ
    vec3 projCoords =
        fragPosLightSpace.xyz / fragPosLightSpace.w;


	//-1`1 ‚ð 0`1 ‚É•ÏŠ·
    projCoords = projCoords * 0.5 + 0.5;

	//shadow map‚É•Û‘¶‚³‚ê‚½Aˆê”ÔŽè‘O‚Ì[‚³
    float closestDepth =
        texture(shadowMap, projCoords.xy).r;

	 //¡•`‚¢‚Ä‚¢‚éfragmentŽ©g‚Ì[‚³
    float currentDepth = projCoords.z;

    float bias = 0.001;

    return currentDepth - bias > closestDepth
        ? 1.0
        : 0.0;

}

void main() {
	vec4 texColor = texture(u_Texture, TexCoord);
	

	vec3 sunColor = vec3(1.0, 0.95, 0.85);



	float shadow = CalculateShadow(FragPosLightSpace);


	
	float sky = vSkyLightLevel / 15.0;

	float B_brightness = vBlockLightLevel / 15.0;
	float S_brightness = sky * u_skyStrength;

	float ambientBrightness  = max(B_brightness, S_brightness);

	float diffuse = 
		max(dot(normalize(vNormal), -sunDirection), 0.0);


	vec3 sunLight = sunColor * diffuse * sky * (1.0 - shadow);

	vec3 ambientLight = vec3(ambientBrightness);

	vec3 finalLight =
		ambientLight +
		sunLight;

	FragColor = vec4(
		texColor.rgb * finalLight * vAO,
		texColor.a
	);

}