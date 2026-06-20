#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <filesystem>
#include <sstream>

class Shader {
public:
	Shader(const std::string& vertPath, const std::string& fragPath);

	void Use() const;
	void SetMat4(const char* name, const glm::mat4& mat4) const;
	void SetInt(const char* name, const int& value) const;
	void SetVec4(const char* name, const glm::vec4& vec4) const;

private:
	std::string ReadFile(const std::string& shaderPath);

	unsigned int CompileShader(unsigned int type, const std::string& source);

private:
	unsigned int m_id = -1;
};