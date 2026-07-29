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
in float vEmissionStrength;



uniform sampler2D u_Texture;
uniform float u_skyStrength;//’‹1.0, –é0.1‚İ‚½‚¢‚È

uniform vec3 sunDirection;

uniform sampler2D shadowMap;


uniform int uPointLightCount;
uniform PointLight uPointLights[16];

 


float CalculateShadow(vec4 fragPosLightSpace)
{

	//“§‹œZ
    vec3 projCoords =
        fragPosLightSpace.xyz / fragPosLightSpace.w;
 

	//-1`1 ‚ğ 0`1 ‚É•ÏŠ·
    projCoords = projCoords * 0.5 + 0.5;
 
	if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
		projCoords.y < 0.0 || projCoords.y > 1.0 ||
		projCoords.z < 0.0 || projCoords.z > 1.0)
	{
		return 0.0;
	}


	 //¡•`‚¢‚Ä‚¢‚éfragment©g‚Ì[‚³
    float currentDepth = projCoords.z;

    float bias = 0.001;
	vec2 texelSize =
        1.0 / vec2(textureSize(shadowMap, 0));
	

	float shadow = 0.0;

    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
			//shadow map‚É•Û‘¶‚³‚ê‚½Aˆê”Ôè‘O‚Ì[‚³
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

		//–Ê‚ªŒõŒ¹‚Ì•ûŒü‚ğŒü‚¢‚Ä‚¢‚é‚©‚©‚ç‚í‚©‚éA‚Ç‚ê‚¾‚¯Œõ‚ğó‚¯æ‚é‚©
		float diffuseFactor = max(dot(normal, lightDir), 0.0);

		float softLight = 0.15;
		float lightingFactor = mix(softLight, 1.0, diffuseFactor); 



		//ŒõŒ¹‚©‚ç‰“‚­‚È‚é‚Ù‚Çã‚­‚·‚é
		float attenuation = clamp(1.0 - distanceToLight / uPointLights[i].radius, 0.0, 1.0);

		//Œ¸Š‚ğ­‚µ©‘R‚É‚·‚é
		attenuation = attenuation * attenuation * attenuation;

		result +=
			uPointLights[i].color *
			uPointLights[i].intensity *
			lightingFactor *
			attenuation;



		
	}


	return result;
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

	vec3 emissiveColor =
		texColor.rgb *
		vEmissionStrength;


	vec3 hdrColor =
		litColor +
		emissiveColor;

	FragColor = vec4(
		hdrColor,
		texColor.a
	);

}