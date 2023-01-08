#include "mesh.h"
#include "RendererManager.h"
#include "../world/blocks.h"
#include "../world/lighting.h"
#include "../utility/geometry.h"
#include <chrono>
#include "../utility/debug.h"
#include "../world/config.h"
#include "RendererManager.h"
#include <cmath>

inline void setRGB(const uint &r, const uint& g, const uint& b, uint& val) {
    val |= (r << 24);
    val |= (g << 16);
    val |= (b << 8);
}

inline void setS(const uint& s, uint& val) {
    val |= (s << 24);
}

inline void setUV(const uint& tx, const uint& ty, const uint& ind, uint& val) {
    val |= (tx << 31);
    val |= (ty << 30);
    val |= (ind << 20);
}

void Mesh::initialize(ChunkManager* world) {
    this->world = world;
}

void Mesh::resetBuffer() {
    memset(meshBuffer, 0, Config::CHUNK_B_SIZE * 6 * Config::V_SIZE * sizeof(uint));
    memset(waterBuffer, 0, Config::WATER_B_SIZE * 6 * Config::V_SIZE * sizeof(uint));
}

Mesh::Mesh(ChunkManager* world, Chunk* chunk) {
    this->world = world;
    this->chunk = chunk;
    newChunk = true;
    meshBuffer = new uint[Config::CHUNK_B_SIZE * 6 * Config::V_SIZE];
    waterBuffer = new uint[Config::WATER_B_SIZE * 6 * Config::V_SIZE];
    memset(meshBuffer, 0, Config::CHUNK_B_SIZE * 6 * Config::V_SIZE * sizeof(uint));
    memset(waterBuffer, 0, Config::WATER_B_SIZE * 6 * Config::V_SIZE * sizeof(uint));
}

void getCoords(const uint &coord, uint &x, uint &y, uint &z) {
    x = ((coord & 0xFF000000u) >> 24);
    y = ((coord & 0xFFFF00u) >> 8);
    z = ((coord & 0xFF));
}

void Mesh::loadStructures(Chunk* chunk) {
    this->chunk = chunk;
    if (chunk->loaded) newChunk = false;
    while (chunk->structureBlocks.size()) {
        usint block = chunk->structureBlocks.front();
        chunk->structureBlocks.pop();
        int x = (block & 0xF000u) >> 12;
        int y = (block & 0xFF0u) >> 4;
        int z = (block & 0xFu);
        int ind = chunk->voxels[z + D * (x + W * y)];
        bool visible = false;
        int count = 0;
        int start = chunk->visibleSurfaces;
        for (int g = 0; g < 6; g++) {
            if (addSide(chunk, g, x, y, z, BlockManager::sides[ind][g])) {
                visible = true;
                chunk->visibleSurfaces++;
                if (!newChunk) {
                    chunk->toAdd.push_back({ chunk->positions[z + D * (x + W * y)], count });
                    count++;
                }
            }
        }
        if (visible) {
            if (newChunk) {
                chunk->positions[z + D * (x + W * y)] = start;
                updatePos.push_back(z + D * (x + W * y));
            }
            else {
                chunk->changedCount++;
            }            
        }
    }
}

