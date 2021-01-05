#ifdef _WIN32
#define __USE_MINGW_ANSI_STDIO 0
#endif

// Std. Includes
#include <string>

// Loader for OpenGL extensions
// http://glad.dav1d.de/
// THIS IS OPTIONAL AND NOT REQUIRED, ONLY USE THIS IF YOU DON'T WANT GLAD TO INCLUDE windows.h
// GLAD will include windows.h for APIENTRY if it was not previously defined.
// Make sure you have the correct definition for APIENTRY for platforms which define _WIN32 but don't use __stdcall
#ifdef _WIN32
#define APIENTRY __stdcall
#endif

#include <std_include.h>

// another check related to OpenGL loader
// confirm that GLAD didn't include windows.h
#ifdef _WINDOWS_
#error windows.h was included!
#endif

// classes developed during lab lectures to manage shaders, to load models, and for FPS camera
// in this example, the Model and Mesh classes support texturing
#include <utils/shader_v1.h>
#include <utils/include_shader.h>
#include <utils/model_v2.h>
#include <utils/camera.h>

#include <irradiancegrid/Grid.hpp>
#include <ViewUniform.hpp>

#include "DebuggingSphere.hpp"
#include "TextRenderer.hpp"
#include "SceneObject.hpp"
#include "Surface.hpp"
#include "Ray.hpp"
#include "RadianceSphere.hpp"
#include "HDRBuffer.hpp"
#include "UnitHemisphereDirections.h"
#include "TrilinearSphere.hpp"

#include <RadianceSampler.hpp>
#include <objects/Cube.hpp>
#include <utils/include_shader.h>
#include <objects/Bunny.hpp>

// #define DEBUGSHADER


// dimensions of application's window
const GLuint ScreenWidth = 800;
const GLuint ScreenHeight = 600;

struct ApplicationFlags {
	bool Wireframe;
	bool PrintText;

	ApplicationFlags() : Wireframe(false), PrintText(true) {

	}
};


/* Forward declarations */
void Initialize();
// callback functions for keyboard and mouse events
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xpos, double ypos);
// if one of the WASD keys is pressed, we call the corresponding method of the Camera class
void apply_camera_movements(GLfloat deltaTime);
void Update(GLfloat deltaTime);
void Draw();
void RenderDebugInfo(GLfloat updateTime, GLfloat drawTime, int fps);

/* Variables sections */
// we initialize an array of booleans for each keybord key
bool keys[1024];
// we need to store the previous mouse position to calculate the offset with the current frame
GLfloat lastX, lastY;
// when rendering the first frame, we do not have a "previous state" for the mouse, so we need to manage this situation
bool firstMouse = true;
bool _spinning = false;

/* Optional HDR management */
HDRBuffer* _hdrBuffer;
Camera* _camera = nullptr;
TextRenderer* _debugWriter = nullptr;
CCube* _sceneCube = nullptr;
ApplicationFlags _appFlags;
RadianceSampler* _radianceSampler = nullptr;
Grid* _irradianceGrid = nullptr;
RadianceSphere* _radianceSphere = nullptr;
vector<SceneObject*> _sceneObjects;
TrilinearSphere* _trilinearSphere = nullptr;
TrilinearSphere* _secondTrilinear = nullptr;
Bunny* _bunny = nullptr;

