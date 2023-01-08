#include "chunkManager.h"
#include "../graphics/mesh.h"
#include "lighting.h"
#include "config.h"
#include "blocks.h"

void createChunk(Chunk* chunk) {
	chunk->create();
}

void updateVisible(Chunk* chunk) {
	chunk->updateVisible();
}

ChunkManager::ChunkManager() {
	chunks = new Chunk * [Config::b_size];
	meshes = new Mesh * [Config::b_size];
	ctemp = new Chunk * [Config::b_size];
	mtemp = new Mesh * [Config::b_size];
	rx = Config::rx;
	rz = Config::rz;
	wx = Config::wx;
	wz = Config::wz;
	BW = Config::BW;
	maxSurfaces = glm::ivec3(0);
	visibleSurfaces = 0;

	workers = std::thread::hardware_concurrency() - 1;
	tp = new thread_pool(workers);

	for (int x = -rx; x <= rx; x++) {
		for (int z = -rz; z <= rz; z++) {
			int coord = z + rz + BW * (x + rx);
			if (x < -wx || x > wx || z < -wz || z > wz) chunks[coord] = new Chunk(x, z, x + rx, z + rz, true);
			else chunks[coord] = new Chunk(x, z, x + rx, z + rz, false);
			tp->add_task(createChunk, chunks[coord]);
		}
	}
	tp->wait_all();
}
std::mutex glmut;

void initWorld(ChunkManager* world, int coord) {
	Chunk* chunk = world->chunks[coord];
	world->meshes[coord] = new Mesh(world, chunk);
	chunk->mesh = world->meshes[coord];

	if (chunk->asNear) return;
	world->getNears(chunk);

	if (Config::curType != "struct") {
		//chunk->updateVisible();
		if (chunk->changed) {
			std::lock_guard<std::mutex> lock(glmut);
			world->updateStruct.push(chunk);
		}
	}
	
 	LightManager::proceedSunLight(chunk);
	world->meshes[coord]->create(chunk);
	
	std::lock_guard<std::mutex> lock(glmut);
	world->toAdd.push(chunk);
}

void createMesh(ChunkManager* world, int coord) {
	Chunk* chunk = world->chunks[coord];

	world->getNears(chunk);

	if (Config::curType != "struct") {
		//chunk->updateVisible();
	}
	
	LightManager::proceedSunLight(chunk);
	chunk->mesh = world->meshes[coord];
	world->meshes[coord]->create(chunk);

	chunk->asNear = false;
	chunk->meshed = true;
	
	std::lock_guard<std::mutex> lock(glmut);
	world->toAdd.push(chunk);
}

void ChunkManager::init() {
	for (int x = -rx; x <= rx; x++) {
		for (int z = -rz; z <= rz; z++) {
			int coord = z + rz + BW * (x + rx);
			tp->add_task(initWorld, this, coord);	
		}
	}
	tp->wait_all();
}

void ChunkManager::reload() {
	for (int x = -rx; x <= rx; x++) {
		for (int z = -rz; z <= rz; z++) {
			int coord = z + rz + BW * (x + rx);
			if (chunks[coord]->asNear) continue;
			createChunk(chunks[coord]);
			tp->add_task(createMesh, this, coord);
		}
	}
	tp->wait_all();
}

