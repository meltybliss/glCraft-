#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <string>
#include <fstream>
#include <filesystem>
#include <sstream>

class Shader {
public:
	Shader(const std::string& vertPath, const std::string& fragPath);


private:
	std::string ReadFile(const std::string& shaderPath);

	unsigned int CompileShader(unsigned int type, const std::string& source);
};