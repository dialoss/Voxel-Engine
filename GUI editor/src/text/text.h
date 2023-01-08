#pragma once

#define GLEW_STATIC
#include "GL/glew.h"

#include "../graphics/shader.h"
#include "font.h"
#include "glm/gtc/matrix_transform.hpp"

#include "../world/chunkManager.h"
#include "../camera.h"
#include "../events.h"

class Text {
public:
	unsigned int VAO, VBO;
	Shader* shader;
	Font* font;
	glm::mat4 proj;

	Text(std::string fontName, float width, float height);

	void draw(std::string text, float x, float y, float scale, glm::vec3 color);
	void statistics(ChunkManager* world, Camera* camera);
};