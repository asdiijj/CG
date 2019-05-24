#include <GL/gl3w.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

class Camera {
public:
	Camera() {
	}
	void moveForward(GLfloat const deltaTime) {
		cameraSpeed = speed * deltaTime;
		cameraPos += cameraSpeed * cameraFront;
	}
	void moveBackward(GLfloat const deltaTime) {
		cameraSpeed = speed * deltaTime;
		cameraPos -= cameraSpeed * cameraFront;
	}
	void moveLeft(GLfloat const deltaTime) {
		cameraSpeed = speed * deltaTime;
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}
	void moveRight(GLfloat const deltaTime) {
		cameraSpeed = speed * deltaTime;
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}
	void moveUp(float const fov) {
		cameraPos += glm::vec3(0.0f, lastFov - fov, 0.0f);
		lastFov = fov;
	}
	void setFront(GLfloat const xoffset, GLfloat const yoffset) {
		yaw += xoffset * sensitivity;
		pitch += yoffset * sensitivity;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 front;
		front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
		front.y = sin(glm::radians(pitch));
		front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
		cameraFront = glm::normalize(front);
	}
	glm::mat4 getView() {
		return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	}
	glm::vec3 getPosition() {
		return cameraPos;
	}
private:
	glm::mat4 view = glm::mat4(1.0f);
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, -30.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	float cameraSpeed = 1.0f;
	float speed = 7.0f;
	float sensitivity = 0.1f;
	float yaw = -90.0f;
	float pitch = 0.0f;
	float lastFov = 45.0f;
};
