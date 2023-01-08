#pragma once

#define GLEW_STATIC
#include "GL/glew.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


class Shader {
private:
	unsigned int program;
public:
	Shader(std::string vPath, std::string fPath);
	void use();
	void uniformm(std::string name, glm::mat4 value);
	void uniform3f(std::string name, glm::vec3 value);
	void uniformf(std::string name, float value);
	void uniformv(std::string name, int* value);
	void uniformi(std::string name, int value);
};