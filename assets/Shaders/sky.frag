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


float Hash21(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
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



void main() {

	 vec2 screenPos = vScreenUV * 2.0 - 1.0;

	 float offsetX = screenPos.x * aspect * tanHalfFov;

	 float offsetY = screenPos.y * tanHalfFov;
	 

	 float nightFactor = 1.0 - uDayFactor;

	 vec3 worldDirection = normalize(
		cameraForward
		+ cameraRight * offsetX
		+ cameraUp * offsetY
	 
	 );

	 vec3 moonDirection = -sunDirection;

	 float sunView = dot(
		normalize(worldDirection),
		normalize(sunDirection)
	 );


	 float sunDisk = 
		smoothstep(
			0.99972,
			0.99991,
			sunView
		
		);


	float sunGlow = 
		pow(max(sunView, 0.0), 64.0) + pow(max(sunView, 0.0), 8.0) * 0.08;
	
	

	float moonView = dot(
		normalize(worldDirection),
		normalize(moonDirection)
	);

	float moonGlow = 
		pow(max(moonView, 0.0), 48.0);


	float moonDisk =
		smoothstep(
			0.99978, 
			0.99982,
			moonView
		);





	 float moonTexture = 0.72 + 0.28 * ValueNoise(worldDirection.xz * 160.0 + worldDirection.y * 47.0);

	 vec3 dayHorizonColor = vec3(0.35, 0.65, 0.95);
     vec3 dayTopColor     = vec3(0.20, 0.50, 0.95);

	 vec3 nightHorizonColor = vec3(0.018, 0.035, 0.085);
	 vec3 nightTopColor     = vec3(0.012, 0.022, 0.065);

	 vec3 horizonColor = 
		mix(
			nightHorizonColor,
			dayHorizonColor,
			uDayFactor
		
		);

	vec3 topColor = 
		mix(
			nightTopColor,
			dayTopColor,
			uDayFactor
		);


	 float height = clamp(worldDirection.y, 0.0, 1.0);


	 float zenithBlend = pow(clamp(height, 0.0, 1.0), 0.42);

	 vec3 skyColor = mix(
		horizonColor,
		topColor,
		zenithBlend
	 );

	 vec3 sunColor = vec3(1.0, 0.9, 0.65);


	 skyColor += sunColor * (
		sunDisk * 11.0 +
		sunGlow * 0.4
	 ) * uDayFactor;


	skyColor += vec3(0.70, 0.82, 1.15) * moonDisk * moonTexture * nightFactor * 11.0;
    skyColor += vec3(0.12, 0.20, 0.48) * moonGlow * nightFactor * 0.35;

	float noise =
		Hash21(gl_FragCoord.xy) - 0.5;

	skyColor +=
		vec3(noise) / 255.0;

	 FragColor = vec4(max(skyColor, vec3(0.0)), 1.0);

}