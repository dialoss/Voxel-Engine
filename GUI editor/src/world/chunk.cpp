#include "chunk.h"
#include "FastNoiseLite.h"
#include "../utility/debug.h"
#include "config.h"
#include "blocks.h"
#include "lighting.h"
#include "structures.h"
#include <cmath>

void makeCoord(uint& coord, uint x, uint y, uint z) {
	coord |= (x << 24);
	coord |= (y << 8);
	coord |= (z);
}

float getCaveNoise(float x, float y, float z, float offsetX, float offsetY, float offsetZ, float scale){
	FastNoiseLite caves;
	caves.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	return caves.GetNoise((float)(x + offsetX) * scale, (float)(y + offsetY) * scale, (float)(z + offsetZ) * scale);
}

float getHeight(float x, float z, float freq) {
	FastNoiseLite terrain;
	terrain.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	return 1 + terrain.GetNoise((float)x * freq, (float)z * freq);
}

float choiceNoise(float x, float z, float freq) {
	FastNoiseLite terrain;
	terrain.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	return terrain.GetNoise((float)x * freq, (float)z * freq);
}

void Chunk::checkNears(int x, int y, int z) {
	for (int i = 0; i < 6; i++)
	{
		Chunk* chunk = this;
		int nx = x + normals[i][0];
		int ny = y + normals[i][1];
		int nz = z + normals[i][2];
		if (ny < 0 || ny >= H) continue;
		if (voxels[nz + D * (nx + W * ny)] != 0) {
			usint block = (nx << 12) | (ny << 4) | nz;
			updateBlocks.insert(block);
		}
	}
}

void Chunk::setStructure(int ind, int x, const int h, int z) {
	int canSet;
	if (ind <= 2) canSet = rand() % 100;
	else canSet = rand() % 1000;
	if (canSet < 1) {
		for (int i = 0; i < Structures::objects[ind].diffTypes; i++)
		{
			int type = Structures::objects[ind].types[i];
			for (Point p : Structures::objects[ind].points[i]) {
				Chunk* chunk = this;
				int px = p.x + x;
				int py = p.y + h + 1;
				int pz = p.z + z;
				if (py >= H || py < 0) continue;
				if (px < 0 || px >= W || pz < 0 || pz >= D) {
					int mx = MOD(px, 16);
					int mz = MOD(pz, 16);
					int tx = sign(px);
					int tz = sign(pz);
					if (tx == 1 && mx == px) tx = 0;
					if (tz == 1 && mz == pz) tz = 0;
					for (int i = 0; i < v.size(); i++)
					{
						if (tx == v[i][0] && tz == v[i][1]) {
							chunk = chunk->nears[i];
							break;
						}
					}
				}
				px = MOD(px, 16);
				pz = MOD(pz, 16);
				chunk->voxels[pz + D * (px + W * py)] = type;
				usint block = ((usint)px << 12) | ((usint)py << 4) | (usint)pz;
				std::lock_guard<std::mutex> lock(chunkmut);
				chunk->structureBlocks.push(block);
				chunk->changed = true;
			}
		}
	}
}

void getLocals(Chunk* &chunk, int &px, int &pz) {
	int mx = MOD(px, 16);
	int mz = MOD(pz, 16);
	int tx = sign(px);
	int tz = sign(pz);
	if (tx == 1 && mx == px) tx = 0;
	if (tz == 1 && mz == pz) tz = 0;
	for (int i = 0; i < v.size(); i++)
	{
		if (tx == v[i][0] && tz == v[i][1]) {
			chunk = chunk->nears[i];
			break;
		}
	}
	px = MOD(px, 16);
	pz = MOD(pz, 16);
}

void Chunk::proceedGrid(int cx, int cz) {
	int grid = 8;

	int x1 = cx - grid;
	int x2 = cx + grid - 1;
	int z1 = cz - grid;
	int z2 = cz + grid - 1;

	float hs[4];
	for (int i = 0; i < 4; i++)
	{
		Chunk* chunk = this;
		int px = x1 + grid * 2 * uvs[i][0] - 1;
		int pz = z1 + grid * 2 * uvs[i][1] - 1;
		getLocals(chunk, px, pz);
		hs[i] = chunk->heightMap[pz + W * px];
	}

	for (int x = x1; x <= x2; x++)
	{
		for (int z = z1; z <= z2; z++)
		{
			Chunk* chunk = this;
			int px = x;
			int pz = z;

			if (px < 0 || px >= W || pz < 0 || pz >= D) {
				getLocals(chunk, px, pz);
			}

			int h = bilinearInterp(hs[0], hs[1], hs[2], hs[3], x1, x2, z1, z2, x, z);
			chunk->heightMap[pz + W * px] = h;
		}
	}
}

void Chunk::smoothTerrain() {
	proceedGrid(0, 0);
	//proceedGrid(W, 0);
	//proceedGrid(0, D);
	//proceedGrid(W, D);
}

void Chunk::updateVisible() {

	for (int x = 0; x < W; x++)
	{
		for (int z = 0; z < D; z++)
		{
			int h = heightMap[z + W * x];
			int biome = biomeMap[z + W * x];
			if (biome == Biomes::forest) {
				int sType = rand() % Structures::objects.size();
				setStructure(sType, x, h, z);
			}
		}
	}
}

