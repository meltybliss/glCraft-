#version 330 core

out vec4 FragColor;


struct PointLight {
	vec3 position;
	vec3 color;
	float radius;
	float intensity;

};



in vec3 FragPos;
in vec2 TexCoord;
in vec3 vNormal;
in float vBlockLightLevel;
in float vSkyLightLevel;
in float vAO;
in vec4 FragPosLightSpace;
in vec3 vLightColor;



uniform sampler2D u_Texture;
uniform float u_skyStrength;//íã1.0, ñÈ0.1Ç›ÇΩÇ¢Ç»

uniform vec3 sunDirection;

uniform sampler2D shadowMap;


uniform int uPointLightCount;
uniform PointLight uPointLights[16];

uniform sampler3D uLightVolumeTexture; 
uniform vec3 uLightVolumeOrigin;
uniform vec3 uLightVolumeSize;


float CalculateShadow(vec4 fragPosLightSpace)
{

	//ìßéãèúéZ
    vec3 projCoords =
        fragPosLightSpace.xyz / fragPosLightSpace.w;
 

	//-1Å`1 Ç 0Å`1 Ç…ïœä∑
    projCoords = projCoords * 0.5 + 0.5;
 
	if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
		projCoords.y < 0.0 || projCoords.y > 1.0 ||
		projCoords.z < 0.0 || projCoords.z > 1.0)
	{
		return 0.0;
	}


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


vec3 CalcPointLights(vec3 normal) {

	vec3 result = vec3(0.0);

	for (int i = 0; i < uPointLightCount; i++) {
		
		vec3 toLight = uPointLights[i].position - FragPos;

		float distanceToLight = length(toLight);
		if (distanceToLight > uPointLights[i].radius) continue;

		vec3 lightDir = normalize(toLight);

		//ñ Ç™åıåπÇÃï˚å¸Çå¸Ç¢ÇƒÇ¢ÇÈÇ©Ç©ÇÁÇÌÇ©ÇÈÅAÇ«ÇÍÇæÇØåıÇéÛÇØéÊÇÈÇ©
		float diffuseFactor = max(dot(normal, lightDir), 0.0);

		float softLight = 0.15;
		float lightingFactor = mix(softLight, 1.0, diffuseFactor); 



		//åıåπÇ©ÇÁâìÇ≠Ç»ÇÈÇŸÇ«é„Ç≠Ç∑ÇÈ
		float attenuation = clamp(1.0 - distanceToLight / uPointLights[i].radius, 0.0, 1.0);

		//å∏êäÇè≠Çµé©ëRÇ…Ç∑ÇÈ
		attenuation = attenuation * attenuation * attenuation;

		result +=
			uPointLights[i].color *
			uPointLights[i].intensity *
			lightingFactor *
			attenuation;



		
	}


	return result;
}


vec3 GetLightFromLightV(vec3 fallbackLight) {
	
	vec3 localPos =
        FragPos - uLightVolumeOrigin;

    bool insideVolume =
        all(greaterThanEqual(
            localPos,
            vec3(0.0)
        )) &&
        all(lessThan(
            localPos,
            uLightVolumeSize
        ));

    if (!insideVolume) {
        return fallbackLight;
    }


	vec3 texturePos =
        localPos / uLightVolumeSize;

    return texture(
        uLightVolumeTexture,
        texturePos
    ).rgb;
}


void main() {
	vec4 texColor = texture(u_Texture, TexCoord);
	


	vec3 sunColor = vec3(1.0, 0.95, 0.85);



	float shadow = CalculateShadow(FragPosLightSpace);


	
	float sky = vSkyLightLevel / 15.0;

	float B_brightness = vBlockLightLevel / 15.0;
	float S_brightness = sky * u_skyStrength;

	
	vec3 oldBlockLight = vLightColor * B_brightness;

	vec3 blockLightFromLightV = GetLightFromLightV(oldBlockLight);



	float diffuse = 
		max(dot(normalize(vNormal), -sunDirection), 0.0);


	vec3 sunLight = sunColor * diffuse * sky * (1.0 - shadow);


	vec3 skyLight = vec3(S_brightness);

	vec3 ambientLight = skyLight + blockLightFromLightV;


	vec3 normal = normalize(vNormal);
	vec3 pointLight = CalcPointLights(normal);


	vec3 finalLight =
		ambientLight +
		sunLight +
		pointLight;


	vec3 litColor =
		texColor.rgb *
		finalLight *
		vAO;

	

	vec3 hdrColor =
		litColor;

	FragColor = vec4(
		hdrColor,
		texColor.a
	);

}