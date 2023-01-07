#pragma once

#define GLEW_STATIC
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <cstring>
#include "glm/glm.hpp"

#include <vector>
#include "camera.h"
#include "world/chunkManager.h"

class Events {
public:
	static GLFWwindow* window;
	static Camera* camera;
	static ChunkManager* world;
	static int screenWidth, screenHeight;

	static float dt;
	static int curFrame;
	static float cameraSpeed;
	static float sensetivity;
	
	static int mouseX, mouseY;
	static int deltaX, deltaY;
	static float angleX, angleY;

	static bool hideCursor;
	static bool* pressed;
	static int* keys;
	static bool mouseEnter;
	static bool closeWindow;
	static int curBlock;
	static glm::vec4 selectedBlock;

	static std::vector<std::vector<int>> updateBlock;

	static void initialize(GLFWwindow* window, Camera* camera, ChunkManager* world);
	static void pollEvents();

	static void update();
	static bool clicked(int key);
	static bool justClicked(int key);
	static bool mouseClicked(int btn);
	static bool justMouseClicked(int btn);
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void onMouseScroll(GLFWwindow* window, double xoffset, double yoffset);
	static void onMousePress(GLFWwindow* window, int button, int action, int mods);
	static void onMouseMoved(GLFWwindow* window, double xpos, double ypos);
	static void windowResize(GLFWwindow* window, int width, int height);
};