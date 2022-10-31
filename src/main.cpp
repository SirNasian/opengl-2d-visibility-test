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
		cursor_x =  ((cursor_x/320.0f)-1.0f);
		cursor_y = -((cursor_y/320.0f)-1.0f);

		const float vertices[] = {
			(float)(cursor_x), (float)(cursor_y),
			-0.9f,  0.9f,
			 0.9f,  0.9f,
			 0.9f, -0.9f,
			-0.9f, -0.9f,
			-0.9f,  0.9f,
		};
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(vertices)/sizeof(vertices[0])/2);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	return 0;
}
