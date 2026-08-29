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


uniform sampler2D u_Texture;
uniform float u_skyStrength;//昼1.0, 夜0.1みたいな

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

uniform sampler1D uRenderableDistancesTexture;
uniform float uFogEndMargin;
uniform float uFogWidth;



uniform vec3 dayHorizonColor;	 
uniform vec3 nightHorizonColor;


uniform float uDayFactor;


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
    if (fragPosLightSpace.w <= 0.0) return 0.0;
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (any(lessThan(projCoords, vec3(0.0))) ||
        any(greaterThan(projCoords, vec3(1.0)))) return 0.0;

   
    float ndotl = max(dot(normal, normalize(sunDirection)), 0.0);
    float bias = max(0.00065, 0.0022 * (1.0 - ndotl));
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    float visibility = 0.0;
    for (int i = 0; i < 12; ++i) {
        visibility += texture(shadowMap, vec3(
            projCoords.xy + disk[i] * texelSize * 1.65,
            projCoords.z - bias));
    }
    return 1.0 - visibility / 12.0;
}


const float PI = 3.14159265359;
const float DETAIL_STRENGTH = 0.22;
const float BLOCK_LIGHT_STRENGTH = 0.85;
const float POINT_LIGHT_STRENGTH = 1.0;

vec3 SafeNormalize(vec3 v, vec3 fallback) {
    float len2 = dot(v, v);
    return len2 > 0.000001 ? v * inversesqrt(len2) : fallback;
}

float Luminance(vec3 color) {
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}


ivec2 AtlasTile() {
    return ivec2(floor(vec2(TexCoord.x, 1.0 - TexCoord.y) * vec2(32.0, 16.0)));
}

bool IsTile(ivec2 tile, ivec2 target) {
    return all(equal(tile, target));
}

float MaterialRoughness(vec3 albedo) {
    ivec2 tile = AtlasTile();
    float roughness = 0.86;
    if (IsTile(tile, ivec2(19, 0))) {
       
        roughness = mix(0.82, 0.74, smoothstep(0.025, 0.32, Luminance(albedo)));
    } else if (IsTile(tile, ivec2(18, 1))) {
        roughness = 0.96;
    } else if (IsTile(tile, ivec2(2, 0)) || IsTile(tile, ivec2(3, 0))) {
        roughness = 0.93;
    } else if (IsTile(tile, ivec2(5, 7))) {
        roughness = 0.62;
    }
    return roughness;
}

vec3 MaterialNormal(vec3 geometricNormal) {
    vec2 atlasSize = vec2(textureSize(u_Texture, 0));
    vec2 texel = 1.0 / atlasSize;
    vec2 uvDx = dFdx(TexCoord);
    vec2 uvDy = dFdy(TexCoord);
    vec3 posDx = dFdx(FragPos);
    vec3 posDy = dFdy(FragPos);

   
    vec2 tileCount = vec2(32.0, 16.0);
    vec2 tileMin = floor(TexCoord * tileCount) / tileCount + texel * 0.5;
    vec2 tileMax = tileMin + 1.0 / tileCount - texel;
    float left = Luminance(textureGrad(u_Texture,
        clamp(TexCoord - vec2(texel.x, 0.0), tileMin, tileMax), uvDx, uvDy).rgb);
    float right = Luminance(textureGrad(u_Texture,
        clamp(TexCoord + vec2(texel.x, 0.0), tileMin, tileMax), uvDx, uvDy).rgb);
    float down = Luminance(textureGrad(u_Texture,
        clamp(TexCoord - vec2(0.0, texel.y), tileMin, tileMax), uvDx, uvDy).rgb);
    float up = Luminance(textureGrad(u_Texture,
        clamp(TexCoord + vec2(0.0, texel.y), tileMin, tileMax), uvDx, uvDy).rgb);

    vec3 r1 = cross(posDy, geometricNormal);
    vec3 r2 = cross(geometricNormal, posDx);
    vec3 tangent = r1 * uvDx.x + r2 * uvDy.x;
    vec3 bitangent = r1 * uvDx.y + r2 * uvDy.y;
    float basisLength = max(dot(tangent, tangent), dot(bitangent, bitangent));
    float footprint = max(length(uvDx * atlasSize), length(uvDy * atlasSize));
    float detailFade = 1.0 - smoothstep(1.0, 3.0, footprint);
  

    detailFade *= 1.0 - float(AtlasTile().y == 14);
    vec3 gradient = (tangent * (right - left) + bitangent * (up - down)) *
        inversesqrt(max(basisLength, 0.000001));
    return SafeNormalize(geometricNormal - gradient * DETAIL_STRENGTH * detailFade,
        geometricNormal);
}


