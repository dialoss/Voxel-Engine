#include "config.h"

int Config::V_SIZE;
int Config::S_COUNT;

int Config::wx;
int Config::wz;
int Config::rx;
int Config::rz;
int Config::BW;

int Config::WATER_B_SIZE = 0;
int Config::CHUNK_B_SIZE = 0;
int Config::B_SIZE = 0;
int Config::CHUNKS_IN_B = 0;
int Config::b_size;
int Config::B_START = 0;
int Config::B_VCOUNT = 0;
std::vector<std::pair<int, int>> Config::B_RANGES;
std::vector<int> Config::B_OFFSETS;

uint* Config::linesBuffer;
std::vector<uint*> Config::clearChunks;
uint* Config::clearWater;
uint* Config::clearBlock;
uint* Config::clearSide;

bool Config::caves = false;
bool Config::flatWorld;
bool Config::canUpdate;
std::string Config::curType;

void Config::load(std::string gameMode) {
    curType = gameMode;
    if (gameMode == "flat") {
        wx = 1;
        wz = 1;
        flatWorld = true;
    }
    else if (gameMode == "struct") {
        wx = 0;
        wz = 0;
        flatWorld = true;
    }
    else {
        wx = 10;
        wz = 10;
        flatWorld = false;
        caves = true;
    }
    
    rx = wx + 1;
    rz = wz + 1;
    BW = 2 * rx + 1;
    b_size = (2 * rx + 1) * (2 * rz + 1);

    CHUNKS_IN_B = (2 * wx + 1) * (2 * wz + 1) + 100;
    V_SIZE = 3;

    if (flatWorld) {
        B_RANGES = {
            {0, 0},
            {500, CHUNKS_IN_B * 0.2f},
            {1000, CHUNKS_IN_B * 0.5f},
            {2000, CHUNKS_IN_B * 0.4f},
            {5000, CHUNKS_IN_B * 0.1f}
        };
    }
    else {
        if (caves) {
            B_RANGES = {
                {0, 0},
                {500, CHUNKS_IN_B * 0.1f},
                {1000, CHUNKS_IN_B * 0.1f},
                {2000, CHUNKS_IN_B * 0.1f},
                {3000, CHUNKS_IN_B * 0.1f},
                {5000, CHUNKS_IN_B * 0.3f},
                {6000, CHUNKS_IN_B * 0.3f},
                {7000, CHUNKS_IN_B * 0.2f}
            };
        }
        else {
            B_RANGES = {
                {0, 0},
                {500, CHUNKS_IN_B * 0.15f},
                {1000, CHUNKS_IN_B * 0.4f},
                {2000, CHUNKS_IN_B * 0.5f},
                {3000, CHUNKS_IN_B * 0.3f},
                {5000, CHUNKS_IN_B * 0.03f},
            };
        }
        
    }
   

    B_OFFSETS.resize(B_RANGES.size(), 0);
    clearChunks.resize(B_RANGES.size());

    for (int i = 0; i < B_RANGES.size(); i++)
    {
        B_SIZE += B_RANGES[i].first * B_RANGES[i].second * 6 * V_SIZE;
        B_VCOUNT += B_RANGES[i].first * B_RANGES[i].second * 6;
        B_START += B_RANGES[i].first * B_RANGES[i].second;
        if (i < B_RANGES.size() - 1) B_OFFSETS[i + 1] = B_START;
    }
    B_SIZE += 1e4 * 36 * V_SIZE;
    B_VCOUNT += 1e4 * 36;
    CHUNK_B_SIZE = B_RANGES.back().first;
    WATER_B_SIZE = 256;
}

void Config::init() {
    linesBuffer = new uint[3 * 36];
    for (int i = 0; i < B_RANGES.size(); i++)
    {
        clearChunks[i] = new uint[B_RANGES[i].first * 6 * V_SIZE];
        std::memset(clearChunks[i], 0, B_RANGES[i].first * 6 * V_SIZE * sizeof(uint));
    }

    clearWater = new uint[WATER_B_SIZE * 6 * V_SIZE];
    clearBlock = new uint[36 * V_SIZE];
    clearSide = new uint[6 * V_SIZE];

    std::memset(clearWater, 0, WATER_B_SIZE * 6 * V_SIZE * sizeof(uint));
    std::memset(clearBlock, 0, 36 * V_SIZE * sizeof(uint));
    std::memset(clearSide, 0, 6 * V_SIZE * sizeof(uint));
    std::memset(linesBuffer, 0, 3 * 36 * sizeof(uint));
}