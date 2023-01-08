#include "structures.h"
#include "blocks.h"

std::vector<Object> Structures::objects;
using fs = std::filesystem::recursive_directory_iterator;

void Structures::load(std::string path) {
	int countObjects = 0;
	for (const auto& dirEntry : fs(path)) {
		std::ifstream file(dirEntry);
		if (!file) {
			std::cout << "FAILED TO OPEN STRUCTURE FILE" << std::endl;
			return;
		}
		std::string s;
		int cnt = 0;
		int count = 0;
		int coord = 0;
		Point p;
		objects.push_back(Object());
		while (file >> s) {
			if (s == "=") {
				cnt++;
				continue;
			}
			if (s[0] >= 'a') {
				objects[countObjects].types.push_back(BlockManager::NameIndex[s]);
				objects[countObjects].points.push_back(std::vector<Point>());
				continue;
			}
			if (s[0] == '-') {
				coord = 0;
				for (int i = 0; i < s.size() - 1; i++)
				{
					coord *= pow(10, i);
					coord += (s[1 + i] - '0');
				}
				coord *= -1;
			}
			else if (s[0] - '0' >= 0 && s[0] - '0' <= 9) {
				coord = 0;
				for (int i = 0; i < s.size(); i++)
				{
					coord *= pow(10, i);
					coord += (s[i] - '0');
				}
			}
			if (count == 0) p.x = coord;
			if (count == 1) p.y = coord;
			if (count == 2) p.z = coord;
			count++;
			if (count == 3) {
				objects[countObjects].points[cnt].push_back(p);
				count = 0;
			}
		}
		objects[countObjects].diffTypes = cnt;
		countObjects++;
	}
	
}