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

uniform sampler2DShadow shadowMap;


uniform int uPointLightCount;
uniform PointLight uPointLights[16];

uniform sampler3D uLightVolumeTexture; 
uniform vec3 uLightVolumeOrigin;
uniform vec3 uLightVolumeSize;


uniform vec2 torchMinUV;
uniform vec2 torchMaxUV;


uniform float uSunIntensity;


const vec2 disk[12] = vec2[12](
    vec2(-0.326, -0.406),
    vec2(-0.840, -0.074),
    vec2(-0.696,  0.457),
    vec2(-0.203,  0.621),
    vec2( 0.962, -0.195),
    vec2( 0.473, -0.480),
    vec2( 0.519,  0.767),
    vec2( 0.185, -0.893),
    vec2( 0.507,  0.064),
    vec2( 0.896,  0.412),
    vec2(-0.322, -0.933),
    vec2(-0.792, -0.598)
);

float CalculateShadow(vec4 fragPosLightSpace, vec3 normal)
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


	float normalLight =
		max(
			dot(normal, sunDirection),
			0.0
		);

	float ndotl =
		max(dot(normal, sunDirection), 0.0);

	float bias =
		max(
			0.0008,
			0.0030 * (1.0 - ndotl)
		);

	vec2 texelSize =
        1.0 / vec2(textureSize(shadowMap, 0));
	


	float visibility = 0.0;

    float filterRadius = 1.5;

	for (int i = 0; i < 12; i++) {
		vec2 sampleUV = 
			projCoords.xy +
			disk[i] *
			texelSize *
			filterRadius;

		visibility += 
			texture(
				shadowMap,
				vec3(
					sampleUV,
					currentDepth - bias
				)
			);
	}

	visibility /= 12.0;
	//visibility
	//1.0 = ì˙å¸
    //0.0 = âe

    return 1.0 - visibility;

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
	
	vec3 samplePos =
        FragPos + normalize(vNormal) * 0.501;

	vec3 localPos =
		samplePos - uLightVolumeOrigin;

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
	vec3 caveAmbient =
		vec3(
			0.012,
			0.016,
			0.024
		);


	vec3 sunColor =
		vec3(1.00, 0.90, 0.72);

		
	vec3 normal = normalize(vNormal);

	float shadow = CalculateShadow(FragPosLightSpace, normal);


	float skyLevel =
		clamp(vSkyLightLevel / 15.0, 0.0, 1.0);

	//ínï\, åäÇÃê[Ç≥Ç…ÇÊÇÈãÛåıÇÃìûíBó 
	float skyVisibility =
		pow(skyLevel, 1.6);

	//íãñÈÇ‡ä‹ÇﬂÇΩç≈èIìIÇ»ã≠Ç≥
	float skyAmount =
		skyVisibility *
		u_skyStrength;
	

	float B_brightness = vBlockLightLevel / 15.0;

	
	vec3 oldBlockLight = vLightColor * B_brightness;



	vec3 rawBlockLight =
        GetLightFromLightV(oldBlockLight);

	float blockIntensity =
		max(
			rawBlockLight.r,
			max(rawBlockLight.g, rawBlockLight.b)
		);

	//ã≠ìxÇ∆êFÇï™ó£
    vec3 blockColor =
        blockIntensity > 0.00001
        ? rawBlockLight / blockIntensity
        : vec3(0.0);

	//í·ÉåÉxÉãÇÃBlock LightÇé„Ç≠Ç∑ÇÈ
    blockIntensity =
        pow(
            clamp(blockIntensity, 0.0, 1.0),
            4.0
        );

	vec3 blockLight =
		blockColor *
		blockIntensity *
		1.2;


	

	float wrappedDiffuse =
		clamp(
			(dot(normal, sunDirection) + 0.08) / 1.08,
			0.0,
			1.0
		);

	vec3 sunLight =
		sunColor *
		wrappedDiffuse *
		skyVisibility *
		uSunIntensity *
		(1.0 - shadow * 0.88);


	// ínñ Ç©ÇÁÇÃé„Ç¢ígêFîΩéÀ
	vec3 groundBounce =
		vec3(0.055, 0.035, 0.020) *
		max(-normal.y, 0.0) *
		skyAmount;


	vec3 ambientSkyColor = vec3(0.58, 0.72, 0.92);

	float hemisphere =
		mix(
			0.45,
			1.0,
			normal.y * 0.5 + 0.5
		);

	vec3 skyLight =
		ambientSkyColor *
		mix(0.035, 1.0, skyVisibility) *
		u_skyStrength *
		hemisphere;

	//ãÛÇ™ìÕÇ©Ç»Ç¢èÍèäÇŸÇ«ì¥åAAmbientÇã≠Ç≠Ç∑ÇÈ
    float caveFactor =
        1.0 -
        smoothstep(
            0.0,
            0.25,
            skyLevel
        );


	vec3 caveLight =
        caveAmbient * caveFactor;


	vec3 pointLight = CalcPointLights(normal);


	float ao =
		clamp(vAO, 0.45, 1.0);



	vec3 finalLight =
		(skyLight + groundBounce + caveLight) * ao +
		sunLight * mix(0.72, 1.0, ao) +
		blockLight +
		pointLight;


	vec3 litColor =
		texColor.rgb *
		finalLight;


	vec2 torchLocalUV =
		(TexCoord - torchMinUV) /
		(torchMaxUV - torchMinUV);

	float insideTorch =
		step(0.0, torchLocalUV.x) *
		step(torchLocalUV.x, 1.0) *
		step(0.0, torchLocalUV.y) *
		step(torchLocalUV.y, 1.0);
	

	float flameY = 1.0 - torchLocalUV.y;

	float whiteRegionStart = 0.125;
	float whiteRegionEnd = 0.250;

	float whiteRegion =
		step(whiteRegionStart, flameY) *
		(1.0 - step(whiteRegionEnd, flameY));


	float orangeRegionStart = 0.0;
	float orangeRegionEnd = 0.125;



	float orangeRegion =
		step(orangeRegionStart, flameY) *
		(1.0 - step(orangeRegionEnd, flameY));

	float textureBrightness =
		max(texColor.r, max(texColor.g, texColor.b));


	float visiblePixelMask =
		smoothstep(0.18, 0.65, textureBrightness);



	float whiteMask =
		insideTorch *
		whiteRegion *
		visiblePixelMask;

	float orangeMask =
		insideTorch *
		orangeRegion *
		visiblePixelMask;

	vec3 whiteEmission =
		texColor.rgb *
		vec3(1.15, 1.05, 0.80) *
		4.0 *
		whiteMask;

	vec3 orangeEmission =
		texColor.rgb *
		vec3(1.30, 0.95, 0.45) *
		7.0 *
		orangeMask;


	litColor +=
		whiteEmission +
		orangeEmission;
	
	
	FragColor =
        vec4(
            litColor,
            texColor.a
        );

}