void Mesh::create(Chunk* chunk) {
    this->chunk = chunk;
    newChunk = true;

    auto start = std::chrono::steady_clock::now();

    RendererManager::buffers[0]->freeChunks[chunk->range].push_back(chunk->posInBuffer);
    RendererManager::buffers[0]->freeWater.push_back(chunk->waterPos);

    chunk->clearBuffers();
    chunk->updateBlocks.clear();
    chunk->mesh->resetBuffer();
    RendererManager::buffers[0]->meshRenderer->loadChunk(Config::clearChunks[chunk->range], chunk->posInBuffer * Config::B_RANGES[chunk->range].first + Config::B_OFFSETS[chunk->range], Config::B_RANGES[chunk->range].first);

    chunk->changedCount = 0;
    chunk->visibleSurfaces = 0;
    chunk->waterSurfaces = 0;
   
    for (int x = 0; x < W; x++) {
        for (int z = 0; z < D; z++) {
            int h =  chunk->heightMap[z + W * x];
            for (int y = 0; y <= h; y++)
            {
                int ind = chunk->voxels[z + D * (x + W * y)];
                if (ind == 0) continue;
                bool visible = false;
                if (ind == Blocks::water) {
                    int start = chunk->waterSurfaces;
                    if (addSide(chunk, 4, x, y, z, BlockManager::sides[ind][4])) {
                        visible = true;
                        chunk->waterSurfaces++;
                    }
                    if (visible) {
                        chunk->positions[z + D * (x + W * y)] = start;
                        updatePos.push_back(z + D * (x + W * y));
                    }
                }
                else {
                    int start = chunk->visibleSurfaces;
                    for (int g = 0; g < 6; g++) {
                        if (addSide(chunk, g, x, y, z, BlockManager::sides[ind][g])) {
                            visible = true;
                            chunk->visibleSurfaces++;
                        }
                    }
                    if (visible) {
                        chunk->positions[z + D * (x + W * y)] = start;
                        updatePos.push_back(z + D * (x + W * y));
                    }
                } 
            }
        }
    }

    loadStructures(chunk);

    for (int i = 1; i < Config::B_RANGES.size(); i++)
    {
        if (chunk->visibleSurfaces >= Config::B_RANGES[i - 1].first && chunk->visibleSurfaces < Config::B_RANGES[i].first) chunk->range = i;
    }

    chunk->prevPos = chunk->posInBuffer;
    RendererManager::getChunkPos(chunk);

    for (int pos : updatePos) {
        if (chunk->voxels[pos] == Blocks::water) 
            chunk->positions[pos] += Config::WATER_B_SIZE * chunk->waterPos;
        else chunk->positions[pos] += chunk->posInBuffer * Config::B_RANGES[chunk->range].first + Config::B_OFFSETS[chunk->range];
    }
    updatePos.clear();

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << Debug::update(elapsed_seconds.count() * 1000) << " ms\n";
}

bool Mesh::checkNearSide(Chunk* &chunk, const int& g, const int& x, const int& y, const int& z, const int& ind) {
    int xn = x + normals[g][0];
    int yn = y + normals[g][1];
    int zn = z + normals[g][2];
    int dg = BlockManager::drawGroup[ind];
    if (xn < 0 || zn < 0 || xn >= W || zn >= D) {
        xn = MOD(xn, 16);
        zn = MOD(zn, 16);
        int voxel = chunk->nears[g]->voxels[zn + D * (xn + W * yn)];
        int opacity = BlockManager::opacity[voxel];
        int dg_near = BlockManager::drawGroup[voxel];
        if (dg == 2 && dg_near == 2) return true;
        return opacity == 0;
    }
    if (yn < 0 || yn >= H) return true;
    int voxel = chunk->voxels[zn + D * (xn + W * yn)];
    int opacity = BlockManager::opacity[voxel];
    int dg_near = BlockManager::drawGroup[voxel];
    if (dg == 2 && dg_near == 2) return true;
    return opacity == 0;
}

bool Mesh::addSide(Chunk* &chunk, const int& g, const int& x, const int& y, const int& z, const int& uv) {
    if (checkNearSide(chunk, g, x, y, z, chunk->voxels[z + D * (x + W * y)])) return false;
    int pos = chunk->positions[z + D * (x + W * y)];
    if (pos == -1) {
        if (!Mesh::newChunk) {
            pos = RendererManager::getBlockPos(chunk);
            chunk->bufferPos.insert(z + D * (x + W * y));
            chunk->positions[z + D * (x + W * y)] = pos;
        }
    }

    if (pos < Config::B_START && !newChunk) {
        for (int i = 0; i < chunk->numberSides[z + D * (x + W * y)]; i++)
        {
            chunk->toRemove.push_back({ pos, i });
        }
        pos = RendererManager::getBlockPos(chunk);
        chunk->positions[z + D * (x + W * y)] = pos;
        chunk->bufferPos.insert(z + D * (x + W * y));
    }

    chunk->numberSides[z + D * (x + W * y)] += 1;
        
    updateSide(chunk, g, x, y, z, uv);

    return true;
}

