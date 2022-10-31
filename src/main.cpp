#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if ((key == GLFW_KEY_Q) && (action == GLFW_PRESS))
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}


const char *vertex_source = R"glsl(
	#version 330 core
	in vec2 position;
	out vec2 world_pos;
	void main()
	{
		world_pos = position;
		gl_Position = vec4(position, 0.0, 1.0);
	}
)glsl";

const char *fragment_source = R"glsl(
	#version 330 core
	in vec2 world_pos;
	out vec4 colour;
	uniform vec2 cursor_pos;
	uniform vec4 line_segments[1024];

	float calcVisibility(in vec2 line_start, in vec2 line_end)
	{
		if (distance(line_end, line_start) == 0.0) return 1.0;

		vec2 ray         = world_pos - cursor_pos;
		vec2 line_normal = cross(vec3(line_start - line_end, 0.0), vec3(0.0, 0.0, 1.0)).xy;
		if (dot(line_normal, ray) == 0.0) return 1.0;

		float d = dot(line_normal, line_start);
		float x = (d - dot(line_normal, cursor_pos)) / dot(line_normal, normalize(ray));
		if ((x >= 0.0) && (x <= distance(world_pos, cursor_pos)))
		{
			vec2 ray_normal = cross(vec3(world_pos - cursor_pos, 0.0), vec3(0.0, 0.0, 1.0)).xy;
			d = dot(ray_normal, cursor_pos);
			x = (d - dot(ray_normal, line_start)) / dot(ray_normal, normalize(line_end - line_start));
			if ((x >= 0.0) && (x <= distance(line_end, line_start))) return 0.0;
		}

		return 1.0;
	}

	void main()
	{
		float visibility = 1.0;
		for (int i = 0; i < 1024; i++)
			visibility *= calcVisibility(line_segments[i].xy, line_segments[i].zw);

		colour = visibility
		       * (1.0 - distance(world_pos, cursor_pos))
		       * vec4(1.0, 1.0, 1.0, 1.0);
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

const float vertices[] = {
	-0.9f,  0.9f,
	 0.9f,  0.9f,
	 0.9f, -0.9f,
	-0.9f, -0.9f,
};

const unsigned int indices[] = {
	0, 1, 2, 3,
};

unsigned int createVAO(unsigned int shader_program)
{
	unsigned int vao, vbo, ebo;

	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glCreateBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glCreateBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	unsigned int position_attribute = glGetAttribLocation(shader_program, "position");
	glVertexAttribPointer(position_attribute, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), 0);
	glEnableVertexAttribArray(position_attribute);

	glBindVertexArray(0);

	return vao;
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

	unsigned int shader_program = createShaderProgram(vertex_source, fragment_source);
	unsigned int vao = createVAO(shader_program);
	glUseProgram(shader_program);
	glBindVertexArray(vao);

	double cursor_x, cursor_y;
	unsigned int cursor_pos_uniform = glGetUniformLocation(shader_program, "cursor_pos");

	float line_segments[] = {
		// Triangle
		 0.1f,  0.1f,  0.2f,  0.2f,
		 0.1f,  0.1f,  0.2f,  0.0f,
		 0.2f,  0.2f,  0.2f,  0.0f,
		// Square
		-0.5f, -0.5f, -0.5f, -0.2f,
		-0.5f, -0.5f, -0.2f, -0.5f,
		-0.2f, -0.2f, -0.5f, -0.2f,
		-0.2f, -0.2f, -0.2f, -0.5f,
	};
	glUniform4fv(glGetUniformLocation(shader_program, "line_segments"), sizeof(line_segments)/sizeof(line_segments[0])/4, line_segments);

	double time_curr, time_last, time_delta;
	time_last = glfwGetTime();

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
		glUniform2f(cursor_pos_uniform, (cursor_x/320.0f)-1.0f, -((cursor_y/320.0f)-1.0f));
		glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	return 0;
}
