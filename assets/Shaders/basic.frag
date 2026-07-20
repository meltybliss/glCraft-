#version 330 core

out vec4 FragColor;


in vec2 TexCoord;
in vec3 vNormal;
in float vBlockLightLevel;
in float vSkyLightLevel;
in float vAO;
in vec4 FragPosLightSpace;

uniform sampler2D u_Texture;
uniform float u_skyStrength;//íã1.0, ñÈ0.1Ç›ÇΩÇ¢Ç»

uniform vec3 sunDirection;

uniform sampler2D shadowMap;


float CalculateShadow(vec4 fragPosLightSpace)
{

	//ìßéãèúéZ
    vec3 projCoords =
        fragPosLightSpace.xyz / fragPosLightSpace.w;
 

	//-1Å`1 Ç 0Å`1 Ç…ïœä∑
    projCoords = projCoords * 0.5 + 0.5;
 

	 //ç°ï`Ç¢ÇƒÇ¢ÇÈfragmenté©êgÇÃê[Ç≥
    float currentDepth = projCoords.z;

    float bias = 0.001;
	vec2 texelSize =
        1.0 / vec2(textureSize(shadowMap, 0));
	

	float shadow = 0.0;

    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
			//shadow mapÇ…ï€ë∂Ç≥ÇÍÇΩÅAàÍî‘éËëOÇÃê[Ç≥
            float closestDepth =
                texture(
                    shadowMap,
                    projCoords.xy +
                    vec2(x, y) * texelSize
                ).r;

            shadow +=
                currentDepth - bias > closestDepth
                ? 1.0
                : 0.0;
        }
    }

    return shadow / 9.0;

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