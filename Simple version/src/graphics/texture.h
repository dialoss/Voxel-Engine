#pragma once

#include <string>
#include "stb_image.h"
#define GLEW_STATIC
#include "GL/glew.h"
#include <vector>

class Texture {
public:
	unsigned int texture_array, texture;
	std::vector<std::string> textures;
	std::string folderPath;
	int width, height, count, curCount;

	Texture(std::string folderPath, int width, int height, int count);
	Texture(std::string imagePath);
	void addTexture(std::string name);
	void use(std::string type);
	void generateMipmaps();
};