#pragma once

#include "../utility/variables.h"
#include "../graphics/mesh.h"
#include "noise.h"
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <vector>
#include <mutex>

using uint = unsigned int;
using usint = unsigned short int;

enum Biomes {
	forest,
	desert,
	ocean
};

class Chunk {
public:
	std::vector<std::pair<int,int>> toRemove, toAdd;
	std::set<usint> updateBlocks;
	std::queue<usint> structureBlocks;
	std::set<int> bufferPos;

	int* positions;
	usint* lightMap;
	char* voxels;
	char* numberSides;
	char* heightMap;
	char* biomeMap;
	Mesh* mesh;
	
	int changedCount;
	Chunk** nears;
	int posX, posZ, posInBuffer, waterPos, prevPos;
	int bx, bz;
	bool asNear, meshed, needUpdate, removed, updateLight, loaded, changed;
	int visibleSurfaces, range, waterSurfaces;

	Chunk(int posX, int posZ, int bx, int bz, bool asNear);
	~Chunk();
	void create();
	void updateVisible();
	void setStructure(int ind, int x, const int h, int z);
	void defaultWorld();
	void flatWorld();
	void createWorld();
	void smoothTerrain();
	int hValue(const int& x, const int& z);
	void proceedGrid(int cx, int cz);
	void checkNears(int x, int y, int z);
	void clearBuffers();
};