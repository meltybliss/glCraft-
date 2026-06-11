#include "Render/Shader.h"

Shader::Shader(const std::string& vertPath, const std::string& fragPath) {
	std::string vertSource = ReadFile(vertPath);
	std::string fragSource = ReadFile(fragPath);

	unsigned int vertShader = CompileShader(GL_VERTEX_SHADER, vertSource);
	unsigned int fragShader = CompileShader(GL_FRAGMENT_SHADER, fragSource);

	unsigned int program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);

	glLinkProgram(program);


}


std::string Shader::ReadFile(const std::string& shaderPath) {
	std::ifstream file(shaderPath);
	if (!file.is_open()) {
		return "";
	}

	std::stringstream ss;
	ss << file.rdbuf();

	return ss.str();
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source) {

	unsigned int shader = glCreateShader(type);
	const char* src = source.c_str();

	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);

	return shader;
}