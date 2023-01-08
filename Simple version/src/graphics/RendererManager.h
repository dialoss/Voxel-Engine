#pragma once

#include <vector>
#include <queue>
#include <map>
#include <set>
#include <mutex>
#include "renderer.h"
#include "../world/chunk.h"
#include "../world/config.h"

class ChunksRenderer {
public:
	int free, chunks, globalSidePos;
	Renderer* meshRenderer;
	Renderer* waterRenderer;
	std::vector<int> freePlaces;
	std::vector<std::vector<int>> freeChunks;
	std::vector<int> freeWater;
	std::map<int, int> blocks_index;

	ChunksRenderer() {
		free = 1e4;
		chunks = 0;
		globalSidePos = 0;
		freeChunks.resize(Config::B_RANGES.size());

		for (int i = free - 1; i >= 0; i--) {
			freePlaces.push_back(i);
		}
		for (int i = 0; i < Config::B_RANGES.size(); i++) {
			for (int j = Config::B_RANGES[i].second - 1; j >= 0; j--)
			{
				freeChunks[i].push_back(j);
			}
		}
		for (int i = Config::CHUNKS_IN_B - 1; i >= 0; i--)
		{
			freeWater.push_back(i);
		}
		meshRenderer = nullptr;
		waterRenderer = nullptr;
	}
	~ChunksRenderer() {
		delete meshRenderer;
		delete waterRenderer;
	}
};

class RendererManager {
public:
	static int curBuffer;
	static std::map<int, int> buffer_index;
	static std::vector<ChunksRenderer*> buffers;
	static std::queue<Chunk*> needUpdate;
	static bool addBuffer;
	static std::mutex globalMut;

	static void initialize();
	static int getBlockPos(Chunk* chunk);
	static void getChunkPos(Chunk*& chunk);
	static void addRenderer();
	static void addPlace(int cx, int cz, int block);
	static void update(Chunk* &chunk);
	static void loadChunk(Chunk* &chunk);
	static bool checkRenderer(int coord);
	static void removeChunk(Chunk* &chunk, bool mainThread);
	static void draw();
};