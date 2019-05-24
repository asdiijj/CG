#version 330 core

layout (location = 0) in vec2 aPos;
out vec3 color;

uniform bool colorful;

void main() {
	gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
	color = vec3(1.0, 1.0, 1.0);
	if (colorful) {
		color = vec3(1.0, 1.0, 0.5);
	}
}