float SpecularBRDF(vec3 normal, vec3 viewDir, vec3 lightDir, float roughness) {
    float nl = max(dot(normal, lightDir), 0.0);
    float nv = max(dot(normal, viewDir), 0.001);
    if (nl <= 0.0) return 0.0;
    vec3 halfDir = SafeNormalize(viewDir + lightDir, normal);
    float nh = max(dot(normal, halfDir), 0.0);
    float vh = max(dot(viewDir, halfDir), 0.0);
    float alpha = roughness * roughness;
    float a2 = alpha * alpha;
    float denominator = nh * nh * (a2 - 1.0) + 1.0;
    float distribution = a2 / max(PI * denominator * denominator, 0.00001);
    float k = (roughness + 1.0) * (roughness + 1.0) * 0.125;
    float geometryV = nv / (nv * (1.0 - k) + k);
    float geometryL = nl / (nl * (1.0 - k) + k);
    float fresnel = 0.04 + 0.96 * pow(1.0 - vh, 5.0);
   
    return PI * distribution * geometryV * geometryL * fresnel / max(4.0 * nv, 0.001);
}

vec3 SunColor() {
    float height = max(normalize(sunDirection).y, 0.0);
    float airMass = inversesqrt(height * height + 0.0025);
   

    return exp(-vec3(0.0, 0.045, 0.12) * max(airMass - 1.0, 0.0));
}


vec3 HorizonColor(vec3 viewDir) {
    float day = clamp(uDayFactor, 0.0, 1.0);
    vec3 sunDir = normalize(sunDirection);
    float afterglow = smoothstep(-0.18, 0.02, sunDir.y);
    float skyEnergy = max(day, afterglow * 0.28);
    float twilight = (1.0 - smoothstep(0.02, 0.24, sunDir.y)) * afterglow;
    float horizonLuma = dot(dayHorizonColor, vec3(0.2126, 0.7152, 0.0722));
    vec3 daylightHaze = mix(vec3(horizonLuma), dayHorizonColor, 0.32) * 0.55;
    vec3 color = mix(nightHorizonColor * 0.45, daylightHaze, skyEnergy);
    vec2 horizonSun = sunDir.xz / max(length(sunDir.xz), 0.001);
    vec2 horizonView = viewDir.xz / max(length(viewDir.xz), 0.001);
    float towardSun = pow(max(dot(horizonSun, horizonView), 0.0), 4.0);
    vec3 sunsetHaze = mix(vec3(0.46, 0.16, 0.055), vec3(0.52, 0.31, 0.15),
        smoothstep(-0.06, 0.12, sunDir.y));
    return mix(color, sunsetHaze, twilight * towardSun * 0.82);
}

vec3 CalcPointLights(vec3 normal, vec3 geometricNormal, vec3 viewDir,
    float roughness, out vec3 specular) {
    vec3 result = vec3(0.0);
    specular = vec3(0.0);
    for (int i = 0; i < min(uPointLightCount, 16); ++i) {
        vec3 toLight = uPointLights[i].position - FragPos;
        float distance2 = dot(toLight, toLight);
        float radius = max(uPointLights[i].radius, 0.001);
        if (distance2 >= radius * radius) continue;
        vec3 lightDir = SafeNormalize(toLight, geometricNormal);
        float facing = max(dot(geometricNormal, lightDir), 0.0);
        if (facing <= 0.0) continue;
        float range2 = distance2 / (radius * radius);
        float window = max(1.0 - range2 * range2, 0.0);
     

        float attenuation = window * window / (1.0 + 1.5 * distance2);
        vec3 radiance = max(uPointLights[i].color, vec3(0.0)) *
            max(uPointLights[i].intensity, 0.0) * attenuation * POINT_LIGHT_STRENGTH;
        result += radiance * max(dot(normal, lightDir), 0.0) * 0.96;
        float lightRoughness = max(roughness, 0.28);
        specular += radiance * SpecularBRDF(normal, viewDir, lightDir, lightRoughness);
    }
    return result;
}