void ChunkManager::shift(int dx, int dz) {
	int sx = sign(dx);
	int sz = sign(dz);
	bool swaped = false;
  	for (int x = -rx; x <= rx; x++) {
		for (int z = -rz; z <= rz; z++) {
			if (dz != 0) {
				std::swap(x, z);
			}
			if (dz != 0) z *= sz;
			else x *= sx;
			int pc = z + rz + BW * (x + rx);
			int nz = z + dz;
			int nx = x + dx;
			if (nx < -wx || nz < -wz || nx > wx || nz > wz) {
				if (chunks[pc]->loaded) 
					toUnload.push(chunks[pc]);
				chunks[pc]->asNear = true;
			}
			if (nx > rx || nx < -rx || nz > rz || nz < -rz) {
				toRemove.push(chunks[pc]);
				if (nx > rx || nx < -rx) {
					nx += -sign(nx) * BW;
				}
				if (nz > rz || nz < -rz) {
					nz += -sign(nz) * BW;
				}
				int bz = nz + rz;
				int bx = nx + rx;
				Chunk* chunk = new Chunk(centerX + nx, centerZ + nz, nx, nz, true);
				chunk->create();
				ctemp[bz + BW * bx] = chunk;
				mtemp[bz + BW * bx] = meshes[pc];
			} else {
				int bz = nz + rz;
				int bx = nx + rx;
				Chunk* chunk = chunks[pc];
				ctemp[bz + BW * bx] = chunk;
				mtemp[bz + BW * bx] = meshes[pc];
			}
			if (dz != 0) z *= sz;
			else x *= sx;
			if (dz != 0) {
				std::swap(x, z);
			}
		}
	}
	for (int i = 0; i < Config::b_size; i++) {
		chunks[i] = ctemp[i];
	}
	for (int i = 0; i < Config::b_size; i++) {
		meshes[i] = mtemp[i];
	}
	int count = 0;
	for (int x = -wx; x <= wx; x++) {
		for (int z = -wz; z <= wz; z++) {
			if (dz != 0) {
				std::swap(x, z);
			}
			if (dz != 0) z *= sz;
			else x *= sx;
			int bz = z + rz;
			int bx = x + rx;
			int coord = bz + BW * bx;
			Chunk* chunk = ctemp[coord];
			if (chunk->asNear) {
				tp->add_task(createMesh, this, coord);
			}
			if (dz != 0) z *= sz;
			else x *= sx;
			if (dz != 0) {
				std::swap(x, z);
			}
			count++;
			if (count == 2 * wx + 1) x = wx + 1;
		}
	}
	tp->wait_all();
}

using namespace std::chrono_literals;
void ChunkManager::translate(glm::ivec3 delta) {
	if (delta.x == 0 && delta.z == 0) return;
	int sx = sign(delta.x);
	int sz = sign(delta.z);
	for (int dx = 1; dx <= abs(delta.x); dx++)
	{
		centerX += sx;
		shift(-sx, 0);
	}
	for (int dz = 1; dz <= abs(delta.z); dz++)
	{
		centerZ += sz;
		shift(0, -sz);
	}
}

void ChunkManager::getNears(Chunk* chunk) {
	int cx = chunk->posX;
	int cz = chunk->posZ;
	for (int j = 0; j < v.size(); j++) {
		int nx = cx + v[j][0];
		int nz = cz + v[j][1];
		if (abs(nx - centerX) > rx || abs(nz - centerZ) > rz) continue;
		Chunk* near = getChunk(nx, nz);
		chunk->nears[j] = near;
	}	
}

Chunk* ChunkManager::getChunk(const int& cx, const int& cz) {
	if (abs(cx - centerX) > rx || abs(cz - centerZ) > rz) return nullptr;
	Chunk* chunk = chunks[cz - centerZ + rz + BW * (cx - centerX + rx)];
	return chunk;
}

Chunk* ChunkManager::getChunk(const int &x, const int &y, const int &z) {
	if (y < 0 || y > H) return nullptr;
	int cx = x / W;
	int cz = z / D;
	if (x < 0 && abs(x) % 16 != 0) cx--;
	if (z < 0 && abs(z) % 16 != 0) cz--;
	if (abs(cx - centerX) > rx || abs(cz - centerZ) > rz) return nullptr;
	Chunk* chunk = chunks[cz - centerZ + rz + BW * (cx - centerX + rx)];
	return chunk;
}

usint ChunkManager::getLight(const int &x, const int& y, const int& z, Chunk* chunk, const int& channel) {
	usint val = chunk->lightMap[z + D * (x + W * y)];
	return (val & (0xF << channel * 4)) >> channel * 4;
}

