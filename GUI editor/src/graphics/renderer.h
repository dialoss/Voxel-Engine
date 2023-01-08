#pragma once
#define GLEW_STATIC
#include "GL/glew.h"
#include "../utility/variables.h"
#include "../world/config.h"

#include <unordered_map>
#include <vector>

using uint = unsigned int;

class Renderer {
public:

	unsigned int EBO;
	unsigned int VAO;
	unsigned int VBO;
	int vCount, sCount;

	Renderer() {};
	Renderer(const uint* buffer, const int &vCount, const int* attrs);
	Renderer(const float* buffer, const int &vCount, const int* attrs);
	~Renderer();
	void update(const uint* buffer, std::vector<std::pair<int, int>> &sides);
	void loadChunk(const uint* buffer, const int &pos, const int &curCount);
	void loadLines(const uint* buffer);
	void initLines(const float* buffer, const int* indices);
	void removeSides(std::vector<std::pair<int, int>>& sides);
	void removeBlock(int pos);
	void draw(uint type, bool water);
};