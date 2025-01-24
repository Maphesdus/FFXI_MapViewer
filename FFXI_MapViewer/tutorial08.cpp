#define GLM_ENABLE_EXPERIMENTAL // Enable experimental features

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#pragma comment(lib,"opengl32.lib")
//#pragma comment(lib,"glfw3.lib")
//#pragma comment(lib,"glew32.lib")

#define GLFW_DLL

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw/glfw3.h>
GLFWwindow* window;

// Include GLM
#define GLM_FORCE_INLINE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.hpp"
#include "texture.hpp"
//#include "controls.hpp"

#include <iostream>
#include "OpenGLDriver.h"
#include "SceneNode.h"
#include "FFXIMesh.h"
#include "FFXILandscapeMesh.h"
#include "SceneNodeLandscape.h"
#include "FPS_Counter.h"
#include "SceneManager.h"
#include "Frustum.h"

CFrustum *pFrustum;
int screenWidth = 768, screenHeight = 576;
//int screenWidth = 1024, screenHeight = 768;
void winFocus(GLFWwindow* handle, int isFocus)
{
	if(isFocus==GL_TRUE) {
		glfwSetCursorPos(window, screenWidth /2, screenHeight /2);
		pFrustum->setFocus(true);
	}
	else
		pFrustum->setFocus(false);
}

int main( int argc, char** argv )
{
	

	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(screenWidth, screenHeight, "Tutorial 08 - Basic Shading", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		return -1;
	}
	glfwSetWindowPos(window, 600,100);
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetCursorPos(window, screenWidth /2, screenHeight /2);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	// Dark blue background
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LEQUAL); 

	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// Cull triangles which normal is not towards the camera
//	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
//	GLuint programID = LoadShaders( "StandardShading.vertexshader", "StandardShading.fragmentshader" );
	GLuint programID = LoadShaders( "simpleShader.vertexshader", "simpleShader.fragmentshader" );
	GLuint programIDcube = LoadShaders( "cubeShader.vertexshader", "cubeShader.fragmentshader" );
	GLuint programIDSky = LoadShaders("skyShader.vertexshader", "skyShader.fragmentshader");

//	GLuint tid = loadBMP_custom("checker.bmp");
	GLuint tid = loadBMP_custom("black.bmp");

	//create driver and sceneManager
	COpenGLDriver *pDriver = new COpenGLDriver;
	pDriver->setProgramID(programID, programIDcube, programIDSky);
	pDriver->createMatrixHandler();
	pFrustum = new CFrustum;
	pFrustum->setCamInternals(60.0f, 1.0f*screenWidth/ screenHeight, 1.0f, 1600.0f);

	CSceneManager *pSceneMgr = new CSceneManager;
	pSceneMgr->addDriver(pDriver);
	pSceneMgr->addFrustum(pFrustum);

	int mapNo = atoi(argc >= 2 ? argv[1]:"4000010");
	CFFXILandscapeMesh *pFFXLandscapeImesh = (CFFXILandscapeMesh*)pSceneMgr->loadMeshLandscape(argc >= 2 ? argv[1] : "4000010", tid);
	if(!pFFXLandscapeImesh)
		return 2;

	CSceneNodeLandscape *pnode = (CSceneNodeLandscape*)pSceneMgr->createSceneNodeLandscape(pFFXLandscapeImesh);
	pFFXLandscapeImesh->drop();

	pnode->setPosition(glm::vec3(0,0,0));
	pnode->setCurrentFrame(0);

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);

	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);

	pDriver->assignGLBufferID(vertexbuffer, uvbuffer, normalbuffer, colorbuffer);

	GLuint shaderTextureID  = glGetUniformLocation(programID, "myTextureSampler");
	pDriver->setShaderTextureID(shaderTextureID);

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
//	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	glFrontFace(GL_CCW);
//	glDisable(GL_BLEND);
	// Enable blending
