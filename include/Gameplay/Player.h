#pragma once
#include "Math/AABB.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class World;
class Player {
public:

	void SetVelocity(uint8_t xDir, uint8_t yDir, uint8_t zDir);//0 or 1
	void SetVelX(float velX) {
		velocity.x = velX;
	}

	void SetVelY(float velY) {
		velocity.y = velY;
	}

	void SetVelZ(float velZ) {
		velocity.z = velZ;
	}

	void SetPosition(const glm::vec3& pos) { position = pos; }
	void SetYaw(const float yaw) { this->yaw = yaw; }
	void SetPitch(const float pitch) { this->pitch = pitch; }

 	void Tick(float dt, World& w);
	
	void MovePositiveX(int64_t x, glm::vec2 ySet, glm::vec2 zSet, World& w);
	void MoveNegativeX(int64_t x, glm::vec2 ySet, glm::vec2 zSet, World& w);

	void MovePositiveY(glm::vec2 xSet, int64_t y, glm::vec2 zSet, World& w);
	void MoveNegativeY(glm::vec2 xSet, int64_t y, glm::vec2 zSet, World& w);

	void MovePositiveZ(glm::vec2 xSet, glm::vec2 ySet, int64_t z, World& w);
	void MoveNegativeZ(glm::vec2 xSet, glm::vec2 ySet, int64_t z, World& w);

	void UpdateVectors();

	[[nodiscard]] AABB GetPlrBox() const;
	[[nodiscard]] glm::vec3 GetPos() const;
	[[nodiscard]] glm::vec3 GetEyePos() const {

		return position + glm::vec3(0, eyeHeight, 0);
	}
	[[nodiscard]] float GetSpeed() const;
	[[nodiscard]] glm::vec3 GetFront() const {
		return front;
	}

	[[nodiscard]] glm::vec3 GetRight() const {
		return right;
	}

	[[nodiscard]] glm::vec3 GetUp() const {
		return up;
	}
	[[nodiscard]] glm::vec3 GetWorldUp() const {
		return worldUp;
	}

	[[nodiscard]] float GetYaw() const {
		return yaw;
	}

	[[nodiscard]] float GetPitch() const {
		return pitch;
	}

	
private:
	glm::vec3 position{0.f, 200.f, 0.f};
	glm::vec3 feetPos{};

	glm::vec3 velocity{};
	
	//camera系
	glm::vec3 front = glm::vec3(0.f, 0.f, -1.0f);
	glm::vec3 right = glm::vec3(1.f, 0.f, 0.f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);//飛行機のように視点が傾いてるときでも対応できるように上を向いたときの向くべき方向。
	glm::vec3 worldUp = glm::vec3(0.f, 1.f, 0.f);//上昇するときのための

	float yaw = -90.f;
	float pitch = 0.f;
	//

	const float GRAVITY = -25.0f;//-25
	const float MAX_FALL_SPEED = -50.0f;

	float width = 0.6f;
	float depth = 0.6f;

	float eyeHeight = 1.8f;
	float height = 2.f;


	float speed = 10.f;
};
