#include "Demo.h"

Demo::Demo() {

}

Demo::~Demo() {
}

void Demo::Init() {
	// build and compile our shader program
	// ------------------------------------
	shaderProgram = BuildShader("vertexShader.vert", "fragmentShader.frag", nullptr);

	BuildColoredTiang();

	BuildColoredAtap();

	BuildColoredPlane();

	InitCamera();
}

void Demo::DeInit() {
	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void Demo::ProcessInput(GLFWwindow *window) {
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
	glViewport(0, 0, this->screenWidth, this->screenHeight);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_DEPTH_TEST);

	// Pass perspective projection matrix
	glm::mat4 projection = glm::perspective(fovy, (GLfloat)this->screenWidth / (GLfloat)this->screenHeight, 0.1f, 100.0f);
	GLint projLoc = glGetUniformLocation(this->shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// LookAt camera (position, target/direction, up)
	glm::mat4 view = glm::lookAt(glm::vec3(posCamX, posCamY, posCamZ), glm::vec3(viewCamX, viewCamY, viewCamZ), glm::vec3(upCamX, upCamY, upCamZ));
	GLint viewLoc = glGetUniformLocation(this->shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	// set lighting attributes
	GLint lightPosLoc = glGetUniformLocation(this->shaderProgram, "lightPos");
	glUniform3f(lightPosLoc, 0, 1, 0);
	GLint viewPosLoc = glGetUniformLocation(this->shaderProgram, "viewPos");
	glUniform3f(viewPosLoc, 0, 2, 3);
	GLint lightColorLoc = glGetUniformLocation(this->shaderProgram, "lightColor");
	glUniform3f(lightColorLoc, .4f, .4f, .4f);

	DrawColoredTiang();

	DrawColoredAtap();

	DrawColoredPlane();

	glDisable(GL_DEPTH_TEST);
}

void Demo::BuildColoredTiang() {
	// load image into texture memory
	// ------------------------------
	// Load and create a texture 
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
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

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
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

void Demo::DrawColoredTiang()
{
	glUseProgram(shaderProgram);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(glGetUniformLocation(this->shaderProgram, "ourTexture"), 0);

	glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

	glDrawElements(GL_TRIANGLES, 180, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}


void Demo::BuildColoredAtap() {
	// load image into texture memory
	// ------------------------------
	// Load and create a texture 
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
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
		

	
		/*
		//TIANG ATAP -5
		// upper
		-0.5, 5.0,  -6.0, 0, 0,   // 64
		0.5, 5.0, -6.0, 1, 0,  // 65
		0.5, 5.0, -7.5, 1, 1,  // 66
		-0.5, 5.0, -7.5, 0, 1,   // 67
		// bottom
		0.5, 4.0, -7.5, 0, 0, // 68
		-0.5, 4.0, -7.5, 1, 0,  // 69
		-0.5, 4.0,  -6.0, 1, 1,  // 70
		0.5, 4.0,  -6.0, 0, 1, // 71
		// front
		0.5, 4.0, -7.5, 0, 0,  // 72
		0.5, 4.0, -6.0, 1, 0,  // 73
		0.5,  5.0, -6.0, 1, 1, // 74
		0.5,  5.0,-7.5, 0, 1, // 75
		// back
		-0.5, 4.0, -7.5, 0, 0, // 76
		-0.5, 4.0, -6.0, 1, 0, // 77
		-0.5, 5.0, -6.0, 1, 1, // 78
		-0.5, 5.0, -7.5, 0, 1, // 79
		//TIANG ATAP -6
		// upper
		-0.5, 4.8,  -7.5, 0, 0,   // 80
		0.5, 4.8, -7.5, 1, 0,  // 81
		0.5, 4.8, -9.0, 1, 1,  // 82
		-0.5, 4.8, -9.0, 0, 1,   // 83
		// bottom
		0.5, 3.8, -9.0, 0, 0, // 84
		-0.5, 3.8, -9.0, 1, 0,  // 85
		-0.5, 3.8,  -7.5, 1, 1,  // 86
		0.5, 3.8,  -7.5, 0, 1, // 87
		// front
		0.5, 3.8, -9.0, 0, 0,  // 88
		0.5, 3.8, -7.5, 1, 0,  // 89
		0.5,  4.8, -7.5, 1, 1, // 90
		0.5,  4.8,-9.0, 0, 1, // 91
		// back
		-0.5, 3.8, -9.0, 0, 0, // 92
		-0.5, 3.8, -7.5, 1, 0, // 93
		-0.5, 4.8, -7.5, 1, 1, // 94
		-0.5, 4.8, -9.0, 0, 1, // 95
		//TIANG ATAP -7
		// upper
		-0.5, 4.8,  -9.0, 0, 0,   // 96
		0.5, 4.8, -9.0, 1, 0,  // 97
		0.5, 4.8, -20.0, 1, 1,  // 98
		-0.5, 4.8, -20.0, 0, 1,   // 99
		// bottom
		0.5, 3.8, -20.0, 0, 0, // 100
		-0.5, 3.8, -20.0, 1, 0,  // 101
		-0.5, 3.8,  -9.0, 1, 1,  // 102
		0.5, 3.8,  -9.0, 0, 1, // 103
		// front
		0.5, 3.8, -20.0, 0, 0,  // 104
		0.5, 3.8, -9.0, 1, 0,  // 105
		0.5,  4.8, -9.0, 1, 1, // 106
		0.5,  4.8,-20.0, 0, 1, // 107
		// back
		-0.5, 3.8, -20.0, 0, 0, // 108
		-0.5, 3.8, -9.0, 1, 0, // 109
		-0.5, 4.8, -9.0, 1, 1, // 110
		-0.5, 4.8, -20.0, 0, 1, // 111
		//TIANG ATAP 0
		// upper
		-0.5, 6.0,  0.0, 0, 0,   // 112 kiba
		0.5, 6.0, 0.0, 1, 0,  // 113 kaba
		0.5, 6.0, 1.5, 1, 1,  // 114 kaat
		-0.5, 6.0, 1.5, 0, 1,   // 115 kiat
		// bottom
		0.5, 5.0, 1.5, 0, 0, // 116
		-0.5, 5.0, 1.5, 1, 0,  // 117
		-0.5, 5.0,  0.0, 1, 1,  // 118
		0.5, 5.0,  0.0, 0, 1, // 119
		// front
		0.5, 5.0, 1.5, 0, 0,  // 120
		0.5, 5.0, 0.0, 1, 0,  // 121
		0.5,  6.0, 0.0, 1, 1, // 122
		0.5,  6.0, 1.5, 0, 1, // 123
		// back
		-0.5, 5.0, 1.5, 0, 0, // 124
		-0.5,  5.0, 0.0, 1, 0, // 125
		-0.5,   6.0, 0.0, 1, 1, // 126
		-0.5,  6.0, 1.5, 0, 1, // 127
		//TIANG ATAP 1
		// upper
		-0.5, 5.8,  1.5, 0, 0,   // 128 kiba
		0.5, 5.8, 1.5, 1, 0,  // 129 kaba
		0.5, 5.8, 3.0, 1, 1,  // 130 kaat
		-0.5, 5.8, 3.0, 0, 1,   // 131 kiat
		// bottom
		0.5, 4.8, 3.0, 0, 0, // 132
		-0.5, 4.8, 3.0, 1, 0,  // 133
		-0.5, 4.8,  1.5, 1, 1,  // 134
		0.5, 4.8,  1.5, 0, 1, // 135
		// front
		0.5, 4.8, 3.0, 0, 0,  // 136
		0.5, 4.8, 1.5, 1, 0,  // 137
		0.5, 5.8, 1.5, 1, 1, // 138
		0.5, 5.8, 3.0, 0, 1, // 139
		// back
		-0.5, 4.8, 3.0, 0, 0, // 140
		-0.5,  4.8, 1.5, 1, 0, // 141
		-0.5,   5.8, 1.5, 1, 1, // 142
		-0.5,  5.8, 3.0, 0, 1, // 143
		//TIANG ATAP 2
		// upper
		-0.5, 5.4,  3.0, 0, 0,   // 144
		0.5, 5.4, 3.0, 1, 0,  // 145
		0.5, 5.4, 4.5, 1, 1,  // 146
		-0.5, 5.4, 4.5, 0, 1,   // 147
		// bottom
		0.5, 4.4, 4.5, 0, 0, // 148
		-0.5, 4.4, 4.5, 1, 0,  // 149
		-0.5, 4.4, 3.0, 1, 1,  // 150
		0.5, 4.4,  3.0, 0, 1, // 151
		// front
		0.5, 4.4, 4.5, 0, 0,  // 152
		0.5, 4.4, 3.0, 1, 0,  // 153
		0.5,  5.4, 3.0, 1, 1, // 154
		0.5,  5.4,4.5, 0, 1, // 155
		// back
		-0.5, 4.4, 4.5, 0, 0, // 156
		-0.5, 4.4, 3.0, 1, 0, // 157
		-0.5, 5.4, 3.0, 1, 1, // 158
		-0.5, 5.4, 4.5, 0, 1, // 159
		//TIANG ATAP 3
		// upper
		-0.5, 5.2,  4.5, 0, 0,   // 160
		0.5, 5.2, 4.5, 1, 0,  // 161
		0.5, 5.2, 6.0, 1, 1,  // 162
		-0.5, 5.2, 6.0, 0, 1,   // 163
		// bottom
		0.5, 4.2, 6.0, 0, 0, // 164
		-0.5, 4.2, 6.0, 1, 0,  //165
		-0.5, 4.2,  4.5, 1, 1,  // 166
		0.5, 4.2,  4.5, 0, 1, // 167
		// front
		0.5, 4.2, 6.0, 0, 0,  // 168
		0.5, 4.2, 4.5, 1, 0,  // 169
		0.5,  5.2, 4.5, 1, 1, // 170
		0.5,  5.2,6.0, 0, 1, // 171
		// back
		-0.5, 4.2, 6.0, 0, 0, // 172
		-0.5, 4.2, 4.5, 1, 0, // 173
		-0.5, 5.2, 4.5, 1, 1, // 174
		-0.5, 5.2, 6.0, 0, 1, // 175
		//TIANG ATAP 4
		// upper
		-0.5, 5.0,  6.0, 0, 0,   // 176
		0.5, 5.0, 6.0, 1, 0,  // 177
		0.5, 5.0, 7.5, 1, 1,  // 178
		-0.5, 5.0, 7.5, 0, 1,   // 179
		// bottom
		0.5, 4.0, 7.5, 0, 0, // 180
		-0.5, 4.0, 7.5, 1, 0,  // 181
		-0.5, 4.0, 6.0, 1, 1,  // 182
		0.5, 4.0,  6.0, 0, 1, // 183
		// front
		0.5, 4.0, 7.5, 0, 0,  // 184
		0.5, 4.0, 6.0, 1, 0,  // 185
		0.5,  5.0, 6.0, 1, 1, // 186
		0.5,  5.0,7.5, 0, 1, // 187
		// back
		-0.5, 4.0, 7.5, 0, 0, // 188
		-0.5, 4.0, 6.0, 1, 0, // 189
		-0.5, 5.0, 6.0, 1, 1, // 190
		-0.5, 5.0, 7.5, 0, 1, // 191
		//TIANG ATAP 5
		// upper
		-0.5, 4.8,  7.5, 0, 0,   // 192
		0.5, 4.8, 7.5, 1, 0,  // 193
		0.5, 4.8, 9.0, 1, 1,  // 194
		-0.5, 4.8, 9.0, 0, 1,   // 195
		// bottom
		0.5, 3.8, 9.0, 0, 0, // 196
		-0.5, 3.8, 9.0, 1, 0,  // 197
		-0.5, 3.8, 7.5, 1, 1,  // 198
		0.5, 3.8,  7.5, 0, 1, // 199
		// front
		0.5, 3.8, 9.0, 0, 0,  // 200
		0.5, 3.8, 7.5, 1, 0,  // 201
		0.5,  4.8, 7.5, 1, 1, // 202
		0.5,  4.8,9.0, 0, 1, // 203
		// back
		-0.5, 3.8, 9.0, 0, 0, // 204
		-0.5, 3.8, 7.5, 1, 0, // 205
		-0.5, 4.8, 7.5, 1, 1, // 206
		-0.5, 4.8, 9.0, 0, 1, // 207
		//TIANG ATAP 6
		// upper
		-0.5, 4.8,  9.0, 0, 0,   // 208
		0.5, 4.8, 9.0, 1, 0,  // 209
		0.5, 4.8, 20.0, 1, 1,  // 210
		-0.5, 4.8, 20.0, 0, 1,   // 211
		// bottom
		0.5, 3.8, 20.0, 0, 0, // 212
		-0.5, 3.8, 20.0, 1, 0,  // 213
		-0.5, 3.8, 9.0, 1, 1,  // 214
		0.5, 3.8,  9.0, 0, 1, // 215
		// front
		0.5, 3.8, 20.0, 0, 0,  // 216
		0.5, 3.8, 9.0, 1, 0,  // 217
		0.5,  4.8, 9.0, 1, 1, // 218
		0.5,  4.8,20.0, 0, 1, // 219
		// back
		-0.5, 3.8, 20.0, 0, 0, // 220
		-0.5, 3.8, 9.0, 1, 0, // 221
		-0.5, 4.8, 9.0, 1, 1, // 222
		-0.5, 4.8, 20.0, 0, 1, // 223
		*/
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
		 
		
		/*
		//atap tiang -5
		64, 66, 65, 64, 67, 66,  // upper
		68, 70, 69, 68, 71, 70,   // bottom
		72, 73, 74, 72, 74, 75, // front
		76, 77, 78, 76, 78, 79, // back
		//atap tiang -6
		80, 82, 81, 80, 83, 82,  // upper
		84, 86, 85, 84, 87, 86,   // bottom
		88, 89, 90, 88, 90, 91, // front
		92, 93, 94, 92, 94, 95, // back
		//atap tiang -7
		96, 98, 97, 96, 99, 98,  // upper
		100, 102, 101, 100, 103, 102,   // bottom
		104, 105, 106, 104, 106, 107, // front
		108, 109, 110, 108, 110, 111, // back
		//atap tiang 0
		112, 114, 113, 112, 115, 114,  // upper
		116, 118, 117, 116, 119, 118,   // bottom
		120, 121, 122, 120, 122, 123, // front
		124, 125, 126, 124, 126, 127, // back
		//atap tiang 1
		128, 130, 129, 128, 131, 130,  // upper
		132, 134, 133, 132, 135, 134,   // bottom
		136, 137, 138, 136, 138, 139, // front
		140, 141, 142, 140, 142, 143, // back
		//atap tiang 2
		144, 146, 145, 144, 147, 146,  // upper
		148, 150, 149, 148, 151, 150,   // bottom
		152, 153, 154, 152, 154, 155, // front
		156, 157, 158, 156, 158, 159, // back
		//atap tiang 3
		160, 162, 161, 160, 163, 162,  // upper
		164, 166, 165, 164, 167, 166,   // bottom
		168, 169, 170, 168, 170, 171, // front
		172, 173, 174, 172, 174, 175, // back
		//atap tiang 4
		176, 178, 177, 176, 179, 178,  // upper
		180, 182, 181, 180, 183, 182,   // bottom
		184, 185, 186, 184, 186, 187, // front
		188, 189, 190, 188, 190, 191, // back
		//atap tiang 5
		192, 194, 193, 192, 195, 194,  // upper
		196, 198, 197, 196, 199, 198,   // bottom
		200, 201, 202, 200, 202, 203, // front
		204, 205, 206, 204, 206, 207, // back
		//atap tiang 6
		208, 210, 209, 208, 211, 210,  // upper
		212, 214, 213, 212, 215, 214,   // bottom
		216, 217, 218, 216, 218, 219, // front
		220, 221, 222, 220, 222, 223 // back
		*/
	};


	glGenVertexArrays(1, &VAO3);
	glGenBuffers(1, &VBO3);
	glGenBuffers(1, &EBO3);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO3);

	glBindBuffer(GL_ARRAY_BUFFER, VBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO3);
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

void Demo::DrawColoredAtap()
{
	glUseProgram(shaderProgram);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(glGetUniformLocation(this->shaderProgram, "ourTexture"), 0);

	glBindVertexArray(VAO3); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

	glDrawElements(GL_TRIANGLES, 504, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}



void Demo::BuildColoredPlane()
{
	// Load and create a texture 
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
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

	glGenVertexArrays(1, &VAO2);
	glGenBuffers(1, &VBO2);
	glGenBuffers(1, &EBO2);

	glBindVertexArray(VAO2);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	// TexCoord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0); // Unbind VAO
}

void Demo::DrawColoredPlane()
{
	glUseProgram(shaderProgram);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glUniform1i(glGetUniformLocation(this->shaderProgram, "ourTexture"), 1);

	glBindVertexArray(VAO2); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
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
	RenderEngine &app = Demo();
	app.Start("Camera: Free Camera Implementation", 800, 600, false, false);
}