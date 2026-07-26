#include "Gameplay/Player.h"
#include "World/World.h"

void Player::SetVelocity(uint8_t xDir, uint8_t yDir, uint8_t zDir) {//0 or 1

	velocity = glm::dvec3(
		static_cast<double>(xDir) * speed,
		static_cast<double>(yDir) * speed,
		static_cast<double>(zDir) * speed
	);

}


void Player::CalcVelocityXZ(PlayerInput& input) {


	glm::vec3 local_front = GetFront();
	glm::vec3 local_right = GetRight();

	local_front.y = 0.0f;
	local_right.y = 0.0f;

	if (glm::length(local_front) > 0.0f) {
		local_front = glm::normalize(local_front);
	}

	if (glm::length(local_right) > 0.0f) {
		local_right = glm::normalize(local_right);
	}

	glm::vec3 moveDir{ 0.0f };

	float s = m_isSpectator ? flySpeed : speed;


	if (input.forward) moveDir += local_front;
	if (input.back)    moveDir -= local_front;
	if (input.left)    moveDir -= local_right;
	if (input.right)   moveDir += local_right;
	

	if (glm::length(moveDir) > 0.0f) {
		moveDir = glm::normalize(moveDir);
	}


	SetVelX(moveDir.x * s);
	SetVelZ(moveDir.z * s);

}


void Player::Tick(float dt, World& w, PlayerInput& input) {



	bool wasOnGround = onGround;
	onGround = false;

	if (input.toggleSpectator) {
		m_isSpectator = !m_isSpectator;
	}

	
	CalcVelocityXZ(input);


	if (m_isSpectator) {

		if (input.up) {
			velocity.y = 30.0;
		}
		if (input.down) {
			velocity.y = -30.0;
		}

		position.local += velocity * static_cast<double>(dt);

		NormalizePosition(position);




		velocity.y = 0.0f;
	}
	else {

		if (input.up && wasOnGround) {

			velocity.y = jumpPower;
		}

		velocity.y += GRAVITY * dt;
		velocity.y = std::max(velocity.y, MAX_FALL_SPEED);


		position.local.y += velocity.y * static_cast<double>(dt);
		NormalizeAxis(
			position.block.y,
			position.local.y
		);

		if (velocity.y > 0.0) {
			const WorldAABB box = GetPlrBox();
			int64_t hitY = box.originBlock.y + static_cast<int64_t>(std::floor(box.max.y));
			MovePositiveY(hitY, box, w);
		}
		else if (velocity.y < 0.0) {
			WorldAABB box = GetPlrBox();
			int64_t hitY = box.originBlock.y + static_cast<int64_t>(std::floor(box.min.y));
			MoveNegativeY(hitY, box, w);
		}


		position.local.x += velocity.x * static_cast<double>(dt);
		NormalizeAxis(
			position.block.x,
			position.local.x
		);

		if (velocity.x > 0.0) {
			WorldAABB box = GetPlrBox();

			int64_t hitX = box.originBlock.x + static_cast<int64_t>(std::floor(box.max.x));
			MovePositiveX(hitX, box, w);

		}
		else if (velocity.x < 0.0) {
			WorldAABB box = GetPlrBox();
			int64_t hitX = box.originBlock.x + static_cast<int64_t>(std::floor(box.min.x));
			MoveNegativeX(hitX, box, w);
		}

		position.local.z += velocity.z * static_cast<double>(dt);
		NormalizeAxis(
			position.block.z,
			position.local.z
		);

		if (velocity.z > 0.0) {
			WorldAABB box = GetPlrBox();
			int64_t hitZ = box.originBlock.z + static_cast<int64_t>(std::floor(box.max.z));
			MovePositiveZ(hitZ, box, w);

		}
		else if (velocity.z < 0.0) {
			WorldAABB box = GetPlrBox();
			int64_t hitZ = box.originBlock.z + static_cast<int64_t>(std::floor(box.min.z));
			MoveNegativeZ(hitZ, box, w);
		}
	}

	

	velocity.x = 0.0;
	velocity.z = 0.0;

}


void Player::MovePositiveX(int64_t x, const WorldAABB& box, World& w) {
	
	constexpr double EPS = 0.0001;

	int64_t minY = box.originBlock.y + static_cast<int64_t>(std::floor(box.min.y));
	int64_t maxY = box.originBlock.y + static_cast<int64_t>(std::floor(box.max.y - EPS));

	int64_t minZ = box.originBlock.z + static_cast<int64_t>(std::floor(box.min.z));
	int64_t maxZ = box.originBlock.z + static_cast<int64_t>(std::floor(box.max.z - EPS));

	if (w.CanCollideBlock(x, minY, minZ) ||
		w.CanCollideBlock(x, maxY, minZ) ||
		w.CanCollideBlock(x, minY, maxZ) ||
		w.CanCollideBlock(x, maxY, maxZ)) {


		velocity.x = 0.0;

		position.block.x = x;
		position.local.x = -width / 2.0;

		NormalizeAxis(
			position.block.x,
			position.local.x
		);
	}

}



