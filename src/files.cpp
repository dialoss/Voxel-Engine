#include "files.h"
#include "world/config.h"

ChunkManager* FileManager::world;

void FileManager::Initialize(ChunkManager* world) {
	FileManager::world = world;
}

void FileManager::saveStructure(std::string path) {
	Chunk* chunk = FileManager::world->chunks[4];
	std::map<int, std::vector<Point>> points;
	int min_x = 16, min_z = 16;
	int max_x = 0, max_z = 0;
	for (int x = 0; x < W; x++)
	{
		for (int z = 0; z < D; z++)
		{
			for (int y = 7; y < H; y++)
			{
				int ind = chunk->voxels[z + D * (x + W * y)];
				if (ind != 0) {
					points[ind].push_back(Point(x, y, z));
					min_x = std::min(min_x, x);
					min_z = std::min(min_z, z);
					max_x = std::max(max_x, x);
					max_z = std::max(max_z, z);
				}
			}
		}
	}
	int w = (max_x - min_x) / 2;
	int d = (max_z - min_z) / 2;

	std::ofstream file(path);
	//file << points.size() << '\n';
	for (auto ar : points) {
		file << BlockManager::IndexName[ar.first] << '\n';
		for (Point p : ar.second) {
			file << p.x - min_x - w << ' ' << p.y - 7 << ' ' << p.z - min_z - d << '\n';
		}
		file << "=\n";
	}
	file.close();
}

void FileManager::DebugLogs(std::string path) {
	int size = Config::b_size;
	std::ofstream file(path);
	file << "world size " << size << '\n';
	int total = 0;
	std::vector<std::pair<int, int>> ranges = {
		{0, 300},
		{300, 500},
		{500, 800},
		{800, 1000},
		{1000, 2000},
		{2000, 3000},
		{3000, 4000},
		{4000, 5000}
	};
	std::vector<int> counts(ranges.size(), 0);
	for (int i = 0; i < Config::b_size; i++)
	{
		int vSurf = world->chunks[i]->visibleSurfaces;
		if (vSurf > 0) {
			file << vSurf << '\n';
			for (int j = 0; j < ranges.size(); j++)
			{
				if (vSurf >= ranges[j].first && vSurf < ranges[j].second) 
					counts[j]++;
			}
		}
		total += vSurf;
	}
	file << "total visible " << total << '\n';
	for (int i = 0; i < counts.size(); i++)
	{
		file << ranges[i].first << ' ' << ranges[i].second << ' ' << counts[i] << '\n';
	}
	file.close();
}