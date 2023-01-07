#include "camera.h"

Camera::Camera(float fov, float width, float height, float near, float far) {
	pos = glm::vec3(0, 0, 0);
	up = glm::vec3(0, 1, 0);
	dir = glm::vec3(0, 0, 1);
	cPos = glm::ivec3(0);
	iPos = glm::ivec3(0);
	this->fov = fov;
	this->far = far;
	this->near = near;
	this->ratio = width / height;

	proj = glm::perspective(glm::radians(fov), ratio, near, far);
	view = glm::lookAt(pos, pos + dir, up);
}

void Camera::updateProj() {
	proj = glm::perspective(glm::radians(fov), ratio, near, far);
}

void Camera::updateMatrix() {
	view = glm::lookAt(pos, pos + dir, up);
}

void Camera::changeViewDirection(float angleX, float angleY) {
	angleX = glm::radians(angleX);
	angleY = glm::radians(angleY);
	glm::vec3 newDir(
		glm::cos(angleX) * glm::cos(angleY),
		glm::sin(angleY),
		glm::sin(angleX) * glm::cos(angleY)
	);
	newDir = glm::normalize(newDir);
	dir = newDir;
	updateMatrix();
}