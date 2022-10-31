#include <iostream>
#include <list>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "line.hpp"

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if ((key == GLFW_KEY_Q) && (action == GLFW_PRESS))
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

std::list<LineSegment> line_segments;
glm::vec2 last_point;
glm::vec2 cursor_pos;

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS)
			last_point = cursor_pos;
		else if (action == GLFW_RELEASE)
		{
			glm::vec2 normal = glm::normalize(LineSegment(last_point, cursor_pos).normal());
			glm::vec2 p1 = last_point + (normal * 0.01f);
			glm::vec2 p2 = last_point - (normal * 0.01f);
			glm::vec2 p3 = cursor_pos + (normal * 0.01f);
			glm::vec2 p4 = cursor_pos - (normal * 0.01f);
			line_segments.push_back(LineSegment(p1, p2));
			line_segments.push_back(LineSegment(p2, p4));
			line_segments.push_back(LineSegment(p4, p3));
			line_segments.push_back(LineSegment(p3, p1));
		}
	}
	else if ((button == GLFW_MOUSE_BUTTON_RIGHT) && (action == GLFW_PRESS) && (!line_segments.empty()))
		for (int _ = 0; _ < 4; _++)
			line_segments.pop_back();
}

const char *vertex_source = R"glsl(
	#version 330 core
	in vec2 position;
	out vec2 world_pos;
	void main()
	{
		world_pos   = position;
		gl_Position = vec4(position, 0.0, 1.0);
	}
)glsl";

const char *fragment_source = R"glsl(
	#version 330 core
	in vec2 world_pos;
	out vec4 fragment_colour;
	uniform vec2 cursor_pos;
	void main()
	{
		fragment_colour = (1.0 - distance(world_pos, cursor_pos)) * vec4(1.0, 1.0, 1.0, 1.0);
	}
)glsl";

unsigned int createShaderProgram(const char *vertex_source, const char *fragment_source)
{
	unsigned int shader_program  = glCreateProgram();
	unsigned int vertex_shader   = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vertex_shader, 1, &vertex_source, NULL);
	glCompileShader(vertex_shader);

	glShaderSource(fragment_shader, 1, &fragment_source, NULL);
	glCompileShader(fragment_shader);

	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);

	return shader_program;
}

unsigned int createVAO(unsigned int shader_program, unsigned int &vbo)
{
	unsigned int vao;

	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glCreateBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	unsigned int position_attribute = glGetAttribLocation(shader_program, "position");
	glVertexAttribPointer(position_attribute, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), 0);
	glEnableVertexAttribArray(position_attribute);

	glBindVertexArray(0);

	return vao;
}

float getAngle(glm::vec2 origin, glm::vec2 target)
{
	glm::vec2 v1 = glm::vec2(-1.0f, 0.0f);
	glm::vec2 v2 = glm::normalize(target - origin);
	return (v2.y > 0.0f)
		? glm::acos(glm::dot(v1, v2))
		: glm::radians(360.0f) - glm::acos(glm::dot(v1, v2));
}

bool comparePoints(glm::vec2 p1, glm::vec2 p2)
{
	return (getAngle(p1, cursor_pos) < getAngle(p2, cursor_pos));
}

int main()
{
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWwindow *window = glfwCreateWindow(640, 640, "OpenGL 2D Visibility Test", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	if (glewInit() != GLEW_OK)
	{
		glfwTerminate();
		return -1;
	}

	unsigned int shader_program, vao, vbo;
	shader_program = createShaderProgram(vertex_source, fragment_source);
	vao            = createVAO(shader_program, vbo);
	glUseProgram(shader_program);
	glBindVertexArray(vao);

	unsigned int cursor_pos_attribute = glGetUniformLocation(shader_program, "cursor_pos");

	double cursor_x, cursor_y;
	double time_curr, time_last, time_delta;
	time_last = glfwGetTime();

	// Outer Square
	line_segments.push_back(LineSegment(glm::vec2(-0.9f, -0.9f), glm::vec2(-0.9f, 0.9f)));
	line_segments.push_back(LineSegment(glm::vec2(-0.9f, -0.9f), glm::vec2(0.9f, -0.9f)));
	line_segments.push_back(LineSegment(glm::vec2(0.9f, 0.9f),   glm::vec2(-0.9f, 0.9f)));
	line_segments.push_back(LineSegment(glm::vec2(0.9f, 0.9f),   glm::vec2(0.9f, -0.9f)));

	// Inner Square
	line_segments.push_back(LineSegment(glm::vec2(-0.4f, -0.4f), glm::vec2(-0.4f, 0.4f)));
	line_segments.push_back(LineSegment(glm::vec2(-0.4f, -0.4f), glm::vec2(0.4f, -0.4f)));
	line_segments.push_back(LineSegment(glm::vec2(0.4f, 0.4f),   glm::vec2(-0.4f, 0.4f)));
	line_segments.push_back(LineSegment(glm::vec2(0.4f, 0.4f),   glm::vec2(0.4f, -0.4f)));

	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	while (!glfwWindowShouldClose(window))
	{
		time_curr  = glfwGetTime();
		time_delta = time_curr - time_last;
//		if (time_delta < 1.0f/60.0f) continue;
		time_last  = time_curr;
		printf("FPS: %.0f\n", 1.0f/time_delta);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		glfwGetCursorPos(window, &cursor_x, &cursor_y);
		cursor_pos.x =  ((cursor_x/320.0f)-1.0f);
		cursor_pos.y = -((cursor_y/320.0f)-1.0f);

		glm::vec2 point;
		float vertices[line_segments.size()*8];
		unsigned int index = 0;
		for (LineSegment line_segment: line_segments)
		{
			point = line_segment.p1 + (glm::normalize(line_segment.p1 - cursor_pos) * 1024.0f);
			vertices[(index*8)+0] = line_segment.p1.x;
			vertices[(index*8)+1] = line_segment.p1.y;
			vertices[(index*8)+2] = point.x;
			vertices[(index*8)+3] = point.y;

			point = line_segment.p2 + (glm::normalize(line_segment.p2 - cursor_pos) * 1024.0f);
			vertices[(index*8)+4] = line_segment.p2.x;
			vertices[(index*8)+5] = line_segment.p2.y;
			vertices[(index*8)+6] = point.x;
			vertices[(index*8)+7] = point.y;

			index++;
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0xFF);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		for (index = 0; index < line_segments.size(); index++)
			glDrawArrays(GL_TRIANGLE_STRIP, index*4, 4);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glStencilMask(0x00);

		float square[] = {
			-1.0f,  1.0f,
			 1.0f,  1.0f,
			-1.0f, -1.0f,
			 1.0f, -1.0f,
		};
		glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW);
		glUniform2f(cursor_pos_attribute, cursor_pos.x, cursor_pos.y);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	return 0;
}
