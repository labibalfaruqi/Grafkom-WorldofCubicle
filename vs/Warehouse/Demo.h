#pragma once
#include "RenderEngine.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <SOIL/SOIL.h>

class Demo :
	public RenderEngine
{
public:
	Demo();
	~Demo();
private:
	GLuint cameraView, cameraPos, cameraFront, depthmapShader, shadowmapShader, cubeVBO, cubeVAO, cubeEBO, cube_texture, planeVBO, planeVAO, planeEBO, plane_texture, atapVBO, atapVAO, atapEBO, atap_texture, stexture, stexture2, depthMapFBO, depthMap;
	float viewCamX, viewCamY, viewCamZ, upCamX, upCamY, upCamZ, posCamX, posCamY, posCamZ, CAMERA_SPEED, fovy;
	// glm::vec3 cameraView, cameraPos, cameraFront;
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	virtual void Init();
	virtual void DeInit();
	virtual void Update(double deltaTime);
	virtual void Render();
	virtual void ProcessInput(GLFWwindow *window);
	void BuildTexturedCube();
	void BuildTexturedPlane();
	void DrawTexturedCube(GLuint shader);
	void DrawTexturedPlane(GLuint shader);
	
	void BuildColoredTiang();
	void BuildColoredAtap();
	void BuildColoredPlane();
	void DrawColoredTiang(GLuint shader);
	void DrawColoredAtap(GLuint shader);
	void DrawColoredPlane(GLuint shader);

	void BuildDepthMap();
	void BuildShaders();
	void MoveCamera(float speed);
	void StrafeCamera(float speed);
	void RotateCamera(float speed);
	void InitCamera();
};

