#include "Render/Shader.h"
#include <iostream>

Shader::Shader(const std::string& vertPath, const std::string& fragPath) {
	std::string vertSource = ReadFile(vertPath);
	std::string fragSource = ReadFile(fragPath);

	unsigned int vertShader = CompileShader(GL_VERTEX_SHADER, vertSource);
	unsigned int fragShader = CompileShader(GL_FRAGMENT_SHADER, fragSource);

	unsigned int program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);

	glLinkProgram(program);
	int success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);

	if (!success) {
		char infoLog[1024];
		glGetProgramInfoLog(program, 1024, nullptr, infoLog);
		std::cerr << "Shader link error:\n" << infoLog << std::endl;
	}

	m_id = program;

	glDeleteShader(vertShader);
	glDeleteShader(fragShader);
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

	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success) {
		char infoLog[1024];
		glGetShaderInfoLog(shader, 1024, nullptr, infoLog);

		if (type == GL_VERTEX_SHADER) {
			std::cerr << "Vertex shader compile error:\n";
		}
		else if (type == GL_FRAGMENT_SHADER) {
			std::cerr << "Fragment shader compile error:\n";
		}
		else {
			std::cerr << "Shader compile error:\n";
		}

		std::cerr << infoLog << std::endl;
	}


	return shader;
}


void Shader::Use() {
	glUseProgram(m_id);
}