#version 330 core

in vec2 vScreenUV;

out vec4 FragColor;


uniform vec3 cameraForward;
uniform vec3 cameraRight;
uniform vec3 cameraUp;

uniform float tanHalfFov;
uniform float aspect;

uniform float uDayFactor;

uniform vec3 sunDirection;


uniform vec3 dayHorizonColor;
uniform vec3 dayTopColor;
	 
uniform vec3 nightHorizonColor;
uniform vec3 nightTopColor;


float Hash21(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

vec2 hash22(vec2 p)
{
    vec2 x = vec2(
        dot(p, vec2(127.1, 311.7)),
        dot(p, vec2(269.5, 183.3))
    );

    return fract(sin(x) * 43758.5453);
}

float ValueNoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);

    float a = Hash21(i);
    float b = Hash21(i + vec2(1.0, 0.0));
    float c = Hash21(i + vec2(0.0, 1.0));
    float d = Hash21(i + vec2(1.0, 1.0));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}


vec2 DirectionToStarUV(vec3 dir)
{
	const float PI = 3.14159265359;
    
	dir = normalize(dir);

    float u =
        atan(dir.z, dir.x) / (2.0 * PI) + 0.5;

    float v =
        asin(clamp(dir.y, -1.0, 1.0)) / PI + 0.5;

    return vec2(u, v);
}


float RenderStar(vec3 worldDirection)
{
    vec2 p = DirectionToStarUV(worldDirection) * 300.0;
    vec2 cell = floor(p);
    float existence = Hash21(cell + vec2(17.3, 91.7));
    vec2 starPos = mix(vec2(0.15), vec2(0.85), hash22(cell));
    float distanceToStar = length(fract(p) - starPos);
    float aa = max(length(fwidth(p)), 0.008);
    float radius = mix(0.035, 0.070, Hash21(cell + 9.4));
    float star = 1.0 - smoothstep(max(radius - aa, 0.0), radius + aa, distanceToStar);
    // Limit unresolved stars' energy instead of letting them flicker at a hard edge.
    return star * min(1.0, radius * radius / max(aa * aa, 0.00001)) *
        step(0.990, existence);
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

void main() {
    vec2 screenPos = vScreenUV * 2.0 - 1.0;
    vec3 worldDirection = normalize(cameraForward +
        cameraRight * screenPos.x * aspect * tanHalfFov +
        cameraUp * screenPos.y * tanHalfFov);
    vec3 sunDir = normalize(sunDirection);
    vec3 moonDirection = -sunDir;
    float day = clamp(uDayFactor, 0.0, 1.0);
    float nightFactor = 1.0 - day;
    float aboveHorizon = smoothstep(-0.004, 0.004, worldDirection.y);
   

    float height = max(worldDirection.y, 0.0);
    height = height * height / (height + 0.025);

    vec3 horizonColor = HorizonColor(worldDirection);
    float afterglow = smoothstep(-0.18, 0.02, sunDir.y);
    float twilight = (1.0 - smoothstep(0.02, 0.24, sunDir.y)) * afterglow;
    float skyEnergy = max(day, afterglow * 0.20);
    vec3 topColor = mix(nightTopColor * 0.38,
        dayTopColor * vec3(0.40, 0.42, 0.48), skyEnergy);
 

    float horizonWeight = exp(-height * mix(4.5, 9.0, twilight));
    vec3 skyColor = mix(topColor, horizonColor, horizonWeight);

    float sunView = dot(worldDirection, sunDir);
    float sunDisk = smoothstep(0.99982, 0.99996, sunView);
    float airMass = inversesqrt(max(sunDir.y, 0.0) * max(sunDir.y, 0.0) + 0.0025);
    vec3 sunColor = exp(-vec3(0.0, 0.045, 0.12) * max(airMass - 1.0, 0.0));
    float sunVisible = smoothstep(-0.012, 0.005, sunDir.y);
    float sunGlow = pow(max(sunView, 0.0), 256.0) * 0.12 +
        pow(max(sunView, 0.0), 24.0) * 0.018;
    skyColor += sunColor * (sunDisk * 22.0 + sunGlow) * sunVisible * aboveHorizon;

    float moonView = dot(worldDirection, moonDirection);
    float moonDisk = smoothstep(0.99980, 0.99988, moonView);
    float moonGlow = pow(max(moonView, 0.0), 80.0);
    vec3 moonAxis = abs(moonDirection.z) > 0.99 ? vec3(0.0, 1.0, 0.0) : vec3(0.0, 0.0, 1.0);
    vec3 moonRight = normalize(cross(moonAxis, moonDirection));
    vec3 moonUp = cross(moonDirection, moonRight);
    vec2 moonUV = vec2(dot(worldDirection, moonRight), dot(worldDirection, moonUp));
    float moonTexture = 0.65 + 0.23 * ValueNoise(moonUV * 340.0) +
        0.12 * ValueNoise(moonUV * 850.0 + 13.2);
    float moonVisible = smoothstep(-0.03, 0.03, moonDirection.y) * nightFactor;
    skyColor += vec3(0.72, 0.81, 1.0) * moonDisk * moonTexture * 3.5 *
        moonVisible * aboveHorizon;
    skyColor += vec3(0.16, 0.24, 0.43) * moonGlow * 0.07 * moonVisible;

    float starDisk = RenderStar(worldDirection);
    vec3 starColor = mix(vec3(0.68, 0.80, 1.0), vec3(1.0, 0.84, 0.62),
        Hash21(floor(DirectionToStarUV(worldDirection) * 300.0)));
    float starVisibility = nightFactor * (1.0 - smoothstep(-0.16, 0.01, sunDir.y));
    skyColor += starColor * starDisk * 2.3 * starVisibility *
        smoothstep(0.02, 0.20, worldDirection.y);
    FragColor = vec4(max(skyColor, vec3(0.0)), 1.0);
}
