#include "blocks.h"

Texture* BlockManager::textureArray;
std::vector<std::vector<int>> BlockManager::sides;
std::vector<Block> BlockManager::blocks;
std::map<std::string, int> BlockManager::NameIndex;
std::map<int, std::string> BlockManager::IndexName;
std::vector<int> BlockManager::opacity;
std::vector<int> BlockManager::drawGroup;

void BlockManager::loadTextures(std::string path) {
	std::ifstream file(path + "/blocks");
	if (!file) {
		std::cout << "FAILED TO OPEN BLOCKS FOLDER" << std::endl;
		return;
	}
	textureArray = new Texture(path, 32, 32, 256);
	int cur = 0;
	int pos = 0;
	std::string name;
	while (file) {
		std::string s;
		file >> s;
		if (s == "-") {
			cur++;
		} else {
			if (s[0] - '0' <= 9 && s[0] - '0' >= 0) {
				int x = s[0] - '0';
				textureArray->addTexture(name + std::to_string(x) + ".png");
				if (x == 2) {
					sides[cur][4] = pos;
				}
				if (x == 3) {
					sides[cur][5] = pos;
				}
				pos++;
			}
			else if (s == "rgb") {
				file >> blocks[cur].r >> blocks[cur].g >> blocks[cur].b;
				blocks[cur].emission = true;
			}
			else if (s == "opacity") {
				int type = 0;
				file >> type;
				if (type == 1) blocks[cur].transparent = true;
				opacity.push_back(type);
			}
			else if (s == "drawGroup") {
				int type = 0;
				file >> type;
				drawGroup.push_back(type);
			}
			else if (s == "solid") {
				int type = 0;
				file >> type;
				if (type == 0) blocks[cur].solid = false;
				else blocks[cur].solid = true;
			}
			else {
				sides.push_back(std::vector<int>(6, pos));
				name = s;
				NameIndex[name] = cur;
				IndexName[cur] = name;
				blocks.push_back(Block(0, 0, 0, 0));
			}
		}
	}
	textureArray->generateMipmaps();
}

void BlockManager::useTextures() {
	textureArray->use("array");
}