#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <iostream>
#include <filesystem>

static enum Objects {
	tree,
	rock
};

struct Point {
	int x, y, z;
	Point() {}
	Point(int x, int y, int z) : x(x), y(y), z(z) {}
};

struct Object {
	int diffTypes;
	std::vector<int> types;
	std::vector<std::vector<Point>> points;
	Object() {}
};

class Structures {
public:
	static std::vector<Object> objects;

	static void load(std::string path);
};