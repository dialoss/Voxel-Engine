#pragma once

#include "FastNoiseLite.h"
#include "glm/glm.hpp"

class NoiseGenerator {
public:
	static FastNoiseLite generator;
	static float* heightMap;
	static int w, h;
	static glm::vec2 translation;
	static float noiseScale, noiseAmp;

	static void init(int w, int h, glm::vec2 trans, float noiseScale, float noiseAmp);
	static int getHeight(float x, float z);
	static void createHeightMap(int w, int h, glm::vec2 trans, float noiseScale, float noiseAmp);
};