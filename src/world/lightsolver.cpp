#include "lightsolver.h"
#include "../utility/variables.h"
#include "../utility/geometry.h"
#include "blocks.h"
#include "chunk.h"
#include "../events.h"
#include <cmath>

void LightSolver::setLight(Chunk*& chunk, const int& x, const int& y, const int& z, const usint& value) {
    chunk->lightMap[z + D * (x + W * y)] &= (0xFFFF - (usint)pow(16, channel + 1) + (usint)pow(16, channel));
    chunk->lightMap[z + D * (x + W * y)] |= (value << channel * 4);
}

void LightSolver::remove(Chunk*& chunk, const int& x, const int& y, const int& z) {
    std::lock_guard<std::mutex> lock(queueMut);
    int light = world->getLight(x + chunk->posX * W, y, z + chunk->posZ * D, channel);
    removeLight.push(LightCell(x + chunk->posX * W, y, z + chunk->posZ * D, light));
    setLight(chunk, x, y, z, minLight);
}

void LightSolver::add(Chunk*& chunk, const int& x, const int& y, const int& z, const int& emission) {
    if (emission <= 1) return;
    std::lock_guard<std::mutex> lock(queueMut);
    addLight.push(LightCell(x + chunk->posX * W, y, z + chunk->posZ * D, emission));
    setLight(chunk, x, y, z, emission);
}

void LightSolver::add(const int& x, const int& y, const int& z) {
    Chunk* chunk = world->getChunk(x, y, z);
    if (chunk == nullptr) return;
    int emission = world->getLight(x, y, z, channel);
    add(chunk, x - chunk->posX * W, y, z - chunk->posZ * D, emission);
}

void LightSolver::updateLights(bool newChunk) {
    std::lock_guard<std::mutex> lock(queueMut);

    std::vector<Chunk*> toUpdate;
    while (!removeLight.empty()) {
        LightCell source = removeLight.front();
        removeLight.pop();

        for (int i = 0; i < 6; i++) {
            int x = source.x + normals[i][0];
            int y = source.y + normals[i][1];
            int z = source.z + normals[i][2];

            Chunk* chunk = world->getChunk(x, y, z);
            if (chunk == nullptr) continue;

            int vx = x - chunk->posX * W;
            int vz = z - chunk->posZ * D;

            char* voxel = world->getVoxel(x, y, z);
            if (voxel == nullptr) continue;
            if (*voxel && !newChunk) {
                usint block = ((usint)vx << 12) | ((usint)y << 4) | (usint)vz;
                chunk->updateBlocks.insert(block);
                if (!chunk->updateLight && chunk->mesh != nullptr && !chunk->mesh->newChunk) {
                    if (!chunk->updateLight) {
                        chunk->updateLight = true;
                        toUpdate.push_back(chunk);
                    }
                }
                
                continue;
            }

            usint light = world->getLight(x, y, z, channel);

            if (light > minLight && light < source.light) {
                setLight(chunk, vx, y, vz, 0);
                LightCell newlight(x, y, z, light);
                removeLight.push(newlight);
            }
            if (light >= source.light) {
                LightCell newlight(x, y, z, light);
                addLight.push(newlight);
            }
            if (light == minLight) {
                voxel = world->getVoxel(x, y - 1, z);
                if (voxel && *voxel) {
                    usint block = ((usint)vx << 12) | ((usint)(y - 1) << 4) | (usint)vz;
                    chunk->updateBlocks.insert(block);
                }
                voxel = world->getVoxel(x, y + 1, z);
                if (voxel && *voxel) {
                    usint block = ((usint)vx << 12) | ((usint)(y + 1) << 4) | (usint)vz;
                    chunk->updateBlocks.insert(block);
                }
            }
        }
    }

    while (!addLight.empty()) {
        LightCell source = addLight.front();
        addLight.pop();

        for (int i = 0; i < 6; i++) {
            int x = source.x + normals[i][0];
            int y = source.y + normals[i][1];
            int z = source.z + normals[i][2];

            Chunk* chunk = world->getChunk(x, y, z);
            if (chunk == nullptr) continue;
            
            int vx = x - chunk->posX * W;
            int vz = z - chunk->posZ * D;

            char* voxel = world->getVoxel(x, y, z);
            if (voxel == nullptr) continue;
            if (*voxel && !newChunk) {
                usint block = ((usint)vx << 12) | ((usint)y << 4) | (usint)vz;
                chunk->updateBlocks.insert(block);
                if (!chunk->updateLight && chunk->mesh != nullptr && !chunk->mesh->newChunk) {
                    if (!chunk->updateLight) {
                        chunk->updateLight = true;
                        toUpdate.push_back(chunk);
                    }
                }
                continue;
            }

            usint light = world->getLight(x, y, z, channel);

            if (BlockManager::opacity[*voxel] && light + 2 <= source.light) {
                int newLight = source.light - 1;
                if (Blocks::leaves == *voxel) newLight = std::max(5, newLight);
                if (Blocks::glass == *voxel) newLight = source.light;

                setLight(chunk, vx, y, vz, newLight);
                LightCell newlight(x, y, z, newLight);
                addLight.push(newlight);
            }
        }
    }
    for (Chunk* chunk : toUpdate) world->toUpdate.push(chunk);
}