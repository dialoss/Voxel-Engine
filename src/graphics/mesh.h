#pragma once

#include <vector>

using uint = unsigned int;

class ChunkManager;
class Chunk;

class Mesh {
public:
    Chunk* chunk;
    ChunkManager* world;
    bool newChunk;
    uint* meshBuffer;
    uint* waterBuffer;
    std::vector<int> updatePos;

    Mesh(ChunkManager* world, Chunk* chunk);
    void initialize(ChunkManager* world);
    void resetBuffer();
    void create(Chunk* chunk);
    void loadStructures(Chunk* chunk);
    bool checkNearSide(Chunk* &chunk, const int& g, const int& x, const int& y, const int& z, const int& ind);
    bool addSide(Chunk* &chunk, const int& g, const int& x, const int& y, const int& z, const int& uv);
    void updateSide(Chunk*& chunk, const int& g, const int& x, const int& y, const int& z, const int& uv);
    void removeSide(Chunk* &chunk, int g, int x, int y, int z);
    void updateNears(int x, int y, int z);
    void removeBlock(int x, int y, int z, Chunk* chunk);
    void placeBlock(int x, int y, int z, Chunk* chunk);
    void updateLight();
};