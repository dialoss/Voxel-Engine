#pragma once
#include <string>
#include <vector>

using uint = unsigned int;

class Config {
public:
    static int V_SIZE;
    static int S_COUNT;

    static int wx;
    static int wz;
    static int rx;
    static int rz;
    static int b_size;
    static int BW;
    
    static int WATER_B_SIZE;
    static int CHUNK_B_SIZE;
    static int CHUNKS_IN_B;
    static int B_SIZE;
    static int B_START;
    static int B_VCOUNT;
    static std::vector<std::pair<int, int>> B_RANGES;
    static std::vector<int> B_OFFSETS;

    static uint* linesBuffer;
    static std::vector<uint*> clearChunks;
    static uint* clearWater;
    static uint* clearBlock;
    static uint* clearSide;

    static bool caves;
    static bool flatWorld;
    static bool canUpdate;
    static std::string curType;

    static void load(std::string gameMode);
    static void init();
};