#pragma once

#include "../graphics/texture.h"
#include <string>
#include <fstream>
#include <map>
#include <sstream>
#include <iostream>
#include <vector>

static enum Blocks {
	air,
	error,
	dirt,
	grass,
	stone,
	glowstone,
	leaves,
	oak,
	glass,
	bedrock,
	cobble,
	birch,
	sand,
	water
};

struct Block {
	bool emission;
	bool transparent;
	bool solid;
	int r, g, b;
	Block(int r, int g, int b, bool em) : r(r), g(g), b(b), emission(em), solid(true) {}
};

class BlockManager {
public:
	static std::map<std::string, int> NameIndex;
	static std::map<int, std::string> IndexName;
	static std::vector<int> opacity;
	static std::vector<int> drawGroup;
	static std::vector<Block> blocks;
	static Texture* textureArray;
	static std::vector<std::vector<int>> sides;
	static void loadTextures(std::string path);
	static void useTextures();
};