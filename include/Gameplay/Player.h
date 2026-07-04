#pragma once
#include "Math/AABB.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class World;
class Player {
public:

	void SetVelocity(uint8_t xDir, uint8_t yDir, uint8_t zDir);//0 or 1
	void SetPosition(glm::vec3&& pos) { position = pos; }

	void Tick(float dt, World& w);
	
	void MovePositiveX(int64_t x, glm::vec2 ySet, glm::vec2 zSet, World& w);
	void MoveNegativeX(int64_t x, glm::vec2 ySet, glm::vec2 zSet, World& w);

	void MovePositiveY(glm::vec2 xSet, int64_t y, glm::vec2 zSet, World& w);
	void MoveNegativeY(glm::vec2 xSet, int64_t y, glm::vec2 zSet, World& w);

	void MovePositiveZ(glm::vec2 xSet, glm::vec2 ySet, int64_t z, World& w);
	void MoveNegativeZ(glm::vec2 xSet, glm::vec2 ySet, int64_t z, World& w);

	[[nodiscard]] AABB GetPlrBox() const;
private:
	glm::vec3 position{};
	glm::vec3 feetPos{};

	glm::vec3 velocity{};

	const float GRAVITY = -25.0f;
	const float MAX_FALL_SPEED = -50.0f;

	float width = 0.6f;
	float depth = 0.6f;

	float eyeHeight = 1.4f;
	float height = 1.6f;

	float feetHeight = -height / 2.f;

	float speed = 10.f;
};
