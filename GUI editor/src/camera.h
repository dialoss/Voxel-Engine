#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class Camera {
public:
	glm::vec3 pos;
	glm::vec3 up;
	glm::vec3 dir;
	glm::ivec3 cPos;
	glm::ivec3 iPos;

	glm::mat4 proj;
	glm::mat4 view;
	float fov, near, far, ratio;

	Camera(float fov, float width, float height, float near, float far);
	void updateMatrix();
	void updateProj();
	void changeViewDirection(float angleX, float angleY);
};