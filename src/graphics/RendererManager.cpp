#include "RendererManager.h"
#include "../utility/geometry.h"
#include "../utility/variables.h"
#include "../world/config.h"
#include "../events.h"
#include <thread>
#include <iostream>

int RendererManager::curBuffer = -1;
std::map<int, int> RendererManager::buffer_index;
std::vector<ChunksRenderer*> RendererManager::buffers;
bool RendererManager::addBuffer = false;
std::queue<Chunk*> RendererManager::needUpdate;
std::mutex RendererManager::globalMut;

using namespace std::chrono_literals;

void RendererManager::addRenderer() {
	uint* global_buffer = new uint[Config::B_SIZE];
	memset(global_buffer, 0, Config::B_START * sizeof(uint));
	Renderer* meshRend = new Renderer(global_buffer, Config::B_VCOUNT, attrs);
	buffers[curBuffer]->meshRenderer = meshRend;
	delete[] global_buffer;

	uint* water_buffer = new uint[Config::CHUNKS_IN_B * Config::WATER_B_SIZE * 6 * Config::V_SIZE];
	memset(water_buffer, 0, Config::CHUNKS_IN_B * Config::WATER_B_SIZE * 6 * Config::V_SIZE * sizeof(uint));
	Renderer* waterRend = new Renderer(water_buffer, Config::CHUNKS_IN_B * Config::WATER_B_SIZE * 6, attrs);
	buffers[curBuffer]->waterRenderer = waterRend;
	delete[] water_buffer;
}

void RendererManager::initialize() {
	curBuffer++;
	buffers.push_back(new ChunksRenderer());
	addRenderer();
}

void RendererManager::addPlace(int cx, int cz, int block) {
	int coord = cx * chunkPos + cz;
	int ind = buffer_index[coord];
	ChunksRenderer* r = buffers[ind];
}

bool RendererManager::checkRenderer(int coord) {
	return RendererManager::buffers[coord]->meshRenderer != nullptr;
}

void RendererManager::removeChunk(Chunk* &chunk, bool mainThread) {
	int coord = chunk->posX * chunkPos + chunk->posZ;
	int ind = 0;// buffer_index[coord];
	ChunksRenderer* r = buffers[ind];
	int pos = chunk->posInBuffer;
	int waterPos = chunk->waterPos;
	if (pos == -1) return;
	chunk->posInBuffer = -1;

	if (mainThread) {
		buffer_index.erase(coord);
		r->meshRenderer->loadChunk(Config::clearChunks[chunk->range], pos * Config::B_RANGES[chunk->range].first + Config::B_OFFSETS[chunk->range], Config::B_RANGES[chunk->range].first);
		r->freeChunks[chunk->range].push_back(pos);
		if (waterPos != -1) {
			r->waterRenderer->loadChunk(Config::clearWater, waterPos * Config::WATER_B_SIZE, Config::WATER_B_SIZE);
			r->freeWater.push_back(waterPos);
		}
		for (int block : chunk->bufferPos) {
			r->meshRenderer->removeBlock(chunk->positions[block]);
			r->freePlaces.push_back(block);
		}
	}
	else {
		/*for (int i = pos * Variables::chunk_buffer_size; i < (pos + 1) * Variables::chunk_buffer_size; i++) {
			r->renderer->newPos[i] = true;
		}
		for (int blockPos : chunk->bufferPos) {
			r->freePlaces.push(pos);
			r->free++;
			r->renderer->newPos[pos] = true;
		}*/
	}
	
}

int RendererManager::getBlockPos(Chunk* chunk) {
	int coord = chunk->posX * chunkPos + chunk->posZ;
	int ind = 0;// buffer_index[coord];

	ChunksRenderer* r = buffers[ind];
	int pos = r->freePlaces.back();
	pos += Config::B_START;
	r->freePlaces.pop_back();

	return pos;
}

void RendererManager::getChunkPos(Chunk* &chunk) {
	std::lock_guard<std::mutex> lock(globalMut);
	int coord = chunk->posX * chunkPos + chunk->posZ;
	//buffer_index[coord] = curBuffer;

	ChunksRenderer* r = buffers[curBuffer];
	int pos = 0;
	int start = chunk->range;
	for (int i = start; i < Config::B_RANGES.size(); i++)
	{
		if (r->freeChunks[i].size() != 0) {
			pos = r->freeChunks[i].back();
			chunk->range = i;
			break;
		}
	}
	
	r->freeChunks[chunk->range].pop_back();
	r->chunks++;
	
	chunk->posInBuffer = pos;
	if (chunk->waterSurfaces != 0) {
		int waterPos = r->freeWater.back();
		r->freeWater.pop_back();
		chunk->waterPos = waterPos;
	}
}

void RendererManager::loadChunk(Chunk* &chunk) {
 	int coord = chunk->posX * chunkPos + chunk->posZ;
	int ind = 0;// buffer_index[coord];
	ChunksRenderer* r = buffers[ind];
	int pos = chunk->posInBuffer * Config::B_RANGES[chunk->range].first + Config::B_OFFSETS[chunk->range];
	r->meshRenderer->loadChunk(chunk->mesh->meshBuffer, pos, Config::B_RANGES[chunk->range].first);
	r->globalSidePos += chunk->visibleSurfaces;
	int waterPos = chunk->waterPos;
	if (chunk->waterSurfaces != 0) {
		r->waterRenderer->loadChunk(chunk->mesh->waterBuffer, waterPos * Config::WATER_B_SIZE, Config::WATER_B_SIZE);
	}
}

void RendererManager::update(Chunk* &chunk) {
	int coord = chunk->posX * chunkPos + chunk->posZ;
	int ind = 0;// buffer_index[coord];
	buffers[ind]->meshRenderer->removeSides(chunk->toRemove);
	buffers[ind]->meshRenderer->update(chunk->mesh->meshBuffer, chunk->toAdd);
}

void RendererManager::draw() {
	for (int i = 0; i < buffers.size(); i++)
	{
		buffers[i]->meshRenderer->draw(GL_TRIANGLES, false);
		buffers[i]->waterRenderer->draw(GL_TRIANGLES, true);
	}
}