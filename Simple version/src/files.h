#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <map>

#include "world/blocks.h"
#include "world/chunkManager.h"
#include "world/structures.h"

class FileManager {
public:
	static ChunkManager* world;

	static void Initialize(ChunkManager* world);
	
	static void DebugLogs(std::string path);
	static void saveWorld(std::string path);
	static void loadWorld(std::string path);
	static void saveStructure(std::string path);
	static void loadStructure(std::string path);
};