#include <iostream>
#include <vector>
#include <algorithm>
#include "camera.hpp"

#include "imgui\imgui.h"
#include "imgui\imgui_impl_glfw.h"
#include "imgui\imgui_impl_opengl3.h"

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>
#include "shader_s.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

using namespace std;

static void glfw_error_callback(int error, const char* description);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera camera;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool firstMouse = true;
float lastX = SCR_WIDTH / 2.0;
float lastY = SCR_HEIGHT / 2.0;
float fov = 45.0f;
bool freeView = false;

glm::vec3 lightPos(6.0f, 4.0f, 0.0f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
glm::vec3 viewPos(0.0f, 0.0f, -30.0f);

int main(int, char**)
{
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return 1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Triangle", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
	bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
	bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
	bool err = gladLoadGL() == 0;
#else
	bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
	if (err)
	{
		fprintf(stderr, "Failed to initialize OpenGL loader!\n");
		return 1;
	}

	int success;
	char infoLog[512];

	glEnable(GL_DEPTH_TEST);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	// shader
	Shader phongShader("phong.vs", "phong.fs");
	Shader gouraudShader("gouraud.vs", "gouraud.fs");
	Shader lightShader("light.vs", "light.fs");

	float vertices[] = {
		-2.0f, -2.0f, -2.0f,  0.4f, 0.5f, 0.8f,  0.0f, 0.0f, -1.0f,
		2.0f, -2.0f, -2.0f,  0.4f, 0.5f, 0.8f,  0.0f, 0.0f, -1.0f,
		2.0f,  2.0f, -2.0f,  0.4f, 0.5f, 0.8f,  0.0f, 0.0f, -1.0f,
		2.0f,  2.0f, -2.0f,  0.4f, 0.5f, 0.8f,  0.0f, 0.0f, -1.0f,
		-2.0f,  2.0f, -2.0f,  0.4f, 0.5f, 0.8f,  0.0f, 0.0f, -1.0f,
		-2.0f, -2.0f, -2.0f,  0.4f, 0.5f, 0.8f,  0.0f, 0.0f, -1.0f,

		-2.0f, -2.0f,  2.0f,  1.0f, 0.0f, 0.0f,  0.0f,  0.0f,  1.0f,
		2.0f, -2.0f,  2.0f,  1.0f, 0.0f, 0.0f,  0.0f,  0.0f,  1.0f,
		2.0f,  2.0f,  2.0f,  1.0f, 0.0f, 0.0f,  0.0f,  0.0f,  1.0f,
		2.0f,  2.0f,  2.0f,  1.0f, 0.0f, 0.0f,  0.0f,  0.0f,  1.0f,
		-2.0f,  2.0f,  2.0f,  1.0f, 0.0f, 0.0f,  0.0f,  0.0f,  1.0f,
		-2.0f, -2.0f,  2.0f,  1.0f, 0.0f, 0.0f,  0.0f,  0.0f,  1.0f,

		-2.0f,  2.0f,  2.0f,  0.0f, 1.0f, 0.0f,  -1.0f,  0.0f,  0.0f,
		-2.0f,  2.0f, -2.0f,  0.0f, 1.0f, 0.0f,  -1.0f,  0.0f,  0.0f,
		-2.0f, -2.0f, -2.0f,  0.0f, 1.0f, 0.0f,  -1.0f,  0.0f,  0.0f,
		-2.0f, -2.0f, -2.0f,  0.0f, 1.0f, 0.0f,  -1.0f,  0.0f,  0.0f,
		-2.0f, -2.0f,  2.0f,  0.0f, 1.0f, 0.0f,  -1.0f,  0.0f,  0.0f,
		-2.0f,  2.0f,  2.0f,  0.0f, 1.0f, 0.0f,  -1.0f,  0.0f,  0.0f,

		2.0f,  2.0f,  2.0f,  0.0f, 0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
		2.0f,  2.0f, -2.0f,  0.0f, 0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
		2.0f, -2.0f, -2.0f,  0.0f, 0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
		2.0f, -2.0f, -2.0f,  0.0f, 0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
		2.0f, -2.0f,  2.0f,  0.0f, 0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
		2.0f,  2.0f,  2.0f,  0.0f, 0.0f, 1.0f,  1.0f,  0.0f,  0.0f,

		-2.0f, -2.0f, -2.0f,  0.5f, 0.5f, 0.5f,  0.0f, -1.0f,  0.0f,
		2.0f, -2.0f, -2.0f,  0.5f, 0.5f, 0.5f,  0.0f, -1.0f,  0.0f,
		2.0f, -2.0f,  2.0f,  0.5f, 0.5f, 0.5f,  0.0f, -1.0f,  0.0f,
		2.0f, -2.0f,  2.0f,  0.5f, 0.5f, 0.5f,  0.0f, -1.0f,  0.0f,
		-2.0f, -2.0f,  2.0f,  0.5f, 0.5f, 0.5f,  0.0f, -1.0f,  0.0f,
		-2.0f, -2.0f, -2.0f,  0.5f, 0.5f, 0.5f,  0.0f, -1.0f,  0.0f,

		-2.0f,  2.0f, -2.0f,  0.5f, 0.5f, 0.0f,  0.0f,  1.0f,  0.0f,
		2.0f,  2.0f, -2.0f,  0.5f, 0.5f, 0.0f,  0.0f,  1.0f,  0.0f,
		2.0f,  2.0f,  2.0f,  0.5f, 0.5f, 0.0f,  0.0f,  1.0f,  0.0f,
		2.0f,  2.0f,  2.0f,  0.5f, 0.5f, 0.0f,  0.0f,  1.0f,  0.0f,
		-2.0f,  2.0f,  2.0f,  0.5f, 0.5f, 0.0f,  0.0f,  1.0f,  0.0f,
		-2.0f,  2.0f, -2.0f,  0.5f, 0.5f, 0.0f,  0.0f,  1.0f,  0.0f
	};
	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	//glGenBuffers(1, &EBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 9 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 9 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	bool colorMenu = true;
	bool select1 = true;
	bool select2 = false;
	bool select3 = false;
	bool select4 = false;
	bool select5 = false;
	bool select6 = false;

	Shader& shader = phongShader;

	float ambient = 0.1f;
	float specular = 0.5f;
	int shininess = 32;

	// Main loop
	while (!glfwWindowShouldClose(window)) {
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (colorMenu) {
			ImGui::Begin("Select", &colorMenu);
			//ImGui::Text("Press 'X' to show cursor.");
			if (ImGui::Checkbox("Phong Shading", &select1)) {
				select1 = true;
				select4 = select5 = select6 = select3 = select2 = false;
			}
			else if (ImGui::Checkbox("Gouraud Shading", &select2)) {
				select2 = true;
				select4 = select5 = select6 = select1 = select3 = false;
			}
			/*else if (ImGui::Checkbox("View Changin", &select3)) {
				select3 = true;
				select4 = select5 = select6 = select1 = select2 = false;
			}
			else if (ImGui::Checkbox("Camera", &select4)) {
				select4 = true;
				select3 = select5 = select6 = select1 = select2 = false;
			}
			else if (ImGui::Checkbox("Show Scaling", &select5)) {
				select5 = true;
				select4 = select3 = select6 = select1 = select2 = false;
			}
			else if (ImGui::Checkbox("Show my idea", &select6)) {
				select6 = true;
				select4 = select3 = select5 = select1 = select2 = false;
			}*/
			/*if (ImGui::Checkbox("Free camera", &freeView)) {
				if (freeView) {
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				}
				else {
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				}
			}*/
			ImGui::SliderFloat("ambient", &ambient, 0.0f, 1.0f);
			ImGui::SliderFloat("specular", &specular, 0.0f, 2.0f);
			ImGui::SliderInt("shininess", &shininess, 10, 50);
			ImGui::End();
		}

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		if (freeView) {
			viewPos = camera.getPosition();
			view = camera.getView();
		}
		else {
			viewPos = glm::vec3(0.0f, -4.0f, -20.0f);
			view = glm::translate(view, viewPos);
			view = glm::rotate(view, glm::radians(25.0f), glm::vec3(1.0f, -1.0f, 0.0f));
		}

		lightPos.x = 6 * cos(glfwGetTime() * 1.0f);
		lightPos.z = 6 * sin(glfwGetTime() * 1.0f);

		if (select1) {
			shader = phongShader;
		}
		else if (select2) {
			shader = gouraudShader;
		}

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwMakeContextCurrent(window);
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		shader.use();
		shader.setVec3("lightColor", lightColor);
		shader.setVec3("lightPos", lightPos);
		shader.setVec3("viewPos", viewPos);
		shader.setMat4("model", model);
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);
		shader.setFloat("ambientStrength", ambient);
		shader.setFloat("specularStrength", specular);
		shader.setFloat("shininess", shininess);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		lightShader.use();
		lightShader.setVec3("aColor", lightColor);
		model = glm::mat4(1.0f);
		//model = glm::rotate(model, glm::radians((float)glfwGetTime() * 100), glm::vec3(0.0f, 1.0, 0.0f));
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));
		lightShader.setMat4("model", model);
		lightShader.setMat4("view", view);
		lightShader.setMat4("projection", projection);

		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.moveForward(deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.moveBackward(deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.moveLeft(deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.moveRight(deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
		freeView = false;
	}
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;
	camera.moveUp(fov);
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	camera.setFront(xoffset, yoffset);
}
