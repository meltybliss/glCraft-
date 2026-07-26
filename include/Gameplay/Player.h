#pragma once
#include "Gameplay/PlayerInput.h"
#include "Math/AABB.h"
#include "World/WorldPos.h"

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

	void SetPosition(const WorldPos& pos)
	{
		position = pos;
	}
	void SetYaw(const float yaw) { this->yaw = yaw; }
	void SetPitch(const float pitch) { this->pitch = pitch; }

 	void Tick(float dt, World& w, PlayerInput& input);
	
	void MovePositiveX(int64_t x, const WorldAABB& box, World& w);
	void MoveNegativeX(int64_t x, const WorldAABB& box, World& w);

	void MovePositiveY(int64_t y, const WorldAABB& box, World& w);
	void MoveNegativeY(int64_t y, const WorldAABB& box, World& w);

	void MovePositiveZ(int64_t z, const WorldAABB& box, World& w);
	void MoveNegativeZ(int64_t z, const WorldAABB& box, World& w);

	void UpdateVectors();
	

	[[nodiscard]] WorldAABB GetPlrBox() const;
	[[nodiscard]] WorldPos GetPos() const;
	[[nodiscard]] WorldPos GetEyePos() const
	{
		WorldPos eyePos = position;

		eyePos.local.y += eyeHeight;
		NormalizePosition(eyePos);

		return eyePos;
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

	void CalcVelocityXZ(PlayerInput& input);

private:


	WorldPos position{
		glm::i64vec3(34359738160LL, 200, 34359738160LL),
		glm::dvec3(0.0)
	};
	

	glm::dvec3 velocity{};
	
	//camera系
	glm::vec3 front = glm::vec3(0.f, 0.f, -1.0f);
	glm::vec3 right = glm::vec3(1.f, 0.f, 0.f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);//飛行機のように視点が傾いてるときでも対応できるように上を向いたときの向くべき方向。
	glm::vec3 worldUp = glm::vec3(0.f, 1.f, 0.f);//上昇するときのための

	float yaw = -90.f;
	float pitch = 0.f;
	//

	const double GRAVITY = -25.0;//-25
	const double MAX_FALL_SPEED = -50.0;

	double width = 0.6;
	double depth = 0.6;

	double eyeHeight = 1.8;
	double height = 2.0;


	float speed = 10.f;
	float flySpeed = 30.0f;

	float jumpPower = 8.0f;

	bool onGround = false;
	bool m_isSpectator = false;
};