int main()
{
	/* initialize random seed: */
	srand(time(NULL));

	Initialize();

	// we create the application's window
	GLFWwindow* window = glfwCreateWindow(ScreenWidth, ScreenHeight, "IrradianceVolumes", nullptr, nullptr);
	if (!window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	// we put in relation the window and the callbacks
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);

	// GLAD tries to load the context set by GLFW
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize OpenGL context" << std::endl;
		return -1;
	}

	// we define the viewport dimensions
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	// we enable Z test
	glEnable(GL_DEPTH_TEST);
	//the "clear" color for the frame buffer
	glClearColor(0.26f, 0.46f, 0.98f, 1.0f);
	glEnable(GL_LINE_SMOOTH);

	/* Initialization */
	// we create a camera. We pass the initial position as a paramenter to the constructor. The last boolean tells that we want a camera "anchored" to the ground
	_camera = new Camera(glm::vec3(0.0f, 0.0f, 7.0f), GL_TRUE);
	_debugWriter = new TextRenderer();
	ViewUniform viewSharedBuffer;
	// Projection matrix: FOV angle, aspect ratio, near and far planes
	viewSharedBuffer.SetProjection(glm::perspective(45.0f, (float)ScreenWidth / (float)ScreenHeight, 0.1f, 10000.0f));
	_sceneCube = new CCube();

	_radianceSampler = new RadianceSampler();
	_radianceSampler->GetSamplingObjects().push_back(_sceneCube);

	_irradianceGrid = new Grid(_sceneCube->GetBoundingCube());
	_irradianceGrid->SetTransform(_sceneCube->GetTransform());
	_radianceSphere = new RadianceSphere();

	_secondTrilinear = new TrilinearSphere();
	_secondTrilinear->SetPosition(glm::vec3(1.75f));

	_trilinearSphere = new TrilinearSphere();
	_trilinearSphere->SetPosition(glm::vec3(-0.75f));
	_bunny = new Bunny();

	_sceneObjects.push_back(_bunny);
	//_sceneObjects.push_back(_trilinearSphere);
	_sceneObjects.push_back(_secondTrilinear);

#ifdef DEBUGSHADER
	VariableShaderBuffer<int> debugBuffer(4);
	debugBuffer.SetVectorLength(100);
	debugBuffer.Write();
#endif

	DebuggingShere sphere;

	GLfloat lastTime = glfwGetTime();
	GLfloat lastFrame = 0;
	int frames = 0;
	int fps = 0;
	// Rendering loop: this code is executed at each frame
	while (!glfwWindowShouldClose(window))
	{
		// we determine the time passed from the beginning
		// and we calculate time difference between current frame rendering and the previous one
		GLfloat currentFrame = glfwGetTime();
		GLfloat deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Let's do our fps calculation
		frames++;
		if (currentFrame - lastTime >= 1.0) {
			fps = frames;
			frames = 0;
			lastTime += 1.0;
		}

		// Check is an I/O event is happening
		glfwPollEvents();
		// we apply FPS camera movements
		apply_camera_movements(deltaTime);
		viewSharedBuffer.SetCamera(_camera->GetViewMatrix());

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (_appFlags.Wireframe)
			// Draw in wireframe
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


		GLfloat beforeUpdate = glfwGetTime();
		Update(deltaTime);
		GLfloat afterUpdate = glfwGetTime();

		GLfloat beforeDraw = glfwGetTime();
		//sphere.Draw(glm::vec3(0.0f), 0.2f);
		Draw();
		GLfloat afterDraw = glfwGetTime();

#ifdef DEBUGSHADER
#else
		RenderDebugInfo(afterUpdate - beforeUpdate, afterDraw - beforeDraw, fps);
#endif

		glfwSwapBuffers(window);
	}

	_sceneObjects.clear();
	_radianceSampler->GetSamplingObjects().clear();
	/* Cleanup */
	delete _bunny;
	delete _secondTrilinear;
	delete _trilinearSphere;
	delete _radianceSphere;
	delete _irradianceGrid;
	delete _radianceSampler;
	delete _sceneCube;
	delete _debugWriter;
	delete _camera;

	// Let's cleanup all the shaders inclusion
	Include::Instance.Clear();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

void Initialize() {
	// Initialization of OpenGL context using GLFW
	glfwInit();
	// We set OpenGL specifications required for this application
	// In this case: 4.3 Core
	// If not supported by your graphics HW, the context will not be created and the application will close
	// N.B.) creating GLAD code to load extensions, try to take into account the specifications and any extensions you want to use,
	// in relation also to the values indicated in these GLFW commands
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	// we set if the window is resizable
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
}