//	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ZERO);
	char title[100];
	int lastFPS=0, count=0, isFocus=GL_TRUE;

	glfwSetWindowFocusCallback(window, winFocus);
	CFPS_Counter fpsCounter;
	do{
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		// Use our shader
		glUseProgram(programID);

		if (glfwGetKey( window, GLFW_KEY_N ) == GLFW_PRESS){
			if(glfwGetKey(window,GLFW_KEY_N) == GLFW_RELEASE) {
				pnode->nextMMB();
				std::cout << ((pnode->isMZB())? "MZB":"MMB") << pnode->getCurrentMMB() << std::endl;
			}
		}
		else if (glfwGetKey( window, GLFW_KEY_B ) == GLFW_PRESS){
			if(glfwGetKey(window,GLFW_KEY_B) == GLFW_RELEASE) {
				pnode->prevMMB();
				std::cout << ((pnode->isMZB())? "MZB":"MMB") << pnode->getCurrentMMB() << std::endl;
			}
		}
		else if (glfwGetKey( window, GLFW_KEY_J ) == GLFW_PRESS){
			if(glfwGetKey(window,GLFW_KEY_J) == GLFW_RELEASE) {
				pnode->setCurrentMMB(pnode->getCurrentMMB()+10);
				std::cout << ((pnode->isMZB())? "MZB":"MMB") << pnode->getCurrentMMB() << std::endl;
			}
		}
		else if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
			if (glfwGetKey(window, GLFW_KEY_V) == GLFW_RELEASE) {
				pnode->nextMMBModel();
				std::cout << ((pnode->isMZB()) ? "MZB" : "MMB") << pnode->getCurrentMMB() << " Model: " << (pnode->getMMBModelInfo()+1) << std::endl;
			}
		}
		else if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
			if (glfwGetKey(window, GLFW_KEY_X) == GLFW_RELEASE) {
				pnode->toggleMMBModelInclusive();
				std::cout << ((pnode->isMMBModelInclusive()) ? "Model Inclusive" : "Model Exclusive") << std::endl;
			}
		}
		else if (glfwGetKey( window, GLFW_KEY_M ) == GLFW_PRESS){
			if(glfwGetKey(window,GLFW_KEY_M) == GLFW_RELEASE) {
				pnode->toggleIsMZB();
				std::cout << "refresh mesh by: " << ((pnode->isMZB())? "MZB":"MMB") << std::endl;
			}
		}
		//else if (glfwGetKey( window, GLFW_KEY_F ) == GLFW_PRESS){
		//	if(glfwGetKey(window,GLFW_KEY_F) == GLFW_RELEASE) {
		//		pnode->setCurrentMMB(-1);
		//		std::cout << "All MMB " << std::endl;
		//	}
		//}
		else if (glfwGetKey( window, GLFW_KEY_O ) == GLFW_PRESS){
			if(glfwGetKey(window,GLFW_KEY_O) == GLFW_RELEASE) {
				pnode->toggleIsOctree();
				std::cout << "Octree " << ((pnode->isOctree())? "enable":"disable") << std::endl;
			}
		}
		else if (glfwGetKey( window, GLFW_KEY_T ) == GLFW_PRESS){
			if(glfwGetKey(window,GLFW_KEY_T) == GLFW_RELEASE) {
				pFFXLandscapeImesh->toggleMMBTransform();
				std::cout << "MMB Transform: " << ((pFFXLandscapeImesh->isMMBTransform())? "yes":"no") << std::endl;
			}
		}
		//else if (glfwGetKey( window, GLFW_KEY_X ) == GLFW_PRESS){
		//	if(glfwGetKey(window,GLFW_KEY_X) == GLFW_RELEASE) {
		//		pnode->wirteMeshBuffer();
		//		std::cout << "writeMeshInfo: " << pnode->getCurrentMMB() << std::endl;
		//	}
		//}
		else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE) {
				pDriver->toggleWireframe();
				std::cout << "Wireframe: " << ((pDriver->isWireframe()) ? "yes" : "no") << std::endl;
			}
		}
		else if (glfwGetKey( window, GLFW_KEY_C ) == GLFW_PRESS){
			if(glfwGetKey(window,GLFW_KEY_C) == GLFW_RELEASE) {
				pnode->toggleDrawCube();
				std::cout << "draw Cube: " << ((pnode->isDrawCube())? "enable":"disable") << std::endl;
			}
		}
		else if (glfwGetKey( window, GLFW_KEY_G ) == GLFW_PRESS){
			if(glfwGetKey(window,GLFW_KEY_G) == GLFW_RELEASE) {
				pFrustum->toggleCamera();
				std::cout << "Dual Camera: " << ((pFrustum->isUseDualCamera())? "enable":"disable") << std::endl;
			}
		}
		else if (glfwGetKey( window, GLFW_KEY_E ) == GLFW_PRESS){
			if(glfwGetKey(window,GLFW_KEY_E) == GLFW_RELEASE) {
				if( !pFrustum->isUseDualCamera() )
					std::cout << "Press G to use Dual Camera, Press C to view Frustum" << std::endl;
				else {
					pFrustum->toggleEye();
					std::cout << ((pFrustum->isMainEye())? "Eye ":"Camera ") << "active, press arrow key to move" << std::endl;
				}
			}
		}
		else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE) {
				pnode->toggleDrawPVS();
			}
		}
		else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE) {
				if (!pnode->isDrawPVS()) {
					std::cout << "press S to enable Potential visible set" << std::endl;
				}
				else {
					pnode->prevPVS();
					std::cout << "draw Potential visible set: " << pnode->getCurrentPVS()+1 << std::endl;
				}
			}
		}
		else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
			if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
				if (!pnode->isDrawPVS()) {
					std::cout << "press S to enable Potential visible set" << std::endl;
				}
				else {
					pnode->nextPVS();
					std::cout << "draw Potential visible set: " << pnode->getCurrentPVS()+1 << std::endl;
				}
			}
		}
		else if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
			if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE) {
				pnode->toggleDrawNormal();
			}
		}
		else if(glfwGetKey( window, GLFW_KEY_KP_ADD ) == GLFW_PRESS){
			if(glfwGetKey(window,GLFW_KEY_KP_ADD) == GLFW_RELEASE) {
				pFrustum->setSpeedUp();
				std::cout << "Speed: " << pFrustum->getSpeed() << std::endl;
			}
		}
		else if(glfwGetKey( window, GLFW_KEY_KP_SUBTRACT ) == GLFW_PRESS){
			if(glfwGetKey(window,GLFW_KEY_KP_SUBTRACT) == GLFW_RELEASE) {
				pFrustum->setSpeedDown();
				std::cout << "Speed: " << pFrustum->getSpeed() << std::endl;
			}
		}
		else if(glfwGetKey( window, GLFW_KEY_PAGE_UP ) == GLFW_PRESS){
			if(glfwGetKey(window,GLFW_KEY_PAGE_UP) == GLFW_RELEASE) {
				pSceneMgr->resetScene();
				mapNo++;
				sprintf_s(title,20,"%d", mapNo);
				pFFXLandscapeImesh = (CFFXILandscapeMesh*)pSceneMgr->loadMeshLandscape(title, tid);
				if(!pFFXLandscapeImesh) {				
					std::cout << "Unable to load mesh " << mapNo << std::endl;
				}
				else {
					pnode = (CSceneNodeLandscape*)pSceneMgr->createSceneNodeLandscape(pFFXLandscapeImesh);
					pFFXLandscapeImesh->drop();
					pnode->setPosition(glm::vec3(0,0,0));
					pnode->setCurrentFrame(0);
					//pFrustum->setFrustumPos(pnode->getExtend());
					pFrustum->setFrustumPos(0.0);
					std::cout << "Loaded landscapeMesh: " << mapNo << std::endl;
				}
			}
		}
		else if(glfwGetKey( window, GLFW_KEY_PAGE_DOWN ) == GLFW_PRESS){
			if(glfwGetKey(window,GLFW_KEY_PAGE_DOWN) == GLFW_RELEASE) {
				pSceneMgr->resetScene();
				mapNo--;
				sprintf_s(title,20,"%d", mapNo);
				pFFXLandscapeImesh = (CFFXILandscapeMesh*)pSceneMgr->loadMeshLandscape(title, tid);
				if(!pFFXLandscapeImesh) {				
					std::cout << "Unable to load mesh " << mapNo << std::endl;
				}
				else {
					pnode = (CSceneNodeLandscape*)pSceneMgr->createSceneNodeLandscape(pFFXLandscapeImesh);
					pFFXLandscapeImesh->drop();
					pnode->setPosition(glm::vec3(0,0,0));
					pnode->setCurrentFrame(0);
					//pFrustum->setFrustumPos(pnode->getExtend());
					pFrustum->setFrustumPos(0.0);
					std::cout << "Loaded landscapeMesh: " << mapNo << std::endl;
				}
			}
		}
		// Compute the MVP matrix from keyboard and mouse input
		pFrustum->computeMatricesFromInputs(window, screenWidth, screenHeight);
		
		glm::mat4 ProjectionMatrix = pFrustum->getProjectionMatrix();
		glm::mat4 ViewMatrix = pFrustum->getViewMatrix();

//		glm::vec3 lightPos = glm::vec3(4,4,4);
//		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		pSceneMgr->drawAll(ProjectionMatrix, ViewMatrix);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		fpsCounter.registerFrame(pSceneMgr->getTime());
		count=fpsCounter.getFPS();
		if(lastFPS!=count) {
			sprintf_s(title,100,"FPS: %d", count);
			glfwSetWindowTitle(window, title);
			lastFPS=count;
		}
		
	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	pDriver->cleanUp();
	glDeleteVertexArrays(1, &VertexArrayID);
	delete pDriver;
	delete pSceneMgr;
	delete pFrustum;

	// Close OpenGL window and terminate GLFW
	glfwTerminate();


	return 0;
}