vec3 GetLightFromLightV(vec3 fallbackLight) {
    vec3 samplePos = FragPos + normalize(vNormal) * 0.501;
    vec3 localPos = samplePos - uLightVolumeOrigin;
    if (any(lessThan(localPos, vec3(0.0))) ||
        any(greaterThanEqual(localPos, uLightVolumeSize))) return fallbackLight;

    vec4 sampleLight = texture(uLightVolumeTexture, localPos / uLightVolumeSize);
    vec3 edgeDistance = min(localPos, uLightVolumeSize - localPos);
    float edge = min(edgeDistance.x, min(edgeDistance.y, edgeDistance.z));
    


    float valid = step(0.5, sampleLight.a) * smoothstep(0.0, 1.5, edge);
    return mix(fallbackLight, max(sampleLight.rgb, vec3(0.0)), valid);
}

float CalcFogFactor() {
    float fragDist = length(FragPos.xz);
    if (fragDist < 0.0001) return 0.0;
    float angle = atan(FragPos.z, FragPos.x);
    if (angle < 0.0) angle += 2.0 * PI;
    float renderableDist = texture(uRenderableDistancesTexture, angle / (2.0 * PI)).r;
    float fogEnd = max(0.001, renderableDist - uFogEndMargin);
    float fogStart = max(0.0, fogEnd - max(uFogWidth, 0.001));
    return smoothstep(fogStart, max(fogEnd, fogStart + 0.001), fragDist);
}

