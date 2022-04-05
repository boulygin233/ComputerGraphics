// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>


int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1280, 720, "Two triangles, red and blue", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint redTriangle = LoadShaders("SimpleTransform.vertexshader", "RedTriangle.fragmentshader");
	GLuint blueTriangle = LoadShaders("SimpleTransform.vertexshader", "BlueTriangle.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint redVertexes = glGetUniformLocation(redTriangle, "MVP");
	GLuint blueVertexes = glGetUniformLocation(blueTriangle, "MVP");

	// Projection matrix : 45? Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 100.0f);
	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);

	static const GLfloat g_vertex_buffer_data_first[] = {
			1.0f, -1.0f, 0.01f,
		   -1.0f, 1.0f, 0.01f,
		   0.3f,  -1.0f, 0.01f,
	};

	static const GLfloat g_vertex_buffer_data_second[] = {
		-1.0f, -1.0f, -0.01f,
		   1.0f, 1.0f, -0.01f,
		   -0.3f,  -1.0f, -0.01f
	};

	GLuint vertexbuffer[2];
	glGenBuffers(2, &vertexbuffer[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data_first), g_vertex_buffer_data_first, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data_second), g_vertex_buffer_data_second, GL_STATIC_DRAW);

	glm::vec3 position = glm::vec3(0, 0, 7);

	float angle = 0;
	float speed = 1.0f;
	float radius = 7.0f;

	glm::mat4 ViewMatrix;
	double lastTime = glfwGetTime();

	do {

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Compute time difference between current and last frame
		double currentTime = glfwGetTime();
		float deltaTime = float(currentTime - lastTime);

		// Compute new orientation
		angle += deltaTime * speed;
		position = glm::vec3(radius * cos(angle), 0, radius * sin(angle));

		// Camera matrix
		ViewMatrix = glm::lookAt(
			position,           // Camera is here
			glm::vec3(0, 0, 0), // at the same position
			glm::vec3(0, 1, 0)  // Head is up
		);

		// For the next frame, the "last time" will be "now"
		lastTime = currentTime;

		// Our ModelViewProjection : multiplication of our 3 matrices
		glm::mat4 MVP = Projection * ViewMatrix * Model; // Remember, matrix multiplication is the other way around


		// --- First triangle
		// Use our shader
		glUseProgram(redTriangle);

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(redVertexes, 1, GL_FALSE, &MVP[0][0]);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[0]);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 3);

		// --- Second triangle
		glUseProgram(blueTriangle);

		glUniformMatrix4fv(blueVertexes, 1, GL_FALSE, &MVP[0][0]);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[1]);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		glDisableVertexAttribArray(0);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO and shader
	glDeleteBuffers(2, vertexbuffer);
	glDeleteProgram(redTriangle);
	glDeleteProgram(blueTriangle);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
