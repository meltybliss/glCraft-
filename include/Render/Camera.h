#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include "World/WorldPos.h"

struct Camera {

	//apply from player


	WorldPos position;
	glm::vec3 front = glm::vec3(0.f, 0.f, -1.0f);
	glm::vec3 right = glm::vec3(1.f, 0.f, 0.f);

	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);//飛行機のように視点が傾いてるときでも対応できるように上を向いたときの向くべき方向。
	glm::vec3 worldUp = glm::vec3(0.f, 1.f, 0.f);//上昇するときのための

	float fov = 70.0f;

	float mouseSensitivity = 0.1f;

	glm::mat4 GetViewMatrix() const {

		glm::vec3 eye{ 0.0f };//世界をカメラからのrelativeな値の位置に配置する仕組みにするのでカメラは常に原点にする。cameraのposからあるオブジェクトの位置を引いた距離にものを配置

		return glm::lookAt(eye, eye + front, up); 
	}

};