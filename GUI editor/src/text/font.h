#pragma once

#define GLEW_STATIC
#include "GL/glew.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <string>
#include <iostream>
#include <map>
#include "glm/glm.hpp"

struct Character {
    unsigned int textureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    unsigned int Advance;
};

class Font {
public:
    std::map<char, Character> Characters;

	Font(std::string path);
};