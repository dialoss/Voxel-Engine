#pragma once

#include <unordered_map>
#include <vector>
#include <queue>
#include <set>
#include <mutex>
#include "chunk.h"
#include "../utility/geometry.h"
#include "../graphics/mesh.h"
#include "glm/glm.hpp"
#include "../threads/thread_pool.hpp"

class ChunkManager {
public:
	//std::unordered_map<int, Chunk*> chunks;
	std::queue<Chunk*> toAdd, toRemove, toUnload, toUpdate, updateStruct;

	Chunk** chunks;
	Chunk** ctemp;
	Mesh** meshes;
	Mesh** mtemp;
	int centerX, centerZ;
	int wx, wz, rx, rz, BW;
	int visibleSurfaces, waterSurfaces;
	glm::ivec3 maxSurfaces;
	int workers;
	thread_pool *tp;
	
	
	ChunkManager();
	void init();

	usint getLight(const int &x, const int &y, const int &z, const int &channel);
	usint getLight(const int& x, const int& y, const int& z, Chunk* chunk, const int& channel);
	char* rayCast(glm::vec3 pos, glm::vec3 dir, float maxDist, glm::ivec3& norm, glm::ivec3& iend, std::vector<glm::ivec3> &positions, bool log);
	char* getVoxel(int x, int y, int z);
	void getNears(Chunk* chunk);
	Chunk* getChunk(const int& cx, const int& cz);
	Chunk* getChunk(const int &x, const int &y, const int &z);
	void translate(glm::ivec3 delta);
	void shift(int dx, int dz);
};