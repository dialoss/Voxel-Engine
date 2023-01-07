#pragma once

#include <queue>
#include <vector>
#include <tuple>
#include <string>
#include <mutex>
#include "glm/glm.hpp"

#define MOD(a, b) ((a + b) % b)
#define GET_COLOR(r, g, b) ((float)r / 255), ((float)g / 255), ((float)b / 255)

using uint = unsigned int;

const int W = 16;
const int H = 128;
const int D = 16;

const int chunkPos = 10000;
const int coord_const = 5e5;

const int minLight = 0;
static int raycastStep = 0;
static std::mutex chunkmut;

static glm::vec3 getColor(float r, float g, float b) {
    return glm::vec3(r / 255, g / 255, b / 255);
}

static int sign(float t) {
    if (t < 0) return -1;
    return 1;
}

static float bilinearInterp(const float& h11, const float& h12, const float& h21, const float& h22, const int &x1, const int& x2, const int& z1, const int& z2, const int& x, const int& z) {
    float tx = x2 - x1;
    float tz = z2 - z1;
    float d = tx * tz;

    float r11 = h11 * (x2 - x) * (z2 - z);
    float r21 = h21 * (x - x1) * (z2 - z);
    float r12 = h12 * (x2 - x) * (z - z1);
    float r22 = h22 * (x - x1) * (z - z1);

    int h = (r11 + r21 + r12 + r22) / d;
    return h;
}