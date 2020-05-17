#include "Demo.h"



Demo::Demo() {

}


Demo::~Demo() {
}



void Demo::Init() {
	BuildShaders();
	BuildDepthMap();
	/*BuildTexturedCube();
	BuildTexturedPlane();*/

	BuildColoredTiang();
	BuildColoredAtap();
	BuildColoredPlane();

	InitCamera();

}

void Demo::DeInit() {
	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &cubeEBO);
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &planeVBO);
	glDeleteBuffers(1, &planeEBO);
	glDeleteBuffers(1, &depthMapFBO);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void Demo::ProcessInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	// zoom camera
	// -----------
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		if (fovy < 90) {
			fovy += 0.0001f;
		}
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		if (fovy > 0) {
			fovy -= 0.0001f;
		}
	}

	// update camera movement 
	// -------------
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		MoveCamera(CAMERA_SPEED);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		MoveCamera(-CAMERA_SPEED);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		StrafeCamera(-CAMERA_SPEED);
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		StrafeCamera(CAMERA_SPEED);
	}

	// update camera rotation
	// ----------------------
	double mouseX, mouseY;
	double midX = screenWidth / 2;
	double midY = screenHeight / 2;
	float angleY = 0.0f;
	float angleZ = 0.0f;

	// Get mouse position
	glfwGetCursorPos(window, &mouseX, &mouseY);
	if ((mouseX == midX) && (mouseY == midY)) {
		return;
	}

	// Set mouse position
	glfwSetCursorPos(window, midX, midY);

	// Get the direction from the mouse cursor, set a resonable maneuvering speed
	angleY = (float)((midX - mouseX)) / 1000;
	angleZ = (float)((midY - mouseY)) / 1000;

	// The higher the value is the faster the camera looks around.
	viewCamY += angleZ * 2;

	// limit the rotation around the x-axis
	if ((viewCamY - posCamY) > 10) {
		viewCamY = posCamY + 10;
	}
	if ((viewCamY - posCamY) < -10) {
		viewCamY = posCamY - 10;
	}
	RotateCamera(-angleY);
}

void Demo::Update(double deltaTime) {
}

