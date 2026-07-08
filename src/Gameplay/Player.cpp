#include "Gameplay/Player.h"
#include "World/World.h"

void Player::SetVelocity(uint8_t xDir, uint8_t yDir, uint8_t zDir) {//0 or 1

	this->velocity = glm::vec3(xDir * speed, yDir * speed, zDir * speed);

}


void Player::Tick(float dt, World& w) {


	velocity += GRAVITY * dt;
	velocity.y = std::max(velocity.y, MAX_FALL_SPEED);


	position.y += velocity.y * dt;
	if (velocity.y > 0.f) {
		AABB box = GetPlrBox();

		MovePositiveY({ box.min.x, box.max.x }, box.max.y, { box.min.z, box.max.z }, w);
	}
	else {
		AABB box = GetPlrBox();

		MoveNegativeY({ box.min.x, box.max.x }, box.min.y, { box.min.z, box.max.z }, w);
	}


	position.x += velocity.x * dt;
	if (velocity.x > 0.f) {
		AABB box = GetPlrBox();

		MovePositiveX(box.max.x, { box.min.y, box.max.y }, { box.min.z, box.max.z }, w);

	}
	else {
		AABB box = GetPlrBox();

		MoveNegativeX(box.min.x, { box.min.y, box.max.y }, { box.min.z, box.max.z }, w);
	}

	position.z += velocity.z * dt;
	if (velocity.z > 0.f) {
		AABB box = GetPlrBox();

		MovePositiveZ({ box.min.x, box.max.x }, { box.min.y, box.max.y }, box.max.z, w);

	}
	else {
		AABB box = GetPlrBox();

		MoveNegativeZ({ box.min.x, box.max.x }, { box.min.y, box.max.y }, box.min.z, w);
	}

	feetPos.x = position.x;
	feetPos.z = position.z;
	feetPos.y = position.y + feetHeight;




}


void Player::MovePositiveX(int64_t x, glm::vec2 ySet, glm::vec2 zSet, World& w) {
	
	constexpr float EPS = 0.0001f;

	int64_t minY = static_cast<int64_t>(std::floor(ySet[0]));
	int64_t maxY = static_cast<int64_t>(std::floor(ySet[1] - EPS));

	int64_t minZ = static_cast<int64_t>(std::floor(zSet[0]));
	int64_t maxZ = static_cast<int64_t>(std::floor(zSet[1] - EPS));

	if (w.CanCollideBlock(x, minY, minZ) ||
		w.CanCollideBlock(x, maxY, minZ) ||
		w.CanCollideBlock(x, minY, maxZ) ||
		w.CanCollideBlock(x, maxY, maxZ)) {


		velocity.x = 0.f;
		position.x = x - width / 2.0f;
	}

}



void Player::MoveNegativeX(int64_t x, glm::vec2 ySet, glm::vec2 zSet, World& w) {
	constexpr float EPS = 0.0001f;


	int64_t minY = static_cast<int64_t>(std::floor(ySet[0]));
	int64_t maxY = static_cast<int64_t>(std::floor(ySet[1] - EPS));

	int64_t minZ = static_cast<int64_t>(std::floor(zSet[0]));
	int64_t maxZ = static_cast<int64_t>(std::floor(zSet[1] - EPS));

	if (w.CanCollideBlock(x, minY, minZ) ||
		w.CanCollideBlock(x, maxY, minZ) ||
		w.CanCollideBlock(x, minY, maxZ) ||
		w.CanCollideBlock(x, maxY, maxZ)) {


		velocity.x = 0.f;
		position.x = (x + BLOCK_SIZE) + width / 2.0f;
	}

}


void Player::MovePositiveY(glm::vec2 xSet, int64_t y, glm::vec2 zSet, World& w) {
	constexpr float EPS = 0.0001f;

	int64_t minX = static_cast<int64_t>(std::floor(xSet[0]));
	int64_t maxX = static_cast<int64_t>(std::floor(xSet[1] - EPS));

	int64_t minZ = static_cast<int64_t>(std::floor(zSet[0]));
	int64_t maxZ = static_cast<int64_t>(std::floor(zSet[1] - EPS));

	if (w.CanCollideBlock(minX, y, minZ) ||
		w.CanCollideBlock(maxX, y, minZ) ||
		w.CanCollideBlock(minX, y, maxZ) ||
		w.CanCollideBlock(maxX, y, maxZ)) {

		velocity.y = 0.f;
		position.y = y - height;//heightÇÕposÇ™äÓèÄ

	}


}


void Player::MoveNegativeY(glm::vec2 xSet, int64_t y, glm::vec2 zSet, World& w) {

	constexpr float EPS = 0.0001f;

	int64_t minX = static_cast<int64_t>(std::floor(xSet[0]));
	int64_t maxX = static_cast<int64_t>(std::floor(xSet[1] - EPS));

	int64_t minZ = static_cast<int64_t>(std::floor(zSet[0]));
	int64_t maxZ = static_cast<int64_t>(std::floor(zSet[1] - EPS));

	if (w.CanCollideBlock(minX, y, minZ) ||
		w.CanCollideBlock(maxX, y, minZ) ||
		w.CanCollideBlock(minX, y, maxZ) ||
		w.CanCollideBlock(maxX, y, maxZ)) {

		velocity.y = 0.f;
		position.y = (y + BLOCK_SIZE) + height/2.0f;//heightÇÕposÇ™äÓèÄ

	}

}



void Player::MovePositiveZ(glm::vec2 xSet, glm::vec2 ySet, int64_t z, World& w) {

	constexpr float EPS = 0.0001f;

	int64_t minX = static_cast<int64_t>(std::floor(xSet[0]));
	int64_t maxX = static_cast<int64_t>(std::floor(xSet[1] - EPS));

	int64_t minY = static_cast<int64_t>(std::floor(ySet[0]));
	int64_t maxY = static_cast<int64_t>(std::floor(ySet[1] - EPS));


	if (w.CanCollideBlock(minX, minY, z) ||
		w.CanCollideBlock(maxX, minY, z) ||
		w.CanCollideBlock(minX, maxY, z) ||
		w.CanCollideBlock(maxX, maxY, z)) {

		velocity.z = 0.f;

		position.z = z - depth;

	}

}


void Player::MoveNegativeZ(glm::vec2 xSet, glm::vec2 ySet, int64_t z, World& w) {
	constexpr float EPS = 0.0001f;

	int64_t minX = static_cast<int64_t>(std::floor(xSet[0]));
	int64_t maxX = static_cast<int64_t>(std::floor(xSet[1] - EPS));

	int64_t minY = static_cast<int64_t>(std::floor(ySet[0]));
	int64_t maxY = static_cast<int64_t>(std::floor(ySet[1] - EPS));


	if (w.CanCollideBlock(minX, minY, z) ||
		w.CanCollideBlock(maxX, minY, z) ||
		w.CanCollideBlock(minX, maxY, z) ||
		w.CanCollideBlock(maxX, maxY, z)) {

		velocity.z = 0.f;

		position.z = (z + depth) + depth / 2.0f;

	}


}



AABB Player::GetPlrBox() const {

	return {
		glm::vec3{position.x - (width/2.f), position.y + feetHeight, position.z - (depth/2.0f)},
		glm::vec3{position.x + (width/2.f), position.y + height, position.z + (depth/2.0f)}
	};

}


glm::vec3 Player::GetPos() const {

	return position;
}


float Player::GetSpeed() const {
	return speed;
}


void Player::UpdateVectors() {
	glm::vec3 f;
	f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	f.y = sin(glm::radians(pitch));
	f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	front = glm::normalize(f);
	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));

}