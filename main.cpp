#include <iostream>
#include <string>
#include <fstream>
#include <array>
#include <cstdlib>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#define MAX_PARTICLES 999

enum VAO_ID {SpongeVAO, WaterVAO, ParticleVAO, NumVAOs};
enum Buffer_ID {SpongeVBO, SpongeEBO, WaterVBO, ParticleVBO, NumBuffers};
enum Texture_ID {SpongeTexture, NumTextures};
enum ShaderProgram_ID { SpongeProgram, WaterProgram, ParticleProgram, NumPrograms};
enum Attrib_ID {PositionAttrib = 0};

struct Particle {
	glm::vec2 position;
	glm::vec2 velocity;
	bool active{false};
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

GLuint CreateShader(GLenum shadertype, std::string filename);

int main()
{
	srand(time(0));

	// Initialize GLFW for OpenGL 3.3 Core.
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "Sponge", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	glfwMakeContextCurrent(window);

	// GLAD: Load all OpenGL function pointers.
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Allow viewport to be dynamic (must be after GLAD for functions, but GLAD requires glfw)
	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// Shader Program Creation.
	GLuint ShaderPrograms[NumPrograms];
	ShaderPrograms[SpongeProgram] = glCreateProgram();
	ShaderPrograms[WaterProgram] = glCreateProgram();
	ShaderPrograms[ParticleProgram] = glCreateProgram();

	// Sponge Program Shaders.
	GLuint spongeVertexShader = CreateShader(GL_VERTEX_SHADER, "vert.shader");
	GLuint spongeFragmentShader = CreateShader(GL_FRAGMENT_SHADER, "frag.shader");
	glAttachShader(ShaderPrograms[SpongeProgram], spongeVertexShader);
	glAttachShader(ShaderPrograms[SpongeProgram], spongeFragmentShader);

	// Water Program Shaders.
	GLuint waterVertexShader = CreateShader(GL_VERTEX_SHADER, "passthrough_vert.shader");
	GLuint waterFragmentShader = CreateShader(GL_FRAGMENT_SHADER, "water_frag.shader");
	glAttachShader(ShaderPrograms[WaterProgram], waterVertexShader);
	glAttachShader(ShaderPrograms[WaterProgram], waterFragmentShader);

	// Particle Program Shaders.
	GLuint particleVertexShader = CreateShader(GL_VERTEX_SHADER, "passthrough_vert.shader");
	GLuint particleFragmentShader = CreateShader(GL_FRAGMENT_SHADER, "particle_frag.shader");
	glAttachShader(ShaderPrograms[ParticleProgram], particleVertexShader);
	glAttachShader(ShaderPrograms[ParticleProgram], particleFragmentShader);

	// Link, validate, and use all shader programs.
	for (GLuint program_idx = 0; program_idx < NumPrograms; ++program_idx) {
		glLinkProgram(ShaderPrograms[program_idx]);
		{
			int success;
			char infoLog[512];
			glGetProgramiv(ShaderPrograms[program_idx], GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(ShaderPrograms[program_idx], 512, NULL, infoLog);
				std::cout << "Error: Program link failed\n" << infoLog << std::endl;
			}
		}

		glValidateProgram(ShaderPrograms[program_idx]);
		{
			int success;
			char infoLog[512];
			glGetProgramiv(ShaderPrograms[program_idx], GL_VALIDATE_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(ShaderPrograms[program_idx], 512, NULL, infoLog);
				std::cout << "Error: Program validate failed\n" << infoLog << std::endl;
			}
		}
	}

	// Cleanup shader allocations.
	glDeleteShader(spongeVertexShader);
	glDeleteShader(spongeFragmentShader);
	glDeleteShader(waterVertexShader);
	glDeleteShader(waterFragmentShader);
	glDeleteShader(particleVertexShader);
	glDeleteShader(particleFragmentShader);

	 // 3D Vertices for a cube.
	GLfloat cubeVertices[] = {
		-0.5f, -0.5f, -0.5f,  // 0 
		 0.5f, -0.5f, -0.5f,  // 1
		 0.5f,  0.5f, -0.5f,  // 2
		-0.5f,  0.5f, -0.5f,  // 3
		-0.5f, -0.5f,  0.5f,  // 4
		 0.5f, -0.5f,  0.5f,  // 5
		 0.5f,  0.5f,  0.5f,  // 6
		-0.5f,  0.5f,  0.5f,  // 7
	};

	// Given above verts, indices for a cube.
	GLuint cubeIndices[] = {
		// Back Face
		1, 0, 3,
		3, 2, 1,

		// Front Face
		4, 5, 6,
		6, 7, 4,

		// Left Face
		7, 3, 0,
		0, 4, 7,

		// Right Face
		1, 2, 6,
		6, 5, 1,

		// Bottom Face.
		0, 1, 5,
		5, 4, 0,

		// Top Face
		6, 2, 3,
		3, 7, 6,
	};

	// Vertices to draw a simple square covering the screen, for the water effect.
	GLfloat screenVertices[] = {
		 1.0, -1.0,
		-1.0,  1.0,
		-1.0, -1.0,

		-1.0,  1.0,
		 1.0, -1.0,
		 1.0,  1.0,
	};

	//// VAO, VBO, EBO.
	GLuint VAOs[NumVAOs];
	GLuint Buffers[NumBuffers];
	glGenVertexArrays(NumVAOs, VAOs);
	glGenBuffers(NumBuffers, Buffers);

	// Sponge VAO.
	glBindVertexArray(VAOs[SpongeVAO]);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[SpongeVBO]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[SpongeEBO]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(PositionAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
	glEnableVertexAttribArray(PositionAttrib);

	// Water VAO.
	glBindVertexArray(VAOs[WaterVAO]);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[WaterVBO]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screenVertices), screenVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(PositionAttrib, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(PositionAttrib);

	// Particle System!
	std::array<Particle, MAX_PARTICLES> ParticlePool;

	GLfloat particlePos[MAX_PARTICLES][2];

	for (size_t i = 0; i < MAX_PARTICLES; ++i) {
		particlePos[i][0] = ParticlePool[i].position.x;
		particlePos[i][1] = ParticlePool[i].position.y;
	}

	// Particle VAO.
	glBindVertexArray(VAOs[ParticleVAO]);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[ParticleVBO]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(particlePos), particlePos, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(PositionAttrib, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(PositionAttrib);
	
	// Unbind.
	glBindVertexArray(0);

	// Uniform locations.
	GLuint modelLoc = glGetUniformLocation(ShaderPrograms[SpongeProgram], "model");
	GLuint viewLoc = glGetUniformLocation(ShaderPrograms[SpongeProgram], "view");
	GLuint projectionLoc = glGetUniformLocation(ShaderPrograms[SpongeProgram], "projection");
	GLuint textureLoc = glGetUniformLocation(ShaderPrograms[SpongeProgram], "texture1");
	GLuint isWetLoc = glGetUniformLocation(ShaderPrograms[SpongeProgram], "isWet");

	GLuint screenXLoc = glGetUniformLocation(ShaderPrograms[WaterProgram], "screenX");
	GLuint screenYLoc = glGetUniformLocation(ShaderPrograms[WaterProgram], "screenY");
	GLuint waterLevelLoc = glGetUniformLocation(ShaderPrograms[WaterProgram], "WaterLevel");
	GLuint timeLoc = glGetUniformLocation(ShaderPrograms[WaterProgram], "time");

	// Texture loading with stb_image
	GLuint Textures[NumTextures];
	glGenTextures(NumTextures, Textures);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Textures[SpongeTexture]);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);
	int width, height, nrChannels;
	unsigned char* data = stbi_load("SpongeTextureEdges.png", &width, &height, &nrChannels, 0);

	if (data) {
		glActiveTexture(GL_TEXTURE0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	} else {
		std::cout << "Failed to load sponge texture " << std::endl;
	}
	stbi_image_free(data);

	// Texture is at texture unit 0.
	glUseProgram(ShaderPrograms[SpongeProgram]);
	glBindVertexArray(VAOs[SpongeVAO]);
	glUniform1i(textureLoc, 0);
	glBindVertexArray(0);

	// Don't draw things that are obscured.
	glEnable(GL_DEPTH_TEST);

	//// Back-face culling, with CCW as the direction.
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	// Enable Alpha blending.
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	// Allow point size to be changed.
	glEnable(GL_PROGRAM_POINT_SIZE);

	float lastTime = glfwGetTime();
	float step = 0.25f;
	bool isWet = false;
	bool wasClicking = false;
	GLfloat waterLevel = 0.5f;
	bool cursorWasUnderWater = false;

	//// Render loop.
	while (!glfwWindowShouldClose(window)) {
		float curTime = glfwGetTime();
		float deltaTime = glfwGetTime() - lastTime;
		double cursorXPos, cursorYPos;

		// Quit if desired
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, true);
		}

		// If holding up, increase water level.  If holding down, decrease water level.
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
			waterLevel += step * deltaTime;
			if (waterLevel > 1.0f) {
				waterLevel = 1.0f;
			}
		}

		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
			waterLevel -= step * deltaTime;
			if (waterLevel < 0.0f) {
				waterLevel = 0.0f;
			}
		}

		glfwGetCursorPos(window, &cursorXPos, &cursorYPos);
		int width, height;
		glfwGetWindowSize(window, &width, &height);

		// Convert from screen space to viewport space (abs > 1 => offscreen)
		cursorXPos = (cursorXPos / width) * 2 - 1;
		cursorYPos = (cursorYPos / height) * -2 + 1;

		// Matrix location on screen.
		glm::mat4 model = glm::mat4(1.0f);

		// Since FOV is vertical, need to scale x offset by aspect ratio.
		// Note: projection matrix makes this control the center of the cube, not the center of the front face.
		model = glm::translate(model, glm::vec3(cursorXPos * width/height, cursorYPos, -2.5f));

		// Shrink by 5x; vertices are quite large.
		model = glm::scale(model, glm::vec3(0.35, 0.35, 0.35));
		
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT == GLFW_PRESS)) {
			// Squish the sponge if mouse button down.
			model = glm::scale(model, glm::vec3(1.25, 0.6, 0.8));
			
			// Spawn particles if wet and we just left clicked.
			if (!wasClicking && isWet) {
				size_t particlesToSpawn = 100;
			
				for (GLint i = 0; i < MAX_PARTICLES && particlesToSpawn > 0; ++i) {
					if (ParticlePool[i].active == false) {
						// Could apply jitter to position, but as is the cube just functions as an emitter, which is fine by me.
						ParticlePool[i].position = glm::vec2(cursorXPos, cursorYPos);
						// X jitter means += 4.0
						float velocityXJitter = ((float)rand() / RAND_MAX) * 8 - 4;
						// Y jitter means +- 1
						float velocityYJitter = ((float)rand() / RAND_MAX) * 2 - 1;
						ParticlePool[i].velocity = glm::vec2(velocityXJitter, 0.3f + velocityYJitter);
						ParticlePool[i].active = true;
						--particlesToSpawn;
					}
				}
			}

			isWet = false;
			wasClicking = true;
		} else {
			wasClicking = false;
		}

		// Sponge becomes wet when ypos < waterLevel.  ypos[-1,1], while waterLevel is [0,1], so we need to transform one into the others' coordinate space.
		if (cursorYPos < (waterLevel * 2 - 1)) {
			
			// Spawn particles when we initially dunk the sponge under the water.
			if (!cursorWasUnderWater) {
				size_t particlesToSpawn = 20;

				for (GLint i = 0; i < MAX_PARTICLES && particlesToSpawn > 0; ++i) {
					if (ParticlePool[i].active == false) {
						// Could apply jitter to position, but as is the cube just functions as an emitter, which is fine by me.
						ParticlePool[i].position = glm::vec2(cursorXPos, cursorYPos);
						// X jitter means [-1, 1]
						float velocityXJitter = ((float)rand() / RAND_MAX) * 2 - 1;
						// Y jitter means [0,2]
						float velocityYJitter = ((float)rand() / RAND_MAX) * 2;
						ParticlePool[i].velocity = glm::vec2(velocityXJitter, velocityYJitter);
						ParticlePool[i].active = true;
						--particlesToSpawn;
					}
				}
			}

			isWet = true;
			cursorWasUnderWater = true;
		} else {
			cursorWasUnderWater = false;
		}

		if (isWet) {
			// When wet, it's bigger because filled with a little water.
			model = glm::scale(model, glm::vec3(1.1f, 1.1f, 1.1f));
		}

		// View, set to identity, ensuring cursor position = model.
		glm::mat4 view = glm::mat4(1.0f);

		// Projection matrix, with a vertical FOV of 45.0 degrees.
		glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float) width / height, 0.1f, 100.0f);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(VAOs[SpongeVAO]);
		glUseProgram(ShaderPrograms[SpongeProgram]);
		
		glUniform1i(isWetLoc, isWet);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(proj));

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		glUseProgram(ShaderPrograms[ParticleProgram]);
		glBindVertexArray(VAOs[ParticleVAO]);
		
		// Update Particle Positions.
		float* mappedParticlePos = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		for (size_t i = 0; i < MAX_PARTICLES; ++i) {
			if (ParticlePool[i].active) {
				// Only active particles get position updates.

				ParticlePool[i].velocity = glm::vec2(ParticlePool[i].velocity.x, ParticlePool[i].velocity.y - 0.01f);
				ParticlePool[i].position = ParticlePool[i].position + ParticlePool[i].velocity * deltaTime;
				
				// Particles should bounce off the wall
				if (abs(ParticlePool[i].position.x) >= 0.99) {
					ParticlePool[i].velocity = glm::vec2(-ParticlePool[i].velocity.x, ParticlePool[i].velocity.y);
				}
				
				if (ParticlePool[i].position.y < -1.0) {
					// Particle has fallen past the bottom; it's no longer active.
					ParticlePool[i].active = false;
				}

				mappedParticlePos[i * 2 + 0] = ParticlePool[i].position.x;
				mappedParticlePos[i * 2 + 1] = ParticlePool[i].position.y;
			}

		}
		glUnmapBuffer(GL_ARRAY_BUFFER);

		// Render Particles.
		glPointSize((width / 800) * 5);

		for (GLint i = 0; i < MAX_PARTICLES; ++i) {
			if (ParticlePool[i].active) {
				glDrawArrays(GL_POINTS, i, 1);
			}
		}

		// Water drawn after particles, since particles can be blocked by water because at same z depth.
		glUseProgram(ShaderPrograms[WaterProgram]);
		glBindVertexArray(VAOs[WaterVAO]);

		glUniform1i(screenXLoc, width);
		glUniform1i(screenYLoc, height);
		glUniform1f(waterLevelLoc, waterLevel);
		glUniform1f(timeLoc, glfwGetTime());
		
		glDrawArrays(GL_TRIANGLES, 0, 6);


		glfwSwapBuffers(window);
		glfwPollEvents();
		lastTime = curTime;
	}

	return 0;
}

GLuint CreateShader(GLenum shaderType, std::string filename) {
	GLuint shader = glCreateShader(shaderType);
	std::ifstream is(filename);

	if (is.fail()) {
		std::cout << "Error opening " << filename;
		return 0;
	}

	std::string source;
	std::string line;
	while (!is.eof()) {
		std::getline(is, line);
		source += line + '\n';
	}
	const GLchar* sourceChar(source.c_str());
	glShaderSource(shader, 1, &sourceChar, 0);
	glCompileShader(shader);

	int success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "Error: Shader compilation failed for " << filename << "\n" << infoLog << std::endl;
	}

	return shader;
}