/// <summary>
/// Callback for keyboard events
/// </summary>
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	// if ESC is pressed, we close the application
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	// if L is pressed, we activate/deactivate wireframe rendering of models
	if (key == GLFW_KEY_L && action == GLFW_PRESS)
		_appFlags.Wireframe = !_appFlags.Wireframe;
	if (key == GLFW_KEY_T && action == GLFW_PRESS)
		_appFlags.PrintText = !_appFlags.PrintText;

	// we keep trace of the pressed keys
	// with this method, we can manage 2 keys pressed at the same time:
	// many I/O managers often consider only 1 key pressed at the time (the first pressed, until it is released)
	// using a boolean array, we can then check and manage all the keys pressed at the same time
	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
}

// If one of the WASD keys is pressed, the camera is moved accordingly (the code is in utils/camera.h)
void apply_camera_movements(GLfloat deltaTime)
{
	if (keys[GLFW_KEY_W])
		_camera->ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		_camera->ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		_camera->ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		_camera->ProcessKeyboard(RIGHT, deltaTime);
	if (keys[GLFW_KEY_SPACE])
		_camera->ProcessKeyboard(SPACE, deltaTime);
}

/// <summary>
/// Callback for mouse events
/// </summary>
void MouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	// we move the camera view following the mouse cursor
	// we calculate the offset of the mouse cursor from the position in the last frame
	// when rendering the first frame, we do not have a "previous state" for the mouse, so we set the previous state equal to the initial values (thus, the offset will be = 0)
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	// offset of mouse cursor position
	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;

	// the new position will be the previous one for the next frame
	lastX = xpos;
	lastY = ypos;

	// we pass the offset to the Camera class instance in order to update the rendering
	_camera->ProcessMouseMovement(xoffset, yoffset);
}

void ApplyForTrilinearSphereMovement(GLfloat deltaTime) {
	SceneObject* object = _bunny;

	float change = (3.0 * deltaTime);
	if (keys[GLFW_KEY_LEFT]) {
		glm::vec3 newPos = object->GetPosition();
		newPos.x -= change;
		object->SetPosition(newPos);
	}
	if (keys[GLFW_KEY_RIGHT]) {
		glm::vec3 newPos = object->GetPosition();
		newPos.x += change;
		object->SetPosition(newPos);
	}
	if (keys[GLFW_KEY_DOWN]) {
		glm::vec3 newPos = object->GetPosition();
		newPos.y -= change;
		object->SetPosition(newPos);
	}
	if (keys[GLFW_KEY_UP]) {
		glm::vec3 newPos = object->GetPosition();
		newPos.y += change;
		object->SetPosition(newPos);
	}
	if (keys[GLFW_KEY_PAGE_UP]) {
		glm::vec3 newPos = object->GetPosition();
		newPos.z -= change;
		object->SetPosition(newPos);
	}
	if (keys[GLFW_KEY_PAGE_DOWN]) {
		glm::vec3 newPos = object->GetPosition();
		newPos.z += change;
		object->SetPosition(newPos);
	}
}

