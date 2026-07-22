#version 330 core

in vec2 vScreenUV;

out vec4 FragColor;


uniform vec3 cameraForward;
uniform vec3 cameraRight;
uniform vec3 cameraUp;

uniform float tanHalfFov;
uniform float aspect;


void main() {

	 vec2 screenPos = vScreenUV * 2.0 - 1.0;

	 float offsetX = screenPos.x * aspect * tanHalfFov;

	 float offsetY = screenPos.y * tanHalfFov;


	 vec3 worldDirection = normalize(
		cameraForward
		+ cameraRight * offsetX
		+ cameraUp * offsetY
	 
	 );


	 vec3 horizonColor = vec3(0.65, 0.82, 1.00);
     vec3 topColor     = vec3(0.20, 0.50, 0.95);


	 float height = clamp(worldDirection.y, 0.0, 1.0);

	 vec3 skyColor = mix(
		horizonColor,
		topColor,
		height
	 );


	 FragColor = vec4(skyColor, 1.0);

}