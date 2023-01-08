#include "shader.h"

void createShader(unsigned int target, std::string path) {
	std::string source;
	std::ifstream sourceFile;
	try {
		sourceFile.open(path);
		std::stringstream stream;
		stream << sourceFile.rdbuf();

		sourceFile.close();
		source = stream.str();
	}
	catch (std::ifstream::failure e) {
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}

	const char* code = source.c_str();

	glShaderSource(target, 1, &code, NULL);
	glCompileShader(target);
	char log[512];
	int status;
	glGetShaderiv(target, GL_COMPILE_STATUS, &status);
	if (!status) {
		glGetShaderInfoLog(target, 512, NULL, log);
		std::cout << "SHADER COMPILATION FAILED\n" << log << std::endl;
	}
}


Shader::Shader(std::string vPath, std::string fPath) {
	program = glCreateProgram();

	unsigned int vertShader = glCreateShader(GL_VERTEX_SHADER);
	createShader(vertShader, vPath);
	glAttachShader(program, vertShader);

	unsigned int fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	createShader(fragShader, fPath);
	glAttachShader(program, fragShader);

	glLinkProgram(program);
	int success;
	char log[512];
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, 512, NULL, log);
		std::cout << "PROGRAM LINKING FAILED\n" << log << std::endl;
	}

	glDeleteShader(vertShader);
	glDeleteShader(fragShader);
}

void Shader::use() {
	glUseProgram(program);
}

void Shader::uniformm(std::string name, glm::mat4 value) {
	GLuint pos = glGetUniformLocation(program, name.c_str());
	glUniformMatrix4fv(pos, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::uniform3f(std::string name, glm::vec3 value) {
	GLuint pos = glGetUniformLocation(program, name.c_str());
	glUniform3fv(pos, 1, glm::value_ptr(value));
}

void Shader::uniformf(std::string name, float value) {
	GLuint pos = glGetUniformLocation(program, name.c_str());
	glUniform1f(pos, value);
}