void Player::MoveNegativeX(int64_t x, const WorldAABB& box, World& w) {
	constexpr double EPS = 0.0001;


	int64_t minY = box.originBlock.y + static_cast<int64_t>(std::floor(box.min.y));
	int64_t maxY = box.originBlock.y + static_cast<int64_t>(std::floor(box.max.y - EPS));

	int64_t minZ = box.originBlock.z + static_cast<int64_t>(std::floor(box.min.z));
	int64_t maxZ = box.originBlock.z + static_cast<int64_t>(std::floor(box.max.z - EPS));

	if (w.CanCollideBlock(x, minY, minZ) ||
		w.CanCollideBlock(x, maxY, minZ) ||
		w.CanCollideBlock(x, minY, maxZ) ||
		w.CanCollideBlock(x, maxY, maxZ)) {


		velocity.x = 0.f;
		position.block.x = x + BLOCK_SIZE;
		position.local.x = width / 2.0;

		NormalizeAxis(
			position.block.x,
			position.local.x
		);

	}

}


void Player::MovePositiveY(int64_t y, const WorldAABB& box, World& w) {
	constexpr double EPS = 0.0001;

	int64_t minX = box.originBlock.x + static_cast<int64_t>(std::floor(box.min.x));
	int64_t maxX = box.originBlock.x + static_cast<int64_t>(std::floor(box.max.x - EPS));

	int64_t minZ = box.originBlock.z + static_cast<int64_t>(std::floor(box.min.z));
	int64_t maxZ = box.originBlock.z + static_cast<int64_t>(std::floor(box.max.z - EPS));

	if (w.CanCollideBlock(minX, y, minZ) ||
		w.CanCollideBlock(maxX, y, minZ) ||
		w.CanCollideBlock(minX, y, maxZ) ||
		w.CanCollideBlock(maxX, y, maxZ)) {

		velocity.y = 0.0;
	

		position.block.y = y;
		position.local.y = -height;

		NormalizeAxis(
			position.block.y,
			position.local.y
		);

	}


}


void Player::MoveNegativeY(int64_t y, const WorldAABB& box, World& w) {

	constexpr float EPS = 0.0001;

	int64_t minX = box.originBlock.x + static_cast<int64_t>(std::floor(box.min.x));
	int64_t maxX = box.originBlock.x + static_cast<int64_t>(std::floor(box.max.x - EPS));

	int64_t minZ = box.originBlock.z + static_cast<int64_t>(std::floor(box.min.z));
	int64_t maxZ = box.originBlock.z + static_cast<int64_t>(std::floor(box.max.z - EPS));

	if (w.CanCollideBlock(minX, y, minZ) ||
		w.CanCollideBlock(maxX, y, minZ) ||
		w.CanCollideBlock(minX, y, maxZ) ||
		w.CanCollideBlock(maxX, y, maxZ)) {

		velocity.y = 0.f;
	
		position.block.y = y;
		position.local.y = static_cast<double>(BLOCK_SIZE);

		NormalizeAxis(
			position.block.y,
			position.local.y
		);

		onGround = true;
	}

}



void Player::MovePositiveZ(int64_t z, const WorldAABB& box, World& w) {

	constexpr float EPS = 0.0001;

	int64_t minX = box.originBlock.x + static_cast<int64_t>(std::floor(box.min.x));
	int64_t maxX = box.originBlock.x + static_cast<int64_t>(std::floor(box.max.x - EPS));

	int64_t minY = box.originBlock.y + static_cast<int64_t>(std::floor(box.min.y));
	int64_t maxY = box.originBlock.y + static_cast<int64_t>(std::floor(box.max.y - EPS));


	if (w.CanCollideBlock(minX, minY, z) ||
		w.CanCollideBlock(maxX, minY, z) ||
		w.CanCollideBlock(minX, maxY, z) ||
		w.CanCollideBlock(maxX, maxY, z)) {

		velocity.z = 0.f;
		
		position.block.z = z;
		position.local.z = -depth / 2.0;

		NormalizeAxis(
			position.block.z,
			position.local.z
		);
	}

}


void Player::MoveNegativeZ(int64_t z, const WorldAABB& box, World& w) {
	constexpr float EPS = 0.0001;

	int64_t minX = box.originBlock.x + static_cast<int64_t>(std::floor(box.min.x));
	int64_t maxX = box.originBlock.x + static_cast<int64_t>(std::floor(box.max.x - EPS));

	int64_t minY = box.originBlock.y + static_cast<int64_t>(std::floor(box.min.y));
	int64_t maxY = box.originBlock.y + static_cast<int64_t>(std::floor(box.max.y - EPS));


	if (w.CanCollideBlock(minX, minY, z) ||
		w.CanCollideBlock(maxX, minY, z) ||
		w.CanCollideBlock(minX, maxY, z) ||
		w.CanCollideBlock(maxX, maxY, z)) {

		velocity.z = 0.f;
		

		position.block.z = z;
		position.local.z = static_cast<double>(BLOCK_SIZE) + depth / 2.0;

		NormalizeAxis(
			position.block.z,
			position.local.z
		);

	}


}


WorldAABB Player::GetPlrBox() const {



	return {

		position.block,

		glm::dvec3{
			position.local.x - width * 0.5,
			position.local.y + 0.001,
			position.local.z - depth * 0.5
		},
		glm::dvec3{
			position.local.x + width * 0.5,
			position.local.y + height - 0.001,
			position.local.z + depth * 0.5
		}
	};
}


WorldPos Player::GetPos() const {

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

