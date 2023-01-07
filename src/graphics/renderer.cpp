#include "renderer.h"

Renderer::Renderer(const uint* buffer, const int& vCount, const int* attrs) {
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	int vSize = 0;
	for (int i = 0; attrs[i]; i++) vSize += attrs[i];
	this->vCount = vCount;

	glBufferData(GL_ARRAY_BUFFER, vSize * vCount * sizeof(GLuint), buffer, GL_DYNAMIC_DRAW);

	int offset = 0;
	for (int i = 0; attrs[i]; i++) {
		glVertexAttribIPointer(i, attrs[i], GL_UNSIGNED_INT, vSize * sizeof(GLuint), (void*)(offset * sizeof(GLuint)));
		glEnableVertexAttribArray(i);
		offset += attrs[i];
	}
}

Renderer::Renderer(const float* buffer, const int& vCount, const int* attrs) {
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	int vSize = 0;
	for (int i = 0; attrs[i]; i++) vSize += attrs[i];
	if (vCount != 1e6) this->vCount = vCount;

	glBufferData(GL_ARRAY_BUFFER, vSize * vCount * sizeof(GLfloat), buffer, GL_STATIC_DRAW);

	int offset = 0;
	for (int i = 0; attrs[i]; i++) {
		glVertexAttribPointer(i, attrs[i], GL_FLOAT, GL_FALSE, vSize * sizeof(GLfloat), (void*)(offset * sizeof(GLfloat)));
		glEnableVertexAttribArray(i);
		offset += attrs[i];
	}
}

void Renderer::loadChunk(const uint* buffer, const int& pos, const int& curCount) {
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	int buffer_pos = pos * 6 * Config::V_SIZE;

	glBufferSubData(
		GL_ARRAY_BUFFER,
		buffer_pos * sizeof(GLuint),
		curCount * 6 * Config::V_SIZE * sizeof(GLuint),
		buffer
	);
}

void Renderer::update(const uint* buffer, std::vector<std::pair<int, int>>& sides) {
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	int count = 0;
	int prev = -1;
	uint* data = new uint[6 * Config::V_SIZE];
	memset(data, 0, 6 * Config::V_SIZE * sizeof(uint));
	for (std::pair<int, int> item : sides) {
		int pos = item.first;
		int side = ((uint)item.second & 0xFu);
		int index = ((uint)item.second & 0xF0u) >> 4;
		if (prev == -1) prev = pos;
		if (prev != pos) {
			count++;
			prev = pos;
		}
		int buffer_pos = 0;
		if (pos < Config::B_START) buffer_pos = pos * 6 * Config::V_SIZE + index * 6 * Config::V_SIZE;
		else buffer_pos = Config::B_START * 6 * Config::V_SIZE + (pos - Config::B_START) * 36 * Config::V_SIZE + side * 6 * Config::V_SIZE;

		for (int j = 0; j < 6 * Config::V_SIZE; j++) {
			data[j] = buffer[count * 36 * Config::V_SIZE + j + side * 6 * Config::V_SIZE];
		}	

		glBufferSubData(
			GL_ARRAY_BUFFER,
			buffer_pos * sizeof(GLuint),
			6 * Config::V_SIZE * sizeof(GLuint),
			data
		);
	}
	delete[] data;
}

void Renderer::initLines(const float *buffer, const int *indices) {
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, 3 * 8 * sizeof(float), buffer, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * 8 * sizeof(int), indices, GL_STATIC_DRAW);
}

void Renderer::loadLines(const uint* buffer) {
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferSubData(
		GL_ARRAY_BUFFER,
		0 * sizeof(GLfloat),
		48 * 3 * sizeof(GLfloat),
		buffer
	);
}

void Renderer::removeBlock(int pos) {
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	int buffer_pos = (pos - Config::B_START) * 36 * Config::V_SIZE + Config::B_START * 6 * Config::V_SIZE;

	glBufferSubData(
		GL_ARRAY_BUFFER,
		buffer_pos * sizeof(GLuint),
		36 * Config::V_SIZE * sizeof(GLuint),
		Config::clearBlock
	);
}

void Renderer::removeSides(std::vector<std::pair<int, int>>& sides) {
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	for (std::pair<int, int> item : sides) {
		int pos = item.first;
		int side = item.second;

		int buffer_pos = (pos + side) * 6 * Config::V_SIZE;

		if (pos >= Config::B_START) {
			buffer_pos = 6 * side * Config::V_SIZE + (pos - Config::B_START) * 36 * Config::V_SIZE + Config::B_START * 6 * Config::V_SIZE;
		}

		glBufferSubData(
			GL_ARRAY_BUFFER,
			buffer_pos * sizeof(GLuint),
			6 * Config::V_SIZE * sizeof(GLuint),
			Config::clearSide
		);
		
	}
}

void Renderer::draw(uint type, bool water) {
	glBindVertexArray(VAO);
	if (!water) {
		if (type == GL_LINES) {
			glLineWidth(1.3f);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glDrawElements(type, 8 * 3, GL_UNSIGNED_INT, 0);
		}
		else glDrawArrays(type, 0, 1e6 * 20);
	}
	else {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_CULL_FACE);
		glDrawArrays(type, 0, 2e5 * 6);
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
	}
}

Renderer::~Renderer() {
	glBindVertexArray(VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}