void Chunk::createWorld() {
	srand(this->posX * chunkPos + this->posZ);

	for (int x = 0; x < W; x++)
	{
		for (int z = 0; z < D; z++)
		{
			int realX = this->posX * W + x;
			int realZ = this->posZ * D + z;
			int h = heightMap[z + W * x];
			uint lastHeight = 0;
			int biome = biomeMap[z + W * x];

			if (biome == Biomes::ocean) {
				for (uint y = 0; y <= std::max(h, 35); y++) {
					int pos = z + D * (x + W * y);
					if (y <= h) voxels[pos] = Blocks::sand;
					else if (y <= 35) voxels[pos] = Blocks::water;
					else voxels[pos] = Blocks::sand;
				}
				heightMap[z + W * x] = std::max(h, 35);
				continue;
			}

			for (uint y = 0; y <= h; y++) {
				int pos = z + D * (x + W * y);

				lightMap[pos] = minLight;
				if (y <= 1) {
					voxels[pos] = Blocks::bedrock;
					continue;
				}

				if (biome == Biomes::forest) {
					if (Config::caves) {
						float c1 = getCaveNoise(realX, y, realZ, 0, 0, 0, 7);
						float c2 = getCaveNoise(realX, y, realZ, 10, -5, 100, 5);
						float c3 = getCaveNoise(realX, y, realZ, 100, 100, -100, 2);
						if (c1 < -0.1f && c2 < -0.1f && c3 < 0.1f) {
							lightMap[pos] = 7;
							continue;
						}
					}

					if (y <= h * 0.7f) voxels[pos] = Blocks::stone;
					else if (y < h) voxels[pos] = Blocks::dirt;
					else if (y == h) voxels[pos] = Blocks::grass;
				}
				if (biome == Biomes::desert) {
					if (y <= h * 0.7f) voxels[pos] = Blocks::stone;
					else voxels[pos] = Blocks::sand;
				}

				lastHeight = std::max(lastHeight, y);
			}
			heightMap[z + W * x] = (char)lastHeight;
		}
	}
}

int Chunk::hValue(const int &x, const int &z) {
	return heightMap[z + W * x];
}

void Chunk::defaultWorld() {
	srand(this->posX * chunkPos + this->posZ);

	memset(voxels, 0, W * H * D * sizeof(char));
	memset(positions, -1, W * H * D * sizeof(int));
	memset(numberSides, 0, W * H * D * sizeof(char));
	memset(heightMap, 0, W * D * sizeof(char));
	memset(biomeMap, 0, W * D * sizeof(char));

	for (uint x = 0; x < W; x++) {
		for (uint z = 0; z < D; z++) {
			int realX = this->posX * W + x;
			int realZ = this->posZ * D + z;
			float biome = choiceNoise(realX, realZ, 0.6f) + choiceNoise(realX, realZ, 0.2f) + 2;
			biome *= 100;
			if (biome < 170) biomeMap[z + W * x] = Biomes::ocean;
			else if (biome < 171) biomeMap[z + W * x] = Biomes::desert;
			else if (biome < 180) biomeMap[z + W * x] = Biomes::forest;
			
			uint h1 = getHeight(realX, realZ, 5.0f) * 5;
			uint h2 = getHeight(realX, realZ, 3.0f) * 10;
			uint h3 = getHeight(realX, realZ, 1.0f) * 15;
			uint h4 = getHeight(realX, realZ, 0.3f) * 20;
			uint h = h1 + h2 + h3 + h4;
			h = NoiseGenerator::getHeight(realX, realZ);

			int b = biomeMap[z + W * x];

			//if (b == Biomes::forest) h = base + 0.5f * mount;
			//if (b == Biomes::desert) h = base + 25;
			//if (b == Biomes::ocean) h = bottom + 10;

			heightMap[z + W * x] = h;
		}
	}

	createWorld();
}

void Chunk::flatWorld() {
	for (uint x = 0; x < W; x++) {
		for (uint z = 0; z < D; z++) {
			int h = 6;
			for (uint y = 0; y <= h; y++) {
				lightMap[z + D * (x + W * y)] = minLight;

				if (y == 0) voxels[z + D * (x + W * y)] = Blocks::bedrock;
				else if (y < h) voxels[z + D * (x + W * y)] = Blocks::dirt;
				else if (y == h) voxels[z + D * (x + W * y)] = Blocks::grass;
			}
			heightMap[z + W * x] = h;
		}
	}
}

Chunk::Chunk(int posX, int posZ, int bx, int bz, bool asNear) {
	this->posX = posX;
	this->posZ = posZ;
	this->bx = bx;
	this->bz = bz;
	this->asNear = asNear;
	posInBuffer = -1;
	waterPos = -1;
	removed = false;
	meshed = false;
	needUpdate = false;
	updateLight = false;
	loaded = false;
	changed = false;

	nears = new Chunk * [8];
	for (int i = 0; i < 8; i++)
	{
		nears[i] = nullptr;
	}

	positions = new int[W * H * D];
	voxels = new char[W * H * D];
	numberSides = new char[W * H * D];
	heightMap = new char[W * D];
	biomeMap = new char[W * D];
	lightMap = new usint[W * H * D];

	for (int i = 0; i < W * H * D; i++) lightMap[i] = 15;
}

void Chunk::create() {
	if (Config::flatWorld) flatWorld();
	else defaultWorld();
}

void Chunk::clearBuffers() {
	toAdd.clear();
	toRemove.clear();
	changedCount = 0;
}

Chunk::~Chunk() {
	toAdd.clear();
	toRemove.clear();
	bufferPos.clear();
	delete[] voxels;
	delete[] positions;
	delete[] numberSides;
	delete[] lightMap;
	delete[] nears;
	removed = true;
}