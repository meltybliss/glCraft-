#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in float aBlockLightLevel;
layout (location = 4) in float aSkyLightLevel;
layout (location = 5) in float aAO;

out vec2 TexCoord;
out vec3 vNormal;
out float vBlockLightLevel;
out float vSkyLightLevel;
out float vAO;

out vec4 FragPosLightSpace;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform mat4 lightSpaceMatrix;



void main() {

	vec4 worldPos =  model * vec4(aPos, 1.0);
	
	
	FragPosLightSpace = lightSpaceMatrix * worldPos;

	
	gl_Position = projection * view * worldPos;


	TexCoord = aTexCoord;
	vNormal = aNormal;
	vBlockLightLevel = aBlockLightLevel;
	vSkyLightLevel = aSkyLightLevel;
	vAO = aAO;
}