usint ChunkManager::getLight(const int& x, const int& y, const int& z, const int &channel) {
	if (y < 0 || y >= H) return 0;
	int cx = x / W;
	int cz = z / D;
	if (x < 0 && abs(x) % 16 != 0) cx--;
	if (z < 0 && abs(z) % 16 != 0) cz--;
	int vx = x - cx * W;
	int vz = z - cz * D;
	Chunk* chunk = getChunk(x, y, z);
	if (chunk == nullptr) return 0;
	usint val = chunk->lightMap[vz + D * (vx + W * y)];
	return (val & (0xF << channel * 4)) >> channel * 4;
}

char* ChunkManager::getVoxel(int x, int y, int z) {
	if (y < 0 || y >= H) return nullptr;
	int cx = x / W;
	int cz = z / D;
	if (x < 0 && abs(x) % 16 != 0) cx--;
	if (z < 0 && abs(z) % 16 != 0) cz--;
	int vx = x - cx * W;
	int vz = z - cz * D;
	Chunk* chunk = getChunk(x, y, z);
	if (chunk == nullptr) return nullptr;
	return &chunk->voxels[vz + D * (vx + W * y)];
}

char* ChunkManager::rayCast(glm::vec3 pos, glm::vec3 dir, float maxDist, glm::ivec3& norm, glm::ivec3& iend, std::vector<glm::ivec3> &positions, bool log) {
	float px = pos.x;
	float py = pos.y;
	float pz = pos.z;

	float dx = dir.x;
	float dy = dir.y;
	float dz = dir.z;

	float t = 0.0f;
	int ix = floor(px);
	int iy = floor(py);
	int iz = floor(pz);

	float stepx = (dx > 0.0f) ? 1.0f : -1.0f;
	float stepy = (dy > 0.0f) ? 1.0f : -1.0f;
	float stepz = (dz > 0.0f) ? 1.0f : -1.0f;

	float infinity = std::numeric_limits<float>::infinity();

	float txDelta = (dx == 0.0f) ? infinity : abs(1.0f / dx);
	float tyDelta = (dy == 0.0f) ? infinity : abs(1.0f / dy);
	float tzDelta = (dz == 0.0f) ? infinity : abs(1.0f / dz);

	float xdist = (stepx > 0) ? (ix + 1 - px) : (px - ix);
	float ydist = (stepy > 0) ? (iy + 1 - py) : (py - iy);
	float zdist = (stepz > 0) ? (iz + 1 - pz) : (pz - iz);

	float txMax = (txDelta < infinity) ? txDelta * xdist : infinity;
	float tyMax = (tyDelta < infinity) ? tyDelta * ydist : infinity;
	float tzMax = (tzDelta < infinity) ? tzDelta * zdist : infinity;

	int steppedIndex = -1;
	int step = 0;

	while (t <= maxDist) {
		step++;
		char* voxel = getVoxel(ix, iy, iz);
		if (log && step == raycastStep) {
			positions.push_back(glm::vec3(ix, iy, iz));
			step = 0;
		}
		if (!log && (voxel == nullptr || BlockManager::blocks[*voxel].solid)) {
			iend.x = ix;
			iend.y = iy;
			iend.z = iz;

			norm.x = norm.y = norm.z = 0.0f;
			if (steppedIndex == 0) norm.x = -stepx;
			if (steppedIndex == 1) norm.y = -stepy;
			if (steppedIndex == 2) norm.z = -stepz;
			return voxel;
		}

		if (txMax < tyMax) {
			if (txMax < tzMax) {
				ix += stepx;
				t = txMax;
				txMax += txDelta;
				steppedIndex = 0;
			} else {
				iz += stepz;
				t = tzMax;
				tzMax += tzDelta;
				steppedIndex = 2;
			}
		} else {
			if (tyMax < tzMax) {
				iy += stepy;
				t = tyMax;
				tyMax += tyDelta;
				steppedIndex = 1;
			} else {
				iz += stepz;
				t = tzMax;
				tzMax += tzDelta;
				steppedIndex = 2;
			}
		}
	}
	return nullptr;
}