void main() {
    vec4 texColor = texture(u_Texture, TexCoord);
    vec3 albedo = max(texColor.rgb, vec3(0.0));
    vec3 geometricNormal = normalize(vNormal);
    vec3 normal = MaterialNormal(geometricNormal);
    vec3 viewDir = SafeNormalize(-FragPos, geometricNormal);
    float roughness = MaterialRoughness(albedo);



    vec3 normalDx = dFdx(normal);
    vec3 normalDy = dFdy(normal);
    float normalVariance = dot(normalDx, normalDx) + dot(normalDy, normalDy);
    roughness = sqrt(clamp(roughness * roughness + min(normalVariance * 0.25, 0.12),
        0.04, 1.0));
    float ao = clamp(vAO, 0.45, 1.0);
    float skyLevel = clamp(vSkyLightLevel / 15.0, 0.0, 1.0);
    float skyVisibility = pow(skyLevel, 1.6);
    float day = clamp(uDayFactor, 0.0, 1.0);
    vec3 sunDir = normalize(sunDirection);
    float shadow = CalculateShadow(FragPosLightSpace, geometricNormal);
    float sunVisibility = (1.0 - shadow) * skyVisibility * max(uSunIntensity, 0.0);

    vec3 sunRadiance = SunColor() * 1.65 * sunVisibility;
    vec3 sunLight = sunRadiance *
        max(dot(normal, sunDir), 0.0) * 0.96 *
        step(0.0, dot(geometricNormal, sunDir));
    float sunSpecular = SpecularBRDF(normal, viewDir, sunDir, roughness) *
        step(0.0, dot(geometricNormal, sunDir));

    float hemisphere = mix(0.30, 1.0, geometricNormal.y * 0.5 + 0.5);


    vec3 ambientSkyColor = mix(vec3(0.24, 0.27, 0.34),
        vec3(0.30, 0.35, 0.43), smoothstep(0.0, 0.45, sunDir.y));
    float duskAmbient = smoothstep(-0.16, 0.02, sunDir.y) *
        (1.0 - smoothstep(0.02, 0.20, sunDir.y));
    vec3 skyLight = mix(vec3(0.012, 0.021, 0.042),
        ambientSkyColor * max(u_skyStrength, 0.0), day) *
        skyVisibility * hemisphere;
    skyLight += vec3(0.028, 0.023, 0.022) * duskAmbient * skyVisibility * hemisphere;

    vec3 groundBounce = vec3(0.085, 0.078, 0.066) *
        (1.0 - geometricNormal.y * 0.5 - 0.5) * skyVisibility * day;
    vec3 caveLight = vec3(0.0015, 0.0020, 0.0030) * (1.0 - skyVisibility);



    vec3 moonLight = vec3(0.24, 0.35, 0.58) * 0.12 *
        (1.0 - day) * smoothstep(0.0, 0.25, -sunDir.y) *
        skyVisibility * max(dot(normal, -sunDir), 0.0) *
        step(0.0, dot(geometricNormal, -sunDir));

    vec3 oldBlockLight = vec3(1.0, 0.42, 0.14) * clamp(vBlockLightLevel / 15.0, 0.0, 1.0);


    vec3 rawBlockLight = GetLightFromLightV(oldBlockLight);
    float blockIntensity = max(rawBlockLight.r, max(rawBlockLight.g, rawBlockLight.b));
    vec3 blockColor = blockIntensity > 0.00001 ?
        rawBlockLight / blockIntensity : vec3(0.0);
    vec3 blockLight = blockColor * pow(clamp(blockIntensity, 0.0, 1.0), 2.4) *
        BLOCK_LIGHT_STRENGTH;

    vec3 pointSpecular;
    vec3 pointLight = CalcPointLights(normal, geometricNormal, viewDir,
        roughness, pointSpecular);
    vec3 finalLight = (skyLight + groundBounce + caveLight) * ao +
        sunLight + moonLight + blockLight * mix(0.65, 1.0, ao) + pointLight;


    vec3 litColor = albedo * finalLight +
        (sunRadiance * sunSpecular + pointSpecular) *
        mix(0.65, 1.0, ao);



    vec2 torchLocalUV = (TexCoord - torchMinUV) /
        max(torchMaxUV - torchMinUV, vec2(0.00001));
    float insideTorch = step(0.0, torchLocalUV.x) * step(torchLocalUV.x, 1.0) *
        step(0.0, torchLocalUV.y) * step(torchLocalUV.y, 1.0);
    float flameY = 1.0 - torchLocalUV.y;

    float redRegion = step(0.125, flameY) * (1.0 - step(0.250, flameY));
    float orangeRegion = step(0.0, flameY) * (1.0 - step(0.125, flameY));


    float textureBrightness = max(albedo.r, max(albedo.g, albedo.b));
    float visiblePixelMask = smoothstep(0.18, 0.65, textureBrightness);

    //良い感じにbloomを効かせれる値にしたい
    vec3 redEmission =
        albedo *
        vec3(1.15, 0.32, 0.12) *
        17.0 *
        insideTorch *
        redRegion *
        visiblePixelMask;

    vec3 orangeEmission =
        albedo *
        vec3(1.40, 0.82, 0.30) *
        19.0 *
        insideTorch *
        orangeRegion *
        visiblePixelMask;


    float glowstoneMask = float(IsTile(AtlasTile(), ivec2(5, 7)));
    vec3 glowstoneEmission = albedo * vec3(1.0, 0.70, 0.35) *
        2.8 * glowstoneMask * smoothstep(0.06, 0.45, Luminance(albedo));
    litColor += redEmission + orangeEmission + glowstoneEmission;

    float streamFog = CalcFogFactor();
    float aerialFog = (1.0 - exp(-length(FragPos) * 0.0016)) *
        skyVisibility * mix(0.25, 1.0, day);
    float fogFactor = 1.0 - (1.0 - streamFog) * (1.0 - aerialFog);
    vec3 fogColor = HorizonColor(-viewDir) * mix(0.025, 1.0, skyVisibility);
    FragColor = vec4(max(mix(litColor, fogColor, fogFactor), vec3(0.0)), texColor.a);
}
