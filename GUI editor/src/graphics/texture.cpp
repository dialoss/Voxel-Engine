#define STB_IMAGE_IMPLEMENTATION
#include "texture.h"
#include <iostream>

Texture::Texture(std::string folderPath, int width, int height, int count) {
	this->folderPath = folderPath;
	this->width = width;
	this->height = height;
	this->count = count;
	curCount = 0;

	glGenTextures(1, &texture_array);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture_array);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage3D(
		GL_TEXTURE_2D_ARRAY, 0, GL_RGBA,
		width, height, count,
		0, GL_RGBA, GL_UNSIGNED_BYTE, NULL
	);
}

Texture::Texture(std::string imagePath, GLuint numChunnels) {
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(imagePath.c_str(), &width, &height, &nrChannels, 0);
	if (!data) {
		std::cout << "FAILED TO LOAD TEXTURE" << std::endl;
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, numChunnels, width, height, 0, numChunnels, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);
}

void Texture::loadFromBuffer(const float* buffer, int width, int height, GLuint numChunnels) {
	unsigned char* image = new unsigned char[width * height * 4];
	memset(image, 0, width * height * 4 * sizeof(unsigned char));

	for (int x = 0; x < width; x++)
	{
		for (int z = 0; z < height; z++)
		{
			float noise = buffer[z + width * x] + abs(buffer[z + width * x]);
			noise = std::max(noise, (float)0);
			noise = std::min(noise, (float)1);
			unsigned char pixel = 255 * noise;
			for (int i = 0; i < 3; i++)
			{
				image[4 * (z + width * x) + i] = pixel;
			}
			image[4 * (z + width * x) + 3] = 255;
		}
	}

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::addTexture(std::string name) {

	if (find(textures.begin(), textures.end(), name) == textures.end()) {
		textures.push_back(name);
		curCount++;
		//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load((folderPath + "/" + name).c_str(), &width, &height, &nrChannels, 0);
		if (!data) {
			std::cout << "FAILED TO LOAD TEXTURE" << std::endl;
		}

		glBindTexture(GL_TEXTURE_2D_ARRAY, texture_array);

		glTexSubImage3D(
			GL_TEXTURE_2D_ARRAY, 0,
			0, 0, curCount - 1,
			width, height, 1,
			GL_RGBA, GL_UNSIGNED_BYTE,
			data
		);
		stbi_image_free(data);
	}
}

void Texture::generateMipmaps() {
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}

void Texture::use(std::string type) {
	if (type == "array") {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, texture_array);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, texture);
	}
}