void Mesh::updateSide(Chunk*& chunk, const int& g, const int& x, const int& y, const int& z, const int& uv) {
    int pos = chunk->positions[z + D * (x + W * y)];
    if (pos == -1) {
        if (!Mesh::newChunk) {
            pos = RendererManager::getBlockPos(chunk);
            chunk->bufferPos.insert(z + D * (x + W * y));
            chunk->positions[z + D * (x + W * y)] = pos;
        }
    }
    int ind = chunk->voxels[z + D * (x + W * y)];
    int buffer_pos;
    if (Mesh::newChunk) {
        if (ind == Blocks::water) buffer_pos = chunk->waterSurfaces * 6 * Config::V_SIZE;
        else buffer_pos = chunk->visibleSurfaces * 6 * Config::V_SIZE;
    }
    else buffer_pos = chunk->changedCount * 36 * Config::V_SIZE + g * 6 * Config::V_SIZE;
    
    // OPTIMIZE LIGHT SELECTION. ALREADY HAVE NEAR CHUNKS

    int gx = x + chunk->posX * W;
    int gy = y;
    int gz = z + chunk->posZ * D;

    for (int k = 0; k < cube[g].size(); k++) {
        usint R = 0, G = 0, B = 0, S = 0;
        uint vx = cube[g][k][0] + x + chunk->posX * W + coord_const;
        uint vy = cube[g][k][1] + y;
        uint vz = cube[g][k][2] + z + chunk->posZ * D + coord_const;

        for (int i = 0; i < 4; i++) {
            int nx = lightSides[g][k][i][0] + gx;
            int ny = lightSides[g][k][i][1] + gy;
            int nz = lightSides[g][k][i][2] + gz;

            int lx = nx - chunk->posX * W;
            int ly = ny;
            int lz = nz - chunk->posZ * D;

            usint red, green, blue, s;

            if (lx < 0 || lx >= W || lz < 0 || lz >= D) {
                red = LightManager::getLight(nx, ny, nz, 3);
                green = LightManager::getLight(nx, ny, nz, 2);
                blue = LightManager::getLight(nx, ny, nz, 1);
                s = LightManager::getLight(nx, ny, nz, 0);
            }
            else {
                red = world->getLight(lx, ly, lz, chunk, 3);
                green = world->getLight(lx, ly, lz, chunk, 2);
                blue = world->getLight(lx, ly, lz, chunk, 1);
                s = world->getLight(lx, ly, lz, chunk, 0);
            }
            if (i == 0) {
                R += 2 * red; G += 2 * green; B += 2 * blue; S += 2 * s;
            }
            else {
                R += red; G += green; B += blue; S += s;
            }
        }
        if (g < 4 && S > 45) S = 45;
        if (g == 5 && S > 30) S = 30;
        S = std::min(S, (usint)60);
        setS(S, vx);
        setRGB(R, G, B, vy);
        setUV(cube[g][k][3], cube[g][k][4], uv, vz);
        if (ind == Blocks::water) {
            chunk->mesh->waterBuffer[buffer_pos] = vx;
            chunk->mesh->waterBuffer[buffer_pos + 1] = vy;
            chunk->mesh->waterBuffer[buffer_pos + 2] = vz;
        }
        else {
            chunk->mesh->meshBuffer[buffer_pos] = vx;
            chunk->mesh->meshBuffer[buffer_pos + 1] = vy;
            chunk->mesh->meshBuffer[buffer_pos + 2] = vz;
        }
        buffer_pos += Config::V_SIZE;
    }
}

void Mesh::removeSide(Chunk* &chunk, int g, int x, int y, int z) {
    int pos = chunk->positions[z + D * (x + W * y)];
    if (pos == -1) return;
    chunk->toRemove.push_back({ pos, g });
    chunk->numberSides[z + D * (x + W * y)]--;
    if (chunk->numberSides[z + D * (x + W * y)] == 0) {
        chunk->bufferPos.erase(z + D * (x + W * y));
        chunk->positions[z + D * (x + W * y)] = -1;
    }
}

