#pragma once

#include "chunk.h"
#include "chunkManager.h"
#include "lightsolver.h"
#include <queue>

using usint = unsigned short int;

class LightManager {
public:
	static LightSolver* solverR;
	static LightSolver* solverG;
	static LightSolver* solverB;
	static LightSolver* solverS;
	
	static ChunkManager* world;

	static void initialize(ChunkManager* world);
	static usint getLight(int x, int y, int z, int channel);
	static void removeBlock(Chunk* chunk, int x, int y, int z);
	static void placeBlock(Chunk* chunk, int x, int y, int z, int ind);
	static void proceedSunLight(Chunk* chunk);

	static void updateLights();
};