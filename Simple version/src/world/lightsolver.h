#pragma once

#include "chunkManager.h"
#include <queue>
#include <mutex>

struct LightCell {
	int x, y, z, light;
	LightCell(int x, int y, int z, int light) : x(x), y(y), z(z), light(light) {}
};

class LightSolver {
public:
	ChunkManager* world;
	int channel;
	LightSolver(ChunkManager* world, int channel) : world(world), channel(channel) {}
	std::mutex queueMut;

	std::queue<LightCell> removeLight, addLight;
	void setLight(Chunk* &chunk, const int& x, const int& y, const int& z, const usint &value);
	void remove(Chunk*& chunk, const int& x, const int& y, const int& z);
	void add(Chunk* &chunk, const int& x, const int& y, const int& z, const int& emission);
	void add(const int& x, const int& y, const int& z);
	void updateLights(bool newChunk);
};