#version 330 core

in vec2 vScreenUV;

out vec4 FragColor;


void main() {

	 vec3 horizonColor = vec3(0.65, 0.82, 1.00);
     vec3 topColor     = vec3(0.20, 0.50, 0.95);


	 float height = clamp(vScreenUV.y, 0.0, 1.0);

	 vec3 skyColor = mix(
		horizonColor,
		topColor,
		height
	 );


	 FragColor = vec4(skyColor, 1.0);

}