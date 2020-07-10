#pragma once

#include <glm/vec2.hpp>

#include "Constants.h"
#include "Core.h"
#include "GameSettings.h"

#include "Controller.h"
#include "Core/Camera.h"
#include "Core/LayerStack.h"
#include "Core/Scene.h"
#include "Core/Rendering/FreeType.h"
#include "Core/Rendering/OpenGL.h"
#include "Core/Rendering/Shader.h"
#include "Core/Rendering/TextureManager.h"
#include "Core/UI/GUILayer.h"
#include "Core/World/Player.h"
#include "Core/World/PlayerController.h"


class Event;
class Layer;

class Outrospection {
	OpenGL opengl; // defined at the beginning so nothing gets initialized before this
	FreeType freetype;
	
public:
	inline static Outrospection& get()
	{
		return *instance;
	}
	
	Outrospection();

	void run();
    void onEvent(Event& e);

    static void pauseGame();
	static void unpauseGame();

	void pushLayer(Layer* layer);
	void pushOverlay(Layer* layer);

	void captureMouse(bool doCapture) const;

	glm::vec2 lastMousePos = glm::vec2(SCR_HEIGHT / 2.0f, SCR_WIDTH / 2.0f);
	
	Scene scene;
	Player player;
	PlayerController playerController;
	TextureManager textureManager;
	GameSettings gameSettings;
	Controller controller{};

	std::unordered_map<char, FontCharacter> fontCharacters;

	Shader objectShader;
	Shader billboardShader;
	Shader skyShader;
	Shader screenShader;
	Shader simpleShader;
	Shader spriteShader;
	Shader glyphShader;

	DISALLOW_COPY_AND_ASSIGN(Outrospection)
private:
	void runGameLoop();
	void runTick();
	void updateCamera();

	std::unique_ptr<GUILayer> ingameGUI;
	std::unique_ptr<GUILayer> pauseGUI;

	// set to false when the game loop shouldn't run
	bool running = false;

	// timing
	float deltaTime = 0.0f;	// Time between current frame and last frame
	double lastFrame = 0.0f; // Time of last frame

	GLFWwindow* gameWindow;

	GLuint framebuffer, intermediateFBO = 0;
	GLuint textureColorbuffer;
	GLuint quadVAO;

	// camera stuff
	Camera camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));
	bool firstMouse = true;

	static void mouse_callback(GLFWwindow* window, double xPosD, double yPosD);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	static void error_callback(int errorcode, const char* description);

	void registerCallbacks() const;
	void createShaders();
	void updateInput();

	bool isGamePaused{};

	LayerStack layerStack;

	static Outrospection* instance;
};
