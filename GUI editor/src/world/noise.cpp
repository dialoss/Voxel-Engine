#include "noise.h"
#include "../utility/variables.h"

FastNoiseLite NoiseGenerator::generator;
float* NoiseGenerator::heightMap;
float NoiseGenerator::noiseAmp;
float NoiseGenerator::noiseScale;
int NoiseGenerator::w;
int NoiseGenerator::h;
glm::vec2 NoiseGenerator::translation;

void NoiseGenerator::init(int w, int h, glm::vec2 trans, float noiseScale, float noiseAmp) {
	generator.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	NoiseGenerator::noiseAmp = noiseAmp;
	NoiseGenerator::w = w;
	NoiseGenerator::h = h;
	NoiseGenerator::translation = trans;
	NoiseGenerator::noiseScale = noiseScale;
}

int NoiseGenerator::getHeight(float x, float z) {
	float cont = generator.GetNoise((x + translation.x) * noiseScale, (z + translation.y) * noiseScale) * noiseAmp;
	int h = 40 + cont * 20;
	return h;
}

void NoiseGenerator::createHeightMap(int w, int h, glm::vec2 trans, float noiseScale, float noiseAmp) {
	heightMap = new float[w * h];
	memset(heightMap, 0, w * h * sizeof(float));
	for (int x = 0; x < w; x++)
	{
		for (int z = 0; z < h; z++)
		{
			float nx = (float)x + trans.x;
			float nz = (float)z + trans.y;
			nx *= noiseScale;
			nz *= noiseScale;

			float val = generator.GetNoise(nx, nz) * noiseAmp;
			heightMap[z + w * x] = val;
		}
	}
}