void ApplyGridSettingsUpdates() {
	// Sub grid level handfling
	if (keys[GLFW_KEY_KP_ADD]) {
		_irradianceGrid->SetMaxSubGridLevel(_irradianceGrid->GetMaxSubGridLevel() + 1);
		keys[GLFW_KEY_KP_ADD] = false;
	}
	if (keys[GLFW_KEY_KP_SUBTRACT]) {
		_irradianceGrid->SetMaxSubGridLevel(_irradianceGrid->GetMaxSubGridLevel() - 1);
		keys[GLFW_KEY_KP_SUBTRACT] = false;
	}

	for (int i = GLFW_KEY_2; i <= GLFW_KEY_6; i++)
	{
		if (keys[i]) {
			_irradianceGrid->SetGridDivision(glm::ivec3(i - GLFW_KEY_0));
			keys[i] = false;
		}
	}

	if (keys[GLFW_KEY_P]) {
		_irradianceGrid->SetParallelUpdate(!_irradianceGrid->IsParallelUpdateEnabled());
		keys[GLFW_KEY_P] = false;
	}
	if (keys[GLFW_KEY_C]) {
		bool debugColor = !_irradianceGrid->IsDebugColorEnabled();
		_irradianceGrid->SetDebugColorEnabled(debugColor);
		_radianceSphere->SetDebugColor(debugColor);
		_trilinearSphere->SetDebugColor(debugColor);
		_secondTrilinear->SetDebugColor(debugColor);
		_bunny->SetDebugColor(debugColor);
		keys[GLFW_KEY_C] = false;
	}

	if (keys[GLFW_KEY_R]) {
		_radianceSampler->SetResolution(_radianceSampler->GetResolution() == 9 ? 29 : 9);
		keys[GLFW_KEY_R] = false;
	}

	if (keys[GLFW_KEY_K]) {
		_spinning = !_spinning;
		keys[GLFW_KEY_K] = false;
	}
}

void Update(GLfloat deltaTime)
{
	if (_spinning) {
		float orientationY = _bunny->GetYRotation();
		orientationY += (deltaTime * 20.0f);
		_bunny->SetYRotation(orientationY);
	}		
	
	// Object move and other things
	ApplyForTrilinearSphereMovement(deltaTime);
	ApplyGridSettingsUpdates();	

	// Irradiance update
	_irradianceGrid->Update(_sceneObjects.cbegin(), _sceneObjects.cend(), _radianceSampler);
}

void Draw()
{
	_sceneCube->Draw();
	_irradianceGrid->Draw(_radianceSphere);

	for (SceneObject* sceneObject : _sceneObjects) {
		sceneObject->Draw();
	}
}

void RenderDebugInfo(GLfloat updateTime, GLfloat drawTime, int fps)
{

	// ~ WARN: This function will spend most of its time creating strings ~
	// Disable or ENTIRELY REMOVE the function while analizing performance
	static const glm::vec3 textColor = glm::vec3(0.8);
	static const GLfloat scaling = 0.30;
	static const std::string  fpsStr = "FPS: ";

	if (!_appFlags.PrintText)
	{
		// Let's print the FPS just for reference
		_debugWriter->RenderText(fpsStr + std::to_string(fps), 5, 5, scaling, textColor);
		return;
	}

	// String cache
	static const std::string parallelEnabled = "Parallel update: Enabled";
	static const std::string parallelDisabled = "Parallel update: Disabled";
	static const std::string resolution = "Resolution: ";
	static const std::string gridMaxLevels = "Grid max levels: ";
	static const std::string gridResolution = " Grid resolution: ";
	static const std::string debugColorStr = "DebugColor Active ";

	if (_irradianceGrid->IsDebugColorEnabled()) {
		_debugWriter->RenderText(debugColorStr, 5, 75, scaling, textColor);
	}

	_debugWriter->RenderText(resolution + std::to_string(_radianceSampler->GetResolution()), 5, 63, scaling, textColor);
	_debugWriter->RenderText(_irradianceGrid->IsParallelUpdateEnabled() ? parallelEnabled : parallelDisabled, 5, 51, scaling, textColor);

	const string gridInfo =
		gridMaxLevels + std::to_string(_irradianceGrid->GetMaxSubGridLevel()) +
		gridResolution + std::to_string(_irradianceGrid->GetGridDivision().x);
	_debugWriter->RenderText(gridInfo, 5, 39, scaling, textColor);

	_debugWriter->RenderText(fpsStr + std::to_string(fps), 5, 27, scaling, textColor);

	std::string timesString = std::string("Update time: ") + std::to_string(updateTime) + " ms, Draw time: " + std::to_string(drawTime) + " ms";
	_debugWriter->RenderText(timesString, 5, 15, scaling, textColor);


}
