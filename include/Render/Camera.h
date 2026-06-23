#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Camera {

	glm::vec3 position = glm::vec3(8.0f, 28.0f, 24.0f);//floatだと32bit. worldにつかうならworld座標である64bitと対応できるように。
	//だが描画はfloatに直すように
	glm::vec3 front = glm::vec3(0.f, 0.f, -1.0f);
	glm::vec3 right = glm::vec3(1.f, 0.f, 0.f);

	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);//飛行機のように視点が傾いてるときでも対応できるように上を向いたときの向くべき方向。
	glm::vec3 worldUp = glm::vec3(0.f, 1.f, 0.f);//上昇するときのための

	float yaw = -90.f;
	float pitch = 0.f;

	float moveSpeed = 30.f;
	float mouseSensitivity = 0.1f;

	glm::mat4 GetViewMatrix() const {
		return glm::lookAt(position, position + front, up);
	}

	void UpdateVectors() {
		glm::vec3 f;
		f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		f.y = sin(glm::radians(pitch));
		f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

		front = glm::normalize(f);
		right = glm::normalize(glm::cross(front, worldUp));
		up = glm::normalize(glm::cross(right, front));

	}

};