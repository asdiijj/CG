#include <iostream>
#include <vector>
#include <algorithm>

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
#include "point.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

using namespace std;

static void glfw_error_callback(int error, const char* description);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
vector<Point> getCurve();
float* getLines(float t);
Point getBezierPoints(float t);
float B(float i, float n, float t);
float C(int n, int i);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
double x_pos = 0;
double y_pos = 0;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
float time = 0.0f;
bool change = false;

vector<Point> points;

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
	glfwSetMouseButtonCallback(window, mouse_button_callback);

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

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glEnable(GL_DEPTH_TEST);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	Shader shader("shader.vs", "shader.fs");

	float* vertices = NULL;
	float* curveVertices = NULL;

	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);

	unsigned int curveVAO;
	glGenVertexArrays(1, &curveVAO);
	glBindVertexArray(curveVAO);

	bool colorMenu = false;
	bool select1 = true;
	bool select2 = false;
	bool select3 = false;
	bool select4 = false;
	bool select5 = false;
	bool select6 = false;
	float currentFrame = 0.0f;
	int index = 0, size = 0, curveSize = 0;

	shader.use();

	// Main loop
	while (!glfwWindowShouldClose(window)) {
		currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		time += deltaTime / 10;
		if (time > 1.0f) {
			time = 0.0f;
		}

		processInput(window);

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (colorMenu) {
			ImGui::Begin("Select", &colorMenu);
			//ImGui::Text("Press 'X' to show cursor.");
			if (ImGui::Checkbox("Ortho", &select1)) {
				select1 = true;
				select4 = select5 = select6 = select3 = select2 = false;
			}
			else if (ImGui::Checkbox("Perspective", &select2)) {
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
			//ImGui::SliderFloat("ambient", &ambient, 0.0f, 1.0f);
			ImGui::End();
		}

		// Rendering
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		int display_w, display_h;
		glfwMakeContextCurrent(window);
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// render Bezier curve
		if (change) {
			if (curveVertices != NULL) {
				delete[] curveVertices;
			}
			vector<Point> curve = getCurve();
			curveSize = curve.size() * 2;
			curveVertices = new float[curveSize];
			index = 0;
			for (auto it = curve.begin(); it != curve.end(); it++) {
				curveVertices[index++] = it->x;
				curveVertices[index++] = it->y;
			}
			change = false;
		}
		shader.setBool("colorful", true);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * curveSize, curveVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glBindVertexArray(curveVAO);
		glDrawArrays(GL_POINTS, 0, curveSize / 2);
		glBindVertexArray(0);

		// render points and lines
		if (vertices != NULL) {
			delete[] vertices;
		}
		size = 0;
		for (int i = points.size(); i > 1; i--) {
			size += (i - 1) * 2;
		}
		size *= 2;
		vertices = getLines(time);
		shader.setBool("colorful", false);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * size, vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, size / 2);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glDeleteVertexArrays(1, &VAO);
	glDeleteVertexArrays(1, &curveVAO);
	glDeleteBuffers(1, &VBO);

	if (vertices != NULL) {
		delete[] vertices;
	}
	if (curveVertices != NULL) {
		delete[] curveVertices;
	}

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
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	x_pos = xpos;
	y_pos = ypos;
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS) switch (button)
	{
	case GLFW_MOUSE_BUTTON_LEFT:
		points.push_back(Point((x_pos - SCR_WIDTH / 2) / (SCR_WIDTH / 2),
			(SCR_HEIGHT / 2 - y_pos) / (SCR_HEIGHT / 2)));
		change = true;
		break;
	case GLFW_MOUSE_BUTTON_MIDDLE:
		break;
	case GLFW_MOUSE_BUTTON_RIGHT:
		points.pop_back();
		change = true;
		break;
	default:
		return;
	}
	return;
}

vector<Point> getCurve() {
	vector<Point> curve;
	for (float t = 0.0f; t <= 1.0f; t += 0.0001f) {
		curve.push_back(getBezierPoints(t));
	}
	return curve;
}
float* getLines(float t) {
	vector<Point> lines(points);
	int size = 0;
	for (int i = points.size(); i > 1; i--) {
		size += (i - 1) * 2;
	}
	float* vertices = new float[size * 2];
	int index = 0;
	while (lines.size() > 1) {
		for (int i = 0; i < lines.size() - 1; i++) {
			vertices[index++] = lines[i].x;
			vertices[index++] = lines[i].y;
			vertices[index++] = lines[i + 1].x;
			vertices[index++] = lines[i + 1].y;
			lines[i] += (lines[i + 1] - lines[i]) * t;
		}
		lines.pop_back();
	}
	return vertices;
}
Point getBezierPoints(float t) {
	int n = points.size() - 1;
	Point point(0.0f, 0.0f);
	for (int i = 0; i <= n; i++) {
		point += points[i] * B(i, n, t);
	}
	return point;
}
float B(float i, float n, float t) {
	return C(n, i)* pow(t, i)* pow(1 - t, n - i);
}
float C(int n, int i) {
	// select i from n
	int border = n - i;
	if (n - i < i) {
		border = i;
		i = n - border;
	}
	border += 1;
	int numerator = 1, denominator = 1;
	for (int j = n; j >= border; j--) {
		numerator *= j;
	}
	for (i; i >= 1; i--) {
		denominator *= i;
	}
	return numerator / denominator;
}