void Demo::Render() {

	glEnable(GL_DEPTH_TEST);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	// Step 1 Render depth of scene to texture
	// ----------------------------------------
	glm::mat4 lightProjection, lightView;
	glm::mat4 lightSpaceMatrix;
	float near_plane = 1.0f, far_plane = 7.5f;
	lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	lightView = glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
	lightSpaceMatrix = lightProjection * lightView;
	// render scene from light's point of view
	UseShader(this->depthmapShader);
	glUniformMatrix4fv(glGetUniformLocation(this->depthmapShader, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
	glViewport(0, 0, this->SHADOW_WIDTH, this->SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	DrawColoredTiang(this->depthmapShader);
	DrawColoredAtap(this->depthmapShader);
	DrawColoredPlane(this->depthmapShader);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);



	// Step 2 Render scene normally using generated depth map
	// ------------------------------------------------------
	glViewport(0, 0, this->screenWidth, this->screenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Pass perspective projection matrix
	UseShader(this->shadowmapShader);
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)this->screenWidth / (GLfloat)this->screenHeight, 0.1f, 100.0f);
	glUniformMatrix4fv(glGetUniformLocation(this->shadowmapShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//// LookAt camera (position, target/direction, up)
	//glm::vec3 cameraPos = glm::vec3(0, 5, 2);
	//glm::vec3 cameraFront = glm::vec3(0, 0, 0);
	//glm::mat4 view = glm::lookAt(cameraPos, cameraFront, glm::vec3(0, 1, 0));
	//glUniformMatrix4fv(glGetUniformLocation(this->shadowmapShader, "view"), 1, GL_FALSE, glm::value_ptr(view));

	// LookAt camera (position, target/direction, up)
	glm::mat4 view = glm::lookAt(glm::vec3(posCamX, posCamY, posCamZ), glm::vec3(viewCamX, viewCamY, viewCamZ), glm::vec3(upCamX, upCamY, upCamZ));
	GLint viewLoc = glGetUniformLocation(this->shadowmapShader, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	// Setting Light Attributes
	glUniformMatrix4fv(glGetUniformLocation(this->shadowmapShader, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "viewPos"), posCamX, posCamY, posCamZ);
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "lightPos"), -8.0f, 4.0f, -8.0f);

	// Configure Shaders
	glUniform1i(glGetUniformLocation(this->shadowmapShader, "diffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(this->shadowmapShader, "shadowMap"), 1);

	// Render floor
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, plane_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawColoredPlane(this->shadowmapShader);
// Render cube
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cube_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawColoredTiang(this->shadowmapShader);
	// Render cube
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cube_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawColoredAtap (this->shadowmapShader);

	
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
}



//void Demo::BuildTexturedCube()
//{
//	// load image into texture memory
//	// ------------------------------
//	// Load and create a texture 
//	glGenTextures(1, &cube_texture);
//	glBindTexture(GL_TEXTURE_2D, cube_texture);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	int width, height;
//	unsigned char* image = SOIL_load_image("crate.png", &width, &height, 0, SOIL_LOAD_RGBA);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
//	SOIL_free_image_data(image);
//	glBindTexture(GL_TEXTURE_2D, 0);
//
//
//	// set up vertex data (and buffer(s)) and configure vertex attributes
//	// ------------------------------------------------------------------
//	float vertices[] = {
//		// format position, tex coords, normal
//		// front
//		-1.0, -1.0, 1.0, 0, 0, 0.0f,  0.0f,  1.0f, // 0
//		1.0, -1.0, 1.0, 1, 0,  0.0f,  0.0f,  1.0f, // 1
//		1.0,  1.0, 1.0, 1, 1,  0.0f,  0.0f,  1.0f, // 2
//		-1.0,  1.0, 1.0, 0, 1, 0.0f,  0.0f,  1.0f, // 3
//
//		 // right
//		 1.0,  1.0,  1.0, 0, 0, 1.0f,  0.0f,  0.0f, // 4
//		 1.0,  1.0, -1.0, 1, 0, 1.0f,  0.0f,  0.0f, // 5
//		 1.0, -1.0, -1.0, 1, 1, 1.0f,  0.0f,  0.0f, // 6
//		 1.0, -1.0,  1.0, 0, 1, 1.0f,  0.0f,  0.0f, // 7
//
//		// back
//		-1.0, -1.0, -1.0, 0, 0, 0.0f,  0.0f,  -1.0f, // 8 
//		1.0,  -1.0, -1.0, 1, 0, 0.0f,  0.0f,  -1.0f, // 9
//		1.0,   1.0, -1.0, 1, 1, 0.0f,  0.0f,  -1.0f, // 10
//		-1.0,  1.0, -1.0, 0, 1, 0.0f,  0.0f,  -1.0f, // 11
//
//		 // left
//		 -1.0, -1.0, -1.0, 0, 0, -1.0f,  0.0f,  0.0f, // 12
//		 -1.0, -1.0,  1.0, 1, 0, -1.0f,  0.0f,  0.0f, // 13
//		 -1.0,  1.0,  1.0, 1, 1, -1.0f,  0.0f,  0.0f, // 14
//		 -1.0,  1.0, -1.0, 0, 1, -1.0f,  0.0f,  0.0f, // 15
//
//		// upper
//		1.0, 1.0,  1.0, 0, 0,   0.0f,  1.0f,  0.0f, // 16
//		-1.0, 1.0, 1.0, 1, 0,   0.0f,  1.0f,  0.0f, // 17
//		-1.0, 1.0, -1.0, 1, 1,  0.0f,  1.0f,  0.0f, // 18
//		1.0, 1.0, -1.0, 0, 1,   0.0f,  1.0f,  0.0f, // 19
//
//		// bottom
//		-1.0, -1.0, -1.0, 0, 0, 0.0f,  -1.0f,  0.0f, // 20
//		1.0, -1.0, -1.0, 1, 0,  0.0f,  -1.0f,  0.0f, // 21
//		1.0, -1.0,  1.0, 1, 1,  0.0f,  -1.0f,  0.0f, // 22
//		-1.0, -1.0,  1.0, 0, 1, 0.0f,  -1.0f,  0.0f, // 23
//	};
//
//	unsigned int indices[] = {
//		0,  1,  2,  0,  2,  3,   // front
//		4,  5,  6,  4,  6,  7,   // right
//		8,  9,  10, 8,  10, 11,  // back
//		12, 14, 13, 12, 15, 14,  // left
//		16, 18, 17, 16, 19, 18,  // upper
//		20, 22, 21, 20, 23, 22   // bottom
//	};
//
//	glGenVertexArrays(1, &cubeVAO);
//	glGenBuffers(1, &cubeVBO);
//	glGenBuffers(1, &cubeEBO);
//	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
//	glBindVertexArray(cubeVAO);
//
//	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
//
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
//
//	// define position pointer layout 0
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(GLfloat)));
//	glEnableVertexAttribArray(0);
//
//	// define texcoord pointer layout 1
//	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
//	glEnableVertexAttribArray(1);
//
//	// define normal pointer layout 2
//	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
//	glEnableVertexAttribArray(2);
//
//	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//
//	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
//	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
//	glBindVertexArray(0);
//
//	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//}


void Demo::BuildColoredTiang() {
	// load image into texture memory
	// ------------------------------
	// Load and create a texture 
	glGenTextures(1, &cube_texture);
	glBindTexture(GL_TEXTURE_2D, cube_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height;
	unsigned char* image = SOIL_load_image("crate.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices1[] = {
		// format position, tex coords
		// TIANG 1
		//front
		-13.5, -0.5, 0, 0, 0,  // 0
		-12.5, -0.5, 0, 1, 0,   // 1
		-12.5,  4.0, 0, 1, 1,   // 2
		-13.5,  4.0, 0, 0, 1,  // 3
		// right
		-12.5,  4.0, 0, 0, 0,  // 4
		-12.5,  4.0, -25, 1, 0,  // 5
		-12.5, -0.5, -25, 1, 1,  // 6
		-12.5, -0.5,  0, 0, 1,  // 7
		// back
		-13.5, -0.5, -25, 0, 0, // 8 
		-12.5, -0.5, -25, 1, 0, // 9
		-12.5,  4.0, -25, 1, 1, // 10
		-13.5,  4.0, -25, 0, 1, // 11
		// left
		-13.5, -0.5, -25, 0, 0, // 12
		-13.5, -0.5, 0, 1, 0, // 13
		-13.5,  4.0, 0, 1, 1, // 14
		-13.5,  4.0, -25, 0, 1, // 15
		// upper
		-12.5, 4.0, 0, 0, 0,   // 16
		-13.5, 4.0, 0, 1, 0,  // 17
		-13.5, 4.0, -25, 1, 1,  // 18
		-12.5, 4.0, -25, 0, 1,   // 19
		// bottom
		-13.5, -0.5, -25, 0, 0, // 20
		-12.5, -0.5, -25, 1, 0,  // 21
		-12.5, -0.5, 0, 1, 1,  // 22
		-13.5, -0.5, 0, 0, 1, // 23
		//TIANG 2
		// front
		-12.5, -0.5, -25, 0, 0,  // 24 ,,kiba,
		12.5,  -0.5, -25, 1, 0,  // 25 ,,kaba,
		12.5,   4.0, -25, 1, 1,  // 26 ,,kaat,
		-12.5,  4.0, -25, 0, 1,  // 27 ,,kiat
		// right
		12.5,  4.0, -25, 0, 0,  // 28
		12.5,  4.0, -24, 1, 0,  // 29
		12.5, -0.5, -24, 1, 1,  // 30
		12.5, -0.5, -25, 0, 1,  // 31
		// back
		-12.5, -0.5, -24, 0, 0, // 32
		12.5,  -0.5, -24, 1, 0, // 33
		12.5,   4.0, -24, 1, 1, // 34
		-12.5,  4.0, -24, 0, 1, // 35
		// left
		-12.5, -0.5, -24, 0, 0, // 36
		-12.5, -0.5, -25, 1, 0, // 37
		-12.5,  4.0, -25, 1, 1, // 38
		-12.5,  4.0, -24, 0, 1, // 39
		// upper
		12.5,  4.0,  -25, 0, 0,   // 40
		-12.5, 4.0, -25, 1, 0,  // 41
		-12.5, 4.0, -24, 1, 1,  // 42
		12.5,  4.0, -24, 0, 1,   // 43
		// bottom
		-12.5, -0.5, -24, 0, 0, // 44
		12.5,  -0.5, -24, 1, 0,  // 45
		12.5,  -0.5, -25, 1, 1,  // 46
		-12.5, -0.5, -25, 0, 1, // 47
		//TIANG 3
		// front
		12.5, -0.5, 0, 0, 0,  // 48 ,,kiba,
		13.5, -0.5, 0, 1, 0,  // 49 ,,kaba,
		13.5,  4.0, 0, 1, 1,  // 50 ,,kaat,
		12.5,  4.0, 0, 0, 1,  // 51 ,,kiat
		// right
		13.5,  4.0,  0, 0, 0,  // 52
		13.5,  4.0, -25, 1, 0,  // 53
		13.5, -0.5, -25, 1, 1,  // 54
		13.5, -0.5,  0, 0, 1,  // 55
		// back
		12.5, -0.5, -25, 0, 0, // 56
		13.5, -0.5, -25, 1, 0, // 57
		13.5,  4.0, -25, 1, 1, // 58
		12.5,  4.0, -25, 0, 1, // 59
		// left
		12.5, -0.5, -25, 0, 0, // 60
		12.5, -0.5, 0, 1, 0, // 61
		12.5,  4.0,  0, 1, 1, // 62
		12.5,  4.0, -25, 0, 1, // 63
		// upper
		13.5, 4.0,  0, 0, 0,   // 64
		12.5, 4.0, 0, 1, 0,  // 65
		12.5, 4.0, -25, 1, 1,  // 66
		13.5, 4.0, -25, 0, 1,   // 67
		// bottom
		12.5, -0.5, -25, 0, 0, // 68
		13.5, -0.5, -25, 1, 0,  // 69
		13.5, -0.5,  0, 1, 1,  // 70
		12.5, -0.5,  0, 0, 1, // 71
		//TIANG 4
		// front
		-12.5, -0.5, -1, 0, 0,  // 72 ,,kiba,
		-1,    -0.5, -1, 1, 0,  // 73 ,,kaba,
		-1,     4.0, -1, 1, 1,  // 74 ,,kaat,
		-12.5,  4.0, -1, 0, 1,  // 75 ,,kiat
		// right
		-1,  4.0,  -1, 0, 0,  // 76
		-1,  4.0, 0, 1, 0,  // 77
		-1, -0.5, 0, 1, 1,  // 78
		-1, -0.5,  -1, 0, 1,  // 79
		// back
		-12.5, -0.5, 0, 0, 0, // 80
		-1,    -0.5, 0, 1, 0, // 81
		-1,     4.0, 0, 1, 1, // 82
		-12.5,  4.0, 0, 0, 1, // 83
		// left
		-12.5, -0.5, 0, 0, 0, // 84
		-12.5, -0.5,  -1, 1, 0, // 85
		-12.5,  4.0,  -1, 1, 1, // 86
		-12.5,  4.0, 0, 0, 1, // 87
		// upper
		-1,    4.0,  -1, 0, 0,   // 88
		-12.5, 4.0, -1, 1, 0,  // 89
		-12.5, 4.0, 0, 1, 1,  // 90
		-1,    4.0, 0, 0, 1,   // 91
		// bottom
		-12.5, -0.5, 0, 0, 0, // 92
		-1,    -0.5, 0, 1, 0,  // 93
		-1,    -0.5, -1, 1, 1,  // 94
		-12.5, -0.5, -1, 0, 1, // 95
		//TIANG 5
		// front
		1,     -0.5, -1, 0, 0,  // 96 ,,kiba,
		12.5,  -0.5, -1, 1, 0,  // 97 ,,kaba,
		12.5,   4.0, -1, 1, 1,  // 98 ,,kaat,
		1,      4.0, -1, 0, 1,  // 99 ,,kiat
		// right
		12.5,  4.0, -1, 0, 0,  // 100
		12.5,  4.0, 0, 1, 0,  // 101
		12.5, -0.5, 0, 1, 1,  // 102
		12.5, -0.5, -1, 0, 1,  // 103
		// back
		1,    -0.5, 0, 0, 0, // 104
		12.5, -0.5, 0, 1, 0, // 105
		12.5,  4.0, 0, 1, 1, // 106
		1,     4.0, 0, 0, 1, // 107
		// left
		1, -0.5, 0, 0, 0, // 108
		1, -0.5, -1, 1, 0, // 109
		1,  4.0, -1, 1, 1, // 110
		1,  4.0, 0, 0, 1, // 111
		// upper
		12.5, 4.0, -1, 0, 0,   // 112
		1,    4.0, -1, 1, 0,  // 113
		1,    4.0, 0, 1, 1,  // 114
		12.5, 4.0, 0, 0, 1,   // 115
		// bottom
		1,    -0.5, 0, 0, 0, // 116
		12.5, -0.5, 0, 1, 0,  // 117
		12.5, -0.5, -1, 1, 1,  // 118
		1,    -0.5, -1, 0, 1, // 119


	};

	unsigned int indices1[] = {
		//tiang 1
		0, 1, 2, 0, 2, 3,   // front
		4, 5, 6, 4, 6, 7,   // right
		8, 9, 10, 8, 10, 11,  // back
		12, 14, 13, 12, 15, 14,  // left
		16, 18, 17, 16, 19, 18,  // upper
		20, 22, 21, 20, 23, 22,  // bottom
		//tiang 2
		24, 25, 26, 24, 26, 27,   // front
		28, 29, 30, 28, 30, 31,   // right
		32, 33, 34, 32, 34, 35,  // back
		36, 38, 37, 36, 39, 38,  // left
		40, 42, 41, 40, 43, 42,  // upper
		44, 46, 45, 44, 47, 46,   // bottom
		//tiang 3
		48, 49, 50, 48, 50, 51,   // front
		52, 53, 54, 52, 54, 55,   // right
		56, 57, 58, 56, 58, 59,  // back
		60, 62, 61, 60, 63, 62,  // left
		64, 66, 65, 64, 67, 66,  // upper
		68, 70, 69, 68, 71, 70,   // bottom
		//tiang 4
		72, 73, 74, 72, 74, 75,   // front
		76, 77, 78, 76, 78, 79,   // right
		80, 81, 82, 80, 82, 83,  // back
		84, 86, 85, 84, 87, 86,  // left
		88, 90, 89, 88, 91, 90,  // upper
		92, 94, 93, 92, 95, 94,   // bottom
		//tiang 5
		96, 97, 98, 96, 98, 99,   // front
		100, 101, 102, 100, 102, 103,   // right
		104, 105, 106, 104, 106, 107,  // back
		108, 110, 109, 108, 111, 110,  // left
		112, 114, 113, 112, 115, 114,  // upper
		116, 118, 117, 116, 119, 118,   // bottom

	};

	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glGenBuffers(1, &cubeEBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(cubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices1), indices1, GL_STATIC_DRAW);

	// define position pointer layout 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);

	// define texcoord pointer layout 1
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void Demo::DrawColoredTiang(GLuint shader)
{
	glUseProgram(shadowmapShader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cube_texture);
	glUniform1i(glGetUniformLocation(this->shadowmapShader, "ourTexture"), 0);

	glBindVertexArray(cubeVAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

	glDrawElements(GL_TRIANGLES, 180, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::BuildColoredAtap() {
	// load image into texture memory
	// ------------------------------
	// Load and create a texture 
	glGenTextures(1, &cube_texture);
	glBindTexture(GL_TEXTURE_2D, cube_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height;
	unsigned char* image = SOIL_load_image("download (6).jpg", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices2[] = {
		// format position, tex coords
		//TIANG ATAP -1
		// front
		1, -0.5, -18, 0, 0,   // 0
		2, -0.5, -18, 1, 0,  // 1
		2,    4, -18, 1, 1,  // 2
		1,    4, -18, 0, 1,   // 3
		// bottom
		2, -0.5, -24, 0, 0, // 4
		1, -0.5, -24, 1, 0,  // 5
		1, -0.5, -18, 1, 1,  // 6
		2, -0.5, -18, 0, 1, // 77
		// right
		2, -0.5, -24, 0, 0,  // 8
		2, -0.5, -18, 1, 0,  // 9
		2,    4, -18, 1, 1, // 10
		2,    4, -24, 0, 1, // 11
		// left
		1, -0.5, -24, 0, 0, // 12
		1, -0.5, -18, 1, 0, // 13
		1,    4, -18, 1, 1, // 14
		1,    4, -24, 0, 1, // 15
		//TIANG ATAP -2
		// front
		1,     -0.5, -15, 0, 0,   // 16
		12.5,  -0.5, -15, 1, 0,  // 17
		12.5,     4, -15, 1, 1,  // 18
		1,        4, -15, 0, 1,   // 19
		// bottom
		12.5,  -0.5, -16, 0, 0, // 20
		1,     -0.5, -16, 1, 0,  // 21
		1,     -0.5, -15, 1, 1,  // 22
		12.5,  -0.5, -15, 0, 1, // 23
		// back
		1,     -0.5, -16, 0, 0,  // 24
		12.5,  -0.5, -16, 1, 0,  // 25
		12.5,     4, -16, 1, 1, // 26
		1,        4, -16, 0, 1, // 27
		// left
		1, -0.5, -16, 0, 0, // 28
		1, -0.5, -15, 1, 0, // 29
		1,    4, -15, 1, 1, // 30
		1,    4, -16, 0, 1, // 31
		//TIANG ATAP -3
		// front
		-12.5, -0.5,  -15, 0, 0,   // 32
		-1,    -0.5,  -15, 1, 0,  // 33
		-1,       4,  -15, 1, 1,  // 34
		-12.5,    4,  -15, 0, 1,   // 35
		// bottom
		-1,    -0.5, -16, 0, 0, // 36
		-12.5, -0.5, -16, 1, 0,  // 37
		-12.5, -0.5,  -15, 1, 1,  // 38
		-1,    -0.5,  -15, 0, 1, // 39
		// right
		-1, -0.5, -16, 0, 0,  // 40
		-1, -0.5, -15, 1, 0,  // 41
		-1,    4, -15, 1, 1, // 42
		-1,    4,-16, 0, 1, // 43
		// back
		-12.5, -0.5, -16, 0, 0, // 44
		-1,    -0.5, -16, 1, 0, // 45
		-1,       4, -16, 1, 1, // 46
		-12.5,    4, -16, 0, 1, // 47


		//ATAP
		// bottom
		13.5,  4, -25, 0, 0, // 48
		-13.5, 4, -25, 1, 0,  //49
		-13.5, 4,  0, 1, 1,  // 50
		13.5,  4,  0, 0, 1, // 51
		// right
		13.5,   4, -25, 0, 0,  // 52
		13.5,   4, 0, 1, 0,  // 53
		0,      8, 0, 1, 1, // 54
		0,      8, -25, 0, 1, // 55
		// left
		-13.5,  4, -25, 0, 0, // 56
		-13.5,  4, 0, 1, 0, // 57
		0,      8, 0, 1, 1, // 58
		0,      8, -25, 0, 1, // 59
		//front		
		0,       8, 0, 0, 0, //60
		-13.5,   4, 0, 1, 0, //61
		13.5,    4, 0, 0, 1, //62
		//back
		-13.5,  4, -6, 0, 0, //63
		13.5,   4, -6, 1, 0, //64
		0,    5.2, -6, 1, 1, //65


	};

	unsigned int indices2[] = {
		//atap tiang -1
		0, 2, 1, 0, 3, 2,  // upper
		4, 6, 5, 4, 7, 6,   // bottom
		8, 9, 10, 8, 11, 10, // front
		12, 13, 14, 12, 15, 14, // back
		//atap tiang -2
		16, 18, 17, 16, 19, 18,  // upper
		20, 22, 21, 20, 23, 22,   // bottom
		24, 25, 26, 24, 27, 26, // front
		28, 29, 30, 28, 31, 30, // back
		//atap tiang -3
		32, 34, 33, 32, 35, 34,  // upper
		36, 38, 37, 36, 39, 38,   // bottom
		40, 41, 42, 40, 42, 43, // front
		44, 45, 46, 44, 46, 47, // back


		//atap tiang -4
		48, 50, 49, 48, 51, 50,  // bot
		52, 54, 53, 52, 55, 54,   // rig
		56, 57, 58, 56, 58, 59, // left
		60, 61, 62, //fron
		63, 64, 65, //back


	};


	glGenVertexArrays(1, &atapVAO);
	glGenBuffers(1, &atapVBO);
	glGenBuffers(1, &atapEBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(atapVAO);

	glBindBuffer(GL_ARRAY_BUFFER, atapVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, atapEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices2), indices2, GL_STATIC_DRAW);

	// define position pointer layout 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);

	// define texcoord pointer layout 1
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void Demo::DrawColoredAtap(GLuint shader)
{
	glUseProgram(shadowmapShader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cube_texture);
	glUniform1i(glGetUniformLocation(this->shadowmapShader, "ourTexture"), 0);

	glBindVertexArray(atapVAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

	glDrawElements(GL_TRIANGLES, 504, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::BuildColoredPlane()
{
	// Load and create a texture 
	glGenTextures(1, &plane_texture);
	glBindTexture(GL_TEXTURE_2D, plane_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height;
	unsigned char* image = SOIL_load_image("download (2).jpg", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Build geometry
	GLfloat vertices[] = {
		// format position, tex coords
		// bottom
		-50.0, -0.5, -50.0,  0,  0,
		50.0, -0.5, -50.0, 50,  0,
		50.0, -0.5,  50.0, 50, 50,
		-50.0, -0.5,  50.0,  0, 50,
	};

	GLuint indices[] = { 0,  2,  1,  0,  3,  2 };

	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glGenBuffers(1, &planeEBO);

	glBindVertexArray(planeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	// TexCoord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0); // Unbind VAO
}

//void Demo::DrawTexturedCube(GLuint shader)
//{
//	UseShader(shader);
//	glBindVertexArray(cubeVAO);
//	glm::mat4 model;
//	model = glm::translate(model, glm::vec3(0, 0.5f, 0));
//
//	GLint modelLoc = glGetUniformLocation(shader, "model");
//	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
//
//	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
//
//	glBindTexture(GL_TEXTURE_2D, 0);
//	glBindVertexArray(0);
//}

void Demo::DrawColoredPlane(GLuint shader)
{
	UseShader(shader);
	glBindVertexArray(planeVAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

	glm::mat4 model;
	GLint modelLoc = glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::BuildDepthMap() {
	// configure depth map FBO
	// -----------------------
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, this->SHADOW_WIDTH, this->SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Demo::BuildShaders()
{
	// build and compile our shader program
	// ------------------------------------
	shadowmapShader = BuildShader("shadowMapping.vert", "shadowMapping.frag", nullptr);
	depthmapShader = BuildShader("depthMap.vert", "depthMap.frag", nullptr);
}
void Demo::InitCamera()
{
	posCamX = 0.0f;
	posCamY = 1.2f;
	posCamZ = 8.0f;
	viewCamX = 0.0f;
	viewCamY = 1.0f;
	viewCamZ = 0.0f;
	upCamX = 0.0f;
	upCamY = 1.0f;
	upCamZ = 0.0f;
	CAMERA_SPEED = 0.001f;
	fovy = 45.0f;
	glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Demo::MoveCamera(float speed)
{
	float x = viewCamX - posCamX;
	float z = viewCamZ - posCamZ;
	// forward positive cameraspeed and backward negative -cameraspeed.
	posCamX = posCamX + x * speed;
	posCamZ = posCamZ + z * speed;
	viewCamX = viewCamX + x * speed;
	viewCamZ = viewCamZ + z * speed;
}

void Demo::StrafeCamera(float speed)
{
	float x = viewCamX - posCamX;
	float z = viewCamZ - posCamZ;
	float orthoX = -z;
	float orthoZ = x;

	// left positive cameraspeed and right negative -cameraspeed.
	posCamX = posCamX + orthoX * speed;
	posCamZ = posCamZ + orthoZ * speed;
	viewCamX = viewCamX + orthoX * speed;
	viewCamZ = viewCamZ + orthoZ * speed;
}

void Demo::RotateCamera(float speed)
{
	float x = viewCamX - posCamX;
	float z = viewCamZ - posCamZ;
	viewCamZ = (float)(posCamZ + glm::sin(speed) * x + glm::cos(speed) * z);
	viewCamX = (float)(posCamX + glm::cos(speed) * x - glm::sin(speed) * z);
}

int main(int argc, char** argv) {
	RenderEngine& app = Demo();
	app.Start("World of Cubilce : Warehouse", 1240, 720, false, false);
}