void Mesh::updateNears(int x, int y, int z) {
    int mid = chunk->voxels[z + D * (x + W * y)];
    for (int i = 0; i < 6; i++) {
        Chunk* chunk = Mesh::chunk;
        int nx = x + normals[i][0];
        int ny = y + normals[i][1];
        int nz = z + normals[i][2];
        if (nx < 0 || nz < 0 || nx >= W || nz >= D) {
            chunk = chunk->nears[i];
            nx = MOD(nx, 16);
            nz = MOD(nz, 16);
        }
        if (chunk == nullptr || ny < 0 || ny >= H) continue;
        int ind = chunk->voxels[nz + D * (nx + W * ny)];
        int pos = chunk->positions[nz + D * (nx + W * ny)];
        if (ind > 0) {
            bool changed = false;
            if (pos < Config::B_START) {
                for (int g = 0; g < 6; g++)
                {
                    if (addSide(chunk, g, nx, ny, nz, BlockManager::sides[ind][g])) {
                        changed = true;
                        chunk->toAdd.push_back({ chunk->positions[nz + D * (nx + W * ny)], g });
                    }
                }
            }
            else {
                if (BlockManager::drawGroup[ind] != 2) {
                    if (addSide(chunk, sides[i], nx, ny, nz, BlockManager::sides[ind][sides[i]])) {
                        changed = true;
                        chunk->toAdd.push_back({ chunk->positions[nz + D * (nx + W * ny)], sides[i] });
                    }
                }
                else removeSide(chunk, sides[i], nx, ny, nz);
            }

            if (changed) chunk->changedCount++;
            if (chunk != Mesh::chunk && !chunk->needUpdate) chunk->needUpdate = true;
        }
    }
}

void Mesh::removeBlock(int x, int y, int z, Chunk* chunk) {
    Mesh::chunk = chunk;
    Mesh::newChunk = false;
    int pos = chunk->positions[z + D * (x + W * y)];
    if (pos == -1) return;
    if (pos < Config::B_START) {
        for (int i = 0; i < chunk->numberSides[z + D * (x + W * y)]; i++)
        {
            chunk->toRemove.push_back({ pos, i });
        }
    }
    else {
        for (int i = 0; i < 6; i++)
        {
            chunk->toRemove.push_back({ pos, i });
        }
    }
    
    chunk->positions[z + D * (x + W * y)] = -1;
    chunk->numberSides[z + D * (x + W * y)] = 0;
    updateNears(x, y, z);
    chunk->needUpdate = true;
}

void Mesh::placeBlock(int x, int y, int z, Chunk* chunk) {
    Mesh::chunk = chunk;
    Mesh::newChunk = false;
    int ind = chunk->voxels[z + D * (x + W * y)];
    if (BlockManager::blocks[ind].transparent) {
        for (int g = 0; g < 6; g++) {
            if (addSide(chunk, g, x, y, z, BlockManager::sides[ind][g]))
                chunk->toAdd.push_back({ chunk->positions[z + D * (x + W * y)], g });
        }
        chunk->changedCount++;
        world->toUpdate.push(chunk);
    }
    updateNears(x, y, z);
    chunk->needUpdate = true;
}

void Mesh::updateLight() {
    Mesh::newChunk = false;
    for (usint block : chunk->updateBlocks) {
        bool changed = false;
        int x = (block & 0xF000u) >> 12;
        int y = (block & 0xFF0u) >> 4;
        int z = (block & 0xF);
        int ind = chunk->voxels[z + D * (x + W * y)];
        if (BlockManager::blocks[ind].transparent) continue;
        uint cnt = 0;
        for (uint g = 0; g < 6; g++) {
            if (!checkNearSide(chunk, g, x, y, z, ind)) {
                updateSide(chunk, g, x, y, z, BlockManager::sides[ind][g]);
                changed = true;
                int pos = chunk->positions[z + D * (x + W * y)];
                if (pos >= Config::B_START) chunk->toAdd.push_back({ pos, g });
                else {
                    uint tmp = g | (cnt << 4);
                    chunk->toAdd.push_back({ pos, tmp });
                    cnt++;
                } 
            }
        }
        if (changed) chunk->changedCount++;
    }
}