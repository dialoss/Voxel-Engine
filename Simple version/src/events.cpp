#include "events.h"
#include "utility/debug.h"
#include "world/blocks.h"
#include "files.h"
#include <iostream>

GLFWwindow* Events::window;
Camera* Events::camera;
ChunkManager* Events::world;
std::vector<std::vector<int>> Events::updateBlock;
int Events::curFrame = 0;
bool Events::hideCursor = false;
int Events::screenWidth;
int Events::screenHeight;

int* Events::keys;
bool* Events::pressed;

float Events::dt = 0;
float Events::cameraSpeed = 20.0f;
float Events::sensetivity = 0.1f;

int Events::mouseX = 0;
int Events::mouseY = 0;
int Events::deltaX = 0;
int Events::deltaY = 0;
float Events::angleX = 90;
float Events::angleY = 0;

bool Events::mouseEnter = false;
bool Events::closeWindow = false;

int Events::curBlock = Blocks::dirt;
glm::vec4 Events::selectedBlock;

float lastTime = 0;


void Events::initialize(GLFWwindow* window, Camera* camera, ChunkManager* world) {
	Events::window = window;
	Events::camera = camera;
	Events::world = world;

	keys = new int[1024];
	pressed = new bool[1024];
	memset(keys, 0, 1024 * sizeof(int));
	memset(pressed, false, 1024 * sizeof(bool));

	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, onMousePress);
	glfwSetCursorPosCallback(window, onMouseMoved);
	glfwSetScrollCallback(window, onMouseScroll);
	glfwSetWindowSizeCallback(window, windowResize);
}

void Events::windowResize(GLFWwindow* window, int width, int height) {
	width = 1.25f * height;
	glfwSetWindowSize(window, width, height);
	glViewport(0, 0, width, height);
	Events::screenHeight = height;
	Events::screenWidth = width;
}

void Events::pollEvents() {
	glfwPollEvents();
	Events::update();
	curFrame++;
	deltaX = 0;
	deltaY = 0;
}


void Events::update() {
	angleX += deltaX * sensetivity;
	angleY -= deltaY * sensetivity;

	if (angleY < -89.0f) angleY = -89.0f;
	if (angleY > 89.0f) angleY = 89.0f;

	camera->changeViewDirection(angleX, angleY);

	float curTime = glfwGetTime();
	dt = curTime - lastTime;
	lastTime = curTime;

	if (justClicked(GLFW_KEY_ESCAPE)) {
		closeWindow = true;
		glfwSetWindowShouldClose(Events::window, true);
	}
	if (justClicked(GLFW_KEY_TAB)) {
		hideCursor = !hideCursor;
		mouseEnter = false;
		glfwSetInputMode(window, GLFW_CURSOR, hideCursor ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
	}
	if (justClicked(GLFW_KEY_E)) {
		int x = camera->pos.x; int y = camera->pos.y; int z = camera->pos.z;
		if (camera->pos.x < 0) x--;
		if (camera->pos.z < 0) z--;
		std::cout << world->getLight(x, y, z, 0) << '\n';
	}
	if (justClicked(GLFW_KEY_Q)) {
		int x = camera->pos.x; int z = camera->pos.z;
		if (camera->pos.x < 0) x--;
		if (camera->pos.z < 0) z--;
		int cx = x / W;
		int cz = z / D;
		if (x < 0 && abs(x) % 16 != 0) cx--;
		if (z < 0 && abs(z) % 16 != 0) cz--;
		std::cout << cx << ' ' << cz << '\n';
	}
	if (justClicked(GLFW_KEY_R)) {
		int x = camera->pos.x; int z = camera->pos.z;
		if (camera->pos.x < 0) x--;
		if (camera->pos.z < 0) z--;
		int cx = x / W;
		int cz = z / D;
		if (x < 0 && abs(x) % 16 != 0) cx--;
		if (z < 0 && abs(z) % 16 != 0) cz--;
		int vx = x - cx * W;
		int vz = z - cz * D;
		std::cout << x << ' ' << (int)camera->pos.y << ' ' << z << '\n';
		std::cout << (int)world->getChunk(x, (int)camera->pos.y, z)->heightMap[vz + W * vx] << '\n';
	}
	if (clicked(GLFW_KEY_1)) curBlock = Blocks::grass;
	if (clicked(GLFW_KEY_2)) curBlock = Blocks::dirt;
	if (clicked(GLFW_KEY_3)) curBlock = Blocks::glowstone;
	if (clicked(GLFW_KEY_4)) curBlock = Blocks::leaves;
	if (clicked(GLFW_KEY_5)) curBlock = Blocks::oak;
	if (clicked(GLFW_KEY_6)) curBlock = Blocks::glass;
	if (clicked(GLFW_KEY_7)) curBlock = Blocks::cobble;
	if (clicked(GLFW_KEY_8)) curBlock = Blocks::birch;
	if (justMouseClicked(GLFW_MOUSE_BUTTON_MIDDLE)) curBlock = selectedBlock.w;

	if (clicked(GLFW_KEY_C)) {
		camera->fov = 30;
		camera->updateProj();
	} else {
		camera->fov = 70;
		camera->updateProj();
	}
	if (clicked(GLFW_KEY_W)) camera->pos += camera->dir * dt * cameraSpeed;
	if (clicked(GLFW_KEY_S)) camera->pos -= camera->dir * dt * cameraSpeed;
	if (clicked(GLFW_KEY_A)) camera->pos -= glm::normalize(glm::cross(camera->dir, camera->up)) * dt * cameraSpeed;
	if (clicked(GLFW_KEY_D)) camera->pos += glm::normalize(glm::cross(camera->dir, camera->up)) * dt * cameraSpeed;
	if (clicked(GLFW_KEY_LEFT_SHIFT)) camera->pos -= camera->up * dt * cameraSpeed;
	if (clicked(GLFW_KEY_SPACE)) camera->pos += camera->up * dt * cameraSpeed;

	if (justMouseClicked(GLFW_MOUSE_BUTTON_RIGHT) || justMouseClicked(GLFW_MOUSE_BUTTON_LEFT)) {
		glm::ivec3 normal(0, 0, 0);
		glm::ivec3 coords(0, 0, 0);
		std::vector<glm::ivec3> _void;
		char* voxel = world->rayCast(camera->pos, camera->dir, 50, normal, coords, _void, false);
		if (voxel == nullptr) return;
		if (justMouseClicked(GLFW_MOUSE_BUTTON_LEFT)) updateBlock.push_back({ coords[0], coords[1], coords[2], 1 });
		else updateBlock.push_back({ coords[0] + normal[0], coords[1] + normal[1], coords[2] + normal[2], 2 });
	}

	if (clicked(GLFW_KEY_L)) {
		std::string name;
		std::cout << "Input filename:\n";
		std::cin >> name;
		FileManager::saveStructure("saves/structures/" + name + ".txt");
	}
	if (clicked(GLFW_KEY_P)) {
		std::string name = "log";
		FileManager::DebugLogs("saves/logs/" + name + ".txt");
	}
	if (clicked(GLFW_KEY_R)) {
		int cx, cz;
		std::cout << "Input chunk coords:\n";
		std::cin >> cx >> cz;
		world->toAdd.push(world->getChunk(cx, cz));
	}

	int px = (int)camera->pos.x;
	int py = (int)camera->pos.y;
	int pz = (int)camera->pos.z;
	if (camera->pos.x < 0) px--;
	if (camera->pos.z < 0) pz--;
	int cx = px / W;
	int cz = pz / D;
	if (px < 0 && abs(px) % 16 != 0) cx--;
	if (pz < 0 && abs(pz) % 16 != 0) cz--;
	camera->iPos = glm::ivec3(px, py, pz);
	camera->cPos = glm::ivec3(cx, py, cz);
}

bool Events::clicked(int key) {
	return pressed[key];
}
bool Events::justClicked(int key) {
	return pressed[key] && keys[key] == curFrame;
}
bool Events::mouseClicked(int btn) {
	return pressed[btn];
}
bool Events::justMouseClicked(int btn) {
	return pressed[btn] && keys[btn] == curFrame;
}

void Events::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	if (action == GLFW_PRESS) {
		pressed[key] = 1;
		keys[key] = curFrame;
	} else if (action == GLFW_RELEASE) {
		pressed[key] = 0;
	}
}

void Events::onMouseScroll(GLFWwindow* window, double xoffset, double yoffset) {
	if (cameraSpeed < 6) cameraSpeed += yoffset * 0.5f;
	else cameraSpeed += yoffset * 3;
}
void Events::onMousePress(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		pressed[button] = 1;
		keys[button] = curFrame;
	} else if (action == GLFW_RELEASE) {
		pressed[button] = 0;
	}
}
void Events::onMouseMoved(GLFWwindow* window, double xpos, double ypos) {
	if (!hideCursor) return;

	if (!mouseEnter) {
		mouseEnter = true;
		mouseX = xpos;
		mouseY = ypos;
	}

	deltaX += xpos - mouseX;
	deltaY += ypos - mouseY;

	mouseX = xpos;
	mouseY = ypos;
}