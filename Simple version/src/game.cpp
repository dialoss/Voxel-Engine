#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>

#include "utility/geometry.h"
#include "graphics/renderer.h"
#include "graphics/shader.h"
#include "events.h"
#include "camera.h"
#include "world/chunk.h"
#include "world/chunkManager.h"
#include "graphics/texture.h"
#include "graphics/mesh.h"
#include "utility/debug.h"
#include "utility/variables.h"
#include "world/config.h"
#include "graphics/RendererManager.h"
#include "world/blocks.h"
#include "world/lighting.h"
#include "world/structures.h"
#include "text/text.h"
#include "files.h"

class Window {
public:
	static GLFWwindow* window;
	static int width, height;

	Window() {
		glfwInit();

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		glfwWindowHint(GLFW_SAMPLES, 4);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		width = 1000;
		height = 800;
		window = glfwCreateWindow(width, height, "Minecraft", NULL, NULL);
		Events::screenHeight = height;
		Events::screenWidth = width;

		if (!window) {
			glfwTerminate();
			std::cerr << "FAILED TO CREATE WINDOW" << std::endl;
		}

		getWindowSize();

		glfwMakeContextCurrent(window);
		glewExperimental = true;
		glewInit();
		glfwSwapInterval(1);
		glEnable(GL_MULTISAMPLE);
	}

	static void getWindowSize() {
		glfwGetWindowSize(window, &width, &height);
	}

	static bool shouldClose() {
		return glfwWindowShouldClose(window);
	}

	static void close() {
		return glfwTerminate();
	}

};

GLFWwindow* Window::window;
int Window::width = 0;
int Window::height = 0;

std::mutex globalMutex;

void createChunks(Camera* &camera, ChunkManager* &world) {
	using namespace std::chrono_literals;
	glm::ivec3 deltaPos(0, 0, 0);
	glm::ivec3 deltaPosFrames(0, 0, 0);
	glm::ivec3 lastPos(0, 0, 0);
	auto start = std::chrono::steady_clock::now();
	while (!Window::shouldClose()) {
		glm::vec3 pos = camera->pos;
		int px = (int)pos.x;
		int py = (int)pos.y;
		int pz = (int)pos.z;
		if (pos.x < 0) px--;
		if (pos.z < 0) pz--;
		int centerX = px / W;
		int centerZ = pz / D;
		if (px < 0 && abs(px) % 16 != 0) centerX--;
		if (pz < 0 && abs(pz) % 16 != 0) centerZ--;

		glm::ivec3 cpos(centerX, py, centerZ);

		deltaPos = cpos - lastPos;
		deltaPosFrames += cpos - lastPos;
		lastPos = cpos;

		if (Config::curType != "struct" && Config::curType != "flat") world->translate(deltaPos);

		int dirx = sign(camera->dir.x);
		int dirz = sign(camera->dir.z);
	
		if (Events::updateBlock.size() != 0) {
			for (std::vector<int> coords : Events::updateBlock) {
				Chunk* chunk = world->getChunk(coords[0], coords[1], coords[2]);
				if (chunk == nullptr) continue;
				int vx = coords[0] - chunk->posX * W;
				int vz = coords[2] - chunk->posZ * D;
				int vy = coords[1];
				for (int i = -1; i <= 1; i += 2) {
					for (int j = -1; j <= 1; j += 2) {
						for (int k = -1; k <= 1; k += 2) {
							int nx = coords[0] + i;
							int ny = coords[1] + j;
							int nz = coords[2] + k;
							Chunk* chunk = world->getChunk(nx, ny, nz);
							if (chunk == nullptr) continue;
							chunk->needUpdate = true;
							usint block = ((nx - chunk->posX * W) << 12) | (ny << 4) | (nz - chunk->posZ * D);
							if (ny >= 0) chunk->updateBlocks.insert(block);
						}
					}
				}
				if (coords[3] == 1) {
					chunk->voxels[vz + D * (vx + W * vy)] = 0;
					LightManager::removeBlock(chunk, vx, vy, vz);
					chunk->mesh->removeBlock(vx, vy, vz, chunk);
				}
				else {
					chunk->voxels[vz + D * (vx + W * vy)] = Events::curBlock;
					if (!BlockManager::blocks[Events::curBlock].transparent) LightManager::placeBlock(chunk, vx, vy, vz, Events::curBlock);
					chunk->mesh->placeBlock(vx, vy, vz, chunk);
				}
			}
			Events::updateBlock.clear();
		}
	}
}

class Game {
private:
	Shader* shader;
	Shader* croshader;
	Shader* linesShader;
	Renderer* crossRenderer;
	Renderer* linesRenderer;
	ChunkManager* world;
	Camera* camera;
	Text* text;
public:
	Game() {
		Window::Window();
		shader = new Shader("res/shaders/main.vert", "res/shaders/main.frag");
		croshader = new Shader("res/shaders/cross.vert", "res/shaders/cross.frag");
		linesShader = new Shader("res/shaders/lines.vert", "res/shaders/lines.frag");

		Config::load("a");
		Config::init();

		RendererManager::initialize();
		BlockManager::loadTextures("res/textures");
		Structures::load("saves/structures/");

		world = new ChunkManager();
		LightManager::initialize(world);
		world->init();

		crossRenderer = new Renderer(cross, 6, attrs);
		linesRenderer = new Renderer();
		linesRenderer->initLines(cube_lines, cube_ind);
		camera = new Camera(70, Window::width, Window::height, 0.1f, 3000.0f);

		Events::initialize(Window::window, camera, world);

		text = new Text("arial.ttf", Window::width, Window::height);
		FileManager::Initialize(world);
	}

	void updateChunks() {
		while (world->updateStruct.size()) {
			Chunk* chunk = world->updateStruct.front();
			world->updateStruct.pop();
			chunk->mesh->loadStructures(chunk);
			chunk->changed = false;
			if (chunk->loaded) 
				world->toUpdate.push(chunk);
		}

		while (world->toUpdate.size()) {
			Chunk* chunk = world->toUpdate.front();
			world->toUpdate.pop();
			
 			chunk->mesh->updateLight();
			RendererManager::update(chunk);
			chunk->mesh->resetBuffer();
			chunk->clearBuffers();
			chunk->updateBlocks.clear();
			chunk->updateLight = false;
			chunk->needUpdate = false;
		}
	}

	void addChunks() {
		while (world->toAdd.size()) {
			Chunk* chunk = world->toAdd.front();
			world->toAdd.pop();
			
			/*for (int i = 0; i < 8; i++)
			{
				if (!chunk->nears[i]->asNear && chunk->nears[i]->changed && chunk->nears[i]->structureBlocks.size() != 0)
					world->updateStruct.push(chunk->nears[i]);
			}*/

			updateChunks();

			RendererManager::loadChunk(chunk);
			chunk->clearBuffers();
			chunk->updateBlocks.clear();
			chunk->mesh->resetBuffer();
			chunk->mesh->newChunk = false;
			world->visibleSurfaces += chunk->visibleSurfaces;
			world->waterSurfaces += chunk->waterSurfaces;
			if (world->maxSurfaces.x < chunk->visibleSurfaces) {
				world->maxSurfaces = glm::ivec3(chunk->visibleSurfaces, chunk->posX, chunk->posZ);
			}
			chunk->loaded = true;
			
		}
	}

	void removeChunks() {
		while (world->toRemove.size()) {
			Chunk* chunk = world->toRemove.front();
			world->toRemove.pop();
			if (chunk->removed) continue;
			delete chunk;
		}
	}

	void unloadChunks() {
		while (world->toUnload.size()) {
			Chunk* chunk = world->toUnload.front();
			world->toUnload.pop();
			if (chunk->loaded && !chunk->removed) RendererManager::removeChunk(chunk, true);
			chunk->loaded = false;
			world->visibleSurfaces -= chunk->visibleSurfaces;
			world->waterSurfaces -= chunk->waterSurfaces;
		}
	}

	void run() {
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);

		std::thread genWorld(std::ref(createChunks), std::ref(camera), std::ref(world));

		BlockManager::useTextures();

		glm::vec3 skyColor(GET_COLOR(146, 188, 222));

		while (!Window::shouldClose()) {
			glClearColor(skyColor.x, skyColor.y, skyColor.z, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			if (RendererManager::addBuffer) {
				RendererManager::initialize();
				RendererManager::addBuffer = false;
			}

			addChunks();
			updateChunks();
			unloadChunks();
			removeChunks();

			glm::ivec3 iend(0, 0, 0), norm(0, 0, 0);
			std::vector<glm::ivec3> posit;
			char* voxel = world->rayCast(camera->pos, camera->dir, 80, norm, iend, posit, false);
			if (voxel && *voxel != 0) {
				Events::selectedBlock = glm::vec4(iend.x, iend.y, iend.z, *voxel);
				glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(iend.x, iend.y, iend.z));
				linesShader->use();
				linesShader->uniformm("model", model);
				linesShader->uniformm("view", camera->view);
				linesShader->uniformm("proj", camera->proj);
				linesRenderer->draw(GL_LINES, false);
			}

			shader->use();
			shader->uniformm("view", camera->view);
			shader->uniformm("proj", camera->proj);
			shader->uniform3f("skyColor", skyColor);
			shader->uniformf("gradient", std::max(5, Config::wx - 2));

			RendererManager::draw();

			croshader->use();
			crossRenderer->draw(GL_TRIANGLES, false);

			text->statistics(world, camera);

			glfwSwapBuffers(Window::window);
			Events::pollEvents();
		}
		genWorld.join();
		Window::close();
	}
};


int main() {
	Game* game = new Game();
	game->run();
}


