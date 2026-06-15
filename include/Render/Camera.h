#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Camera {

	glm::vec3 position = glm::vec3(8.0f, 28.0f, 24.0f);//float‚¾‚Æ32bit. world‚É‚Â‚©‚¤‚È‚çworldÀ•W‚Å‚ ‚é64bit‚Æ‘Î‰‚Å‚«‚é‚æ‚¤‚ÉB
	//‚¾‚ª•`‰æ‚Ífloat‚É’¼‚·‚æ‚¤‚É
	glm::vec3 target = glm::vec3(8.0f, 3.0f, 8.0f);

	glm::vec3 top = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::mat4 GetViewMatrix() const {
		return glm::lookAt(position, target, top);
	}

};