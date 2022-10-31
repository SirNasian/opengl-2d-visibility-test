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

const char *vertex_source = R"glsl(
	#version 330 core
	in vec2 position;
	void main()
	{
		gl_Position = vec4(position, 0.0, 1.0);
	}
)glsl";

const char *fragment_source = R"glsl(
	#version 330 core
	out vec4 fragment_colour;
	void main()
	{
		fragment_colour = vec4(1.0, 1.0, 1.0, 1.0);
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

glm::vec2 cursor_pos;
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

	double cursor_x, cursor_y;
	double time_curr, time_last, time_delta;
	time_last = glfwGetTime();

	std::list<LineSegment> line_segments;
	std::list<glm::vec2>   points;

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
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	while (!glfwWindowShouldClose(window))
	{
		time_curr  = glfwGetTime();
		time_delta = time_curr - time_last;
//		if (time_delta < 1.0f/60.0f) continue;
		time_last  = time_curr;
		printf("FPS: %.0f\n", 1.0f/time_delta);

		glClear(GL_COLOR_BUFFER_BIT);

		glfwGetCursorPos(window, &cursor_x, &cursor_y);
		cursor_pos.x =  ((cursor_x/320.0f)-1.0f);
		cursor_pos.y = -((cursor_y/320.0f)-1.0f);

		points.clear();
		for (const LineSegment &line_segment: line_segments)
		{
			points.push_back(line_segment.p1);
			points.push_back(line_segment.p2);
		}

		points.sort(comparePoints);
		points.unique();
		points.remove_if([line_segments](glm::vec2 point)
		{
			glm::vec2 intersect;
			for (LineSegment line_segment: line_segments)
				if (
					(line_segment.intersect(LineSegment(cursor_pos, point), intersect)) &&
					((glm::distance(cursor_pos, point) - glm::distance(cursor_pos, intersect)) > 0.01f)
				) return true;
			return false;
		});

		for (glm::vec2 &point: points)
		{
			float split_distance = 0.0001f;
			glm::vec2 normal = glm::normalize(glm::cross(glm::vec3(point - cursor_pos, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
			points.push_front(glm::vec2(point + (normal * split_distance)));
			point = glm::vec2(point - (normal * split_distance));
		}

		for (glm::vec2 &point: points)
		{
			glm::vec2 intersect;
			point = cursor_pos + (glm::normalize(point - cursor_pos) * 1024.0f);
			for (LineSegment line_segment: line_segments)
				if (
					(line_segment.intersect(LineSegment(cursor_pos, point), intersect)) &&
					(glm::distance(cursor_pos, intersect) < glm::distance(cursor_pos, point))
				) point = intersect;
		}

		points.sort(comparePoints);
		points.push_back(points.front());
		points.push_front(cursor_pos);
		float vertices[points.size()*2];
		unsigned int i = 0;
		for (glm::vec2 point: points)
		{
			vertices[(i*2)+0] = point.x;
			vertices[(i*2)+1] = point.y;
			i++;
		}

//		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glDrawArrays(GL_TRIANGLE_FAN, 0, points.size());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	return 0;
}
