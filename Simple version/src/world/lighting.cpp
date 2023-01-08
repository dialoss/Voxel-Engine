#include "lighting.h"
#include "blocks.h"

ChunkManager* LightManager::world;
LightSolver* LightManager::solverR;
LightSolver* LightManager::solverG;
LightSolver* LightManager::solverB;
LightSolver* LightManager::solverS;

void LightManager::initialize(ChunkManager* world) {
    LightManager::world = world;
    solverR = new LightSolver(world, 3);
    solverG = new LightSolver(world, 2);
    solverB = new LightSolver(world, 1);
    solverS = new LightSolver(world, 0);
}

usint LightManager::getLight(int x, int y, int z, int channel) {
    return world->getLight(x, y, z, channel);
}

void LightManager::removeBlock(Chunk* chunk, int x, int y, int z) {
    solverR->remove(chunk, x, y, z);
    solverG->remove(chunk, x, y, z);
    solverB->remove(chunk, x, y, z);
    updateLights();
    if (getLight(x + chunk->posX * W, y + 1, z + chunk->posZ * D, 0) == 0xF) {
        for (int i = y; i >= 0; i--) {
            if (!BlockManager::blocks[chunk->voxels[z + D * (x + W * i)]].transparent) break;
            solverS->add(chunk, x, i, z, 15);
        }
    }
    x += chunk->posX * W;
    z += chunk->posZ * D;
    for (int i = 0; i < 6; i++) {
        solverR->add(x + normals[i][0], y + normals[i][1], z + normals[i][2]);
        solverG->add(x + normals[i][0], y + normals[i][1], z + normals[i][2]);
        solverB->add(x + normals[i][0], y + normals[i][1], z + normals[i][2]);
        solverS->add(x + normals[i][0], y + normals[i][1], z + normals[i][2]);
    }
    updateLights();
}

void LightManager::placeBlock(Chunk* chunk, int x, int y, int z, int ind) {
    solverR->remove(chunk, x, y, z);
    solverG->remove(chunk, x, y, z);
    solverB->remove(chunk, x, y, z);
    solverS->remove(chunk, x, y, z);

    for (int i = y - 1; i >= 0; i--) {
        if (!BlockManager::blocks[chunk->voxels[z + D * (x + W * i)]].transparent) break;
        solverS->remove(chunk, x, i, z);
    }

    updateLights();
    if (BlockManager::blocks[ind].emission) {
        solverR->add(chunk, x, y, z, BlockManager::blocks[ind].r);
        solverG->add(chunk, x, y, z, BlockManager::blocks[ind].g);
        solverB->add(chunk, x, y, z, BlockManager::blocks[ind].b);

        updateLights();
    }
    //if (BlockManager::blocks[ind].transparent) {
    //    if (getLight(x + chunk->posX * W, y + 1, z + chunk->posZ * D, 0) == 0xF) {
    //        for (int i = y - 1; i >= 0; i--) {
    //            if (!BlockManager::blocks[chunk->voxels[z + D * (x + W * i)]].transparent) break;
    //            solverS->add(chunk, x, i, z, 15);
    //        }
    //    }
    //    
    //    //solverS->add(chunk, x, y, z, getLight(x + chunk->posX * W, y + 1, z + chunk->posZ * D, 0));
    //    solverS->updateLights(false);
    //}

}

void LightManager::proceedSunLight(Chunk* chunk) {
    for (int x = 0; x < W; x++) {
        for (int z = 0; z < D; z++) {
            int h = chunk->heightMap[z + W * x];
            solverS->add(chunk, x, h + 1, z, 15);
        }
    }
    solverS->updateLights(true);
}

void LightManager::updateLights() {
    solverR->updateLights(false);
    solverG->updateLights(false);
    solverB->updateLights(false);
    solverS->updateLights(false);
}