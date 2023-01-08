#include "text.h"
#include "../world/config.h"
#include <sstream>
#include "../world/blocks.h"

Text::Text(std::string fontName, float width, float height) {
    proj = glm::ortho(0.0f, width, 0.0f, height);
    shader = new Shader("res/shaders/text.vert", "res/shaders/text.frag");
    font = new Font("res/fonts/" + fontName);
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Text::draw(std::string text, float x, float y, float scale, glm::vec3 color)
{
    shader->use();
    shader->uniform3f("textColor", color);
    shader->uniformm("projection", proj);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        int k = *c;
        Character ch = font->Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        float vertices[6][4] = {
            { xpos, ypos + h, 0.0f, 0.0f },
            { xpos, ypos, 0.0f, 1.0f },
            { xpos + w, ypos, 1.0f, 1.0f },

            { xpos, ypos + h, 0.0f, 0.0f },
            { xpos + w, ypos, 1.0f, 1.0f },
            { xpos + w, ypos + h, 1.0f, 0.0f }
        };

        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.Advance >> 6) * scale;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}

std::string IntToStr(int n) {
    std::string x;
    std::stringstream ss;
    ss << n;
    ss >> x;
    return x;
}

void Text::statistics(ChunkManager* world, Camera* camera) {
    int size = (2 * Config::wx + 1) * (2 * Config::wz + 1);
    int w = Events::screenWidth;
    int h = Events::screenHeight;
    float dx = 30.0f;
    float dy = 30.0f;
    float scale = 0.5f;
    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
    Chunk* pointChunk = world->getChunk(Events::selectedBlock.x, Events::selectedBlock.y, Events::selectedBlock.z);
    int cx, cz, vx, vy, vz, pos;
    if (pointChunk != nullptr && !pointChunk->removed) {
        cx = pointChunk->posX;
        cz = pointChunk->posZ;
        vx = Events::selectedBlock.x - pointChunk->posX * W;
        vz = Events::selectedBlock.z - pointChunk->posZ * D;
        vy = Events::selectedBlock.y;
        pos = pointChunk->positions[vz + D * (vx + vy * W)];
    }
    else {
        cx = cz = vx = vy = vz = pos = 0;
    }

    draw("world size " + IntToStr(size), dx, h - dy, scale, color);
    draw("selected block " + \
        IntToStr(Events::selectedBlock.x) + " " + \
        IntToStr(Events::selectedBlock.y) + " " + \
        IntToStr(Events::selectedBlock.z) + " / type " + \
        BlockManager::IndexName[Events::selectedBlock.w] + " / buf " + \
        IntToStr(pos) + " / chunk XZ " + \
        IntToStr(cx) + " " + \
        IntToStr(cz),
        dx, h - 2 * dy, scale, color);
    draw("position XYZ " + \
        IntToStr(camera->iPos.x) + " " + \
        IntToStr(camera->iPos.y) + " " + \
        IntToStr(camera->iPos.z) + \
        " / chunk XZ " + \
        IntToStr(camera->cPos.x) + " " + \
        IntToStr(camera->cPos.z),
        dx, h - 3 * dy, scale, color);

    Chunk* curChunk = world->getChunk(camera->cPos.x, camera->cPos.z);
    if (curChunk != nullptr) {
        draw("visible " + \
            IntToStr(world->visibleSurfaces) + " / max " + \
            IntToStr(world->maxSurfaces.x) + " " + \
            IntToStr(world->maxSurfaces.y) + " " + \
            IntToStr(world->maxSurfaces.z) + " " + " / cur " + \
            IntToStr(curChunk->visibleSurfaces) + " / " + \
            IntToStr(world->waterSurfaces) + " / " + \
            IntToStr(curChunk->waterSurfaces) + " (buf " + \
            IntToStr(curChunk->waterPos) + ") ",
            dx, h - 4 * dy, scale, color);
    }
    
}