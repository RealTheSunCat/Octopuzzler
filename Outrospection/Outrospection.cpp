﻿#include "Outrospection.h"

#include <glm/ext/matrix_clip_space.hpp>

#include "Util.h"

#include "Core/UI/GUIScreen.h"

Outrospection* Outrospection::instance = nullptr;

Outrospection::Outrospection()
{
	instance = this;

	ingameGUI = std::make_unique<GUIIngame>();
	pauseGUI = std::make_unique<GUIPause>();
	
	gameWindow = opengl.gameWindow;
	quadVAO = opengl.quadVAO;
	framebuffer = opengl.framebuffer;
	textureColorbuffer = opengl.textureColorbuffer;

	fontCharacters = freetype.loadedCharacters;

	registerCallbacks();
	createShaders();

	scene = Scene("TestLevel000");
	player = Player(glm::vec3(0.0, 3.0, 0.0));

	setGUIScreen(ingameGUI.get());
}

void Outrospection::run()
{
	running = true;
	
	while (running)
	{
		runGameLoop();
	}

	glfwTerminate();
}

void Outrospection::pauseGame()
{
	get().isGamePaused = true;

	get().setGUIScreen(get().pauseGUI.get());
}

void Outrospection::unpauseGame()
{
	get().isGamePaused = false;

	get().setGUIScreen(get().ingameGUI.get());
}

void Outrospection::setGUIScreen(GUIScreen* screen, const bool replace)
{
	if(replace && !loadedGUIs.empty())
	{
		loadedGUIs.pop_back();
	}

	loadedGUIs.push_back(screen);

	loadedGUIs.back()->onFocus();
}

void Outrospection::captureMouse(const bool doCapture) const
{
	if (doCapture)
		glfwSetInputMode(gameWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	else
	{
		glfwSetInputMode(gameWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		glfwSetCursorPos(gameWindow, SCR_WIDTH / 2.0f, SCR_HEIGHT / 2.0f);

		mouse_callback(gameWindow, SCR_WIDTH / 2.0f, SCR_HEIGHT / 2.0f);
	}
}

void Outrospection::runGameLoop()
{
	std::cout << "runGameLoop" << std::endl;
	
	const double currentFrame = glfwGetTime();
	deltaTime = float(currentFrame - lastFrame);
	lastFrame = currentFrame;
	
	// fetch input into simplified controller class
	updateInput();

	// exit game on next loop iteration
	if (controller.pause == 1)
	{
		if (isGamePaused)
			unpauseGame();
		else
			pauseGame();
	}

	
	if (!isGamePaused)
	{
		// Run one "tick" of the game physics
		runTick();

		// TODO execute scheduled tasks
		// ----------------------------
	}

	// UIs are also updated when game is paused
	for (auto& screen : loadedGUIs)
	{
		screen->tick();
	}


	// Bind to framebuffer and draw scene to color texture
	// ---------------------------------------------------

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // background color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// Set shader info
	objectShader.use();
	objectShader.setVec3("viewPos", camera.position);
	objectShader.setFloat("shininess", 32.0f);
	objectShader.setVec3("lightPos", player.position + glm::vec3(0, 1.5f, 0));
	objectShader.doProjView(camera, SCR_WIDTH, SCR_HEIGHT, true);

	billboardShader.use();
	billboardShader.doProjView(camera, SCR_WIDTH, SCR_HEIGHT, true);

	skyShader.use();
	skyShader.doProjView(camera, SCR_WIDTH, SCR_HEIGHT, false);

	simpleShader.use();
	simpleShader.doProjView(camera, SCR_WIDTH, SCR_HEIGHT, true);

	// draw stuff
	scene.draw(objectShader, billboardShader, skyShader, simpleShader);

	player.draw(billboardShader);

	//Util::glError(true);

	glDisable(GL_DEPTH_TEST); // disable depth test so stuff near camera isn't clipped
	
	// bind to default framebuffer and draw custom one over that

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// clear all relevant buffers
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
	glClear(GL_COLOR_BUFFER_BIT);

	screenShader.use();
	glBindVertexArray(quadVAO);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);	// use the color attachment texture as the texture of the quad plane
	glDrawArrays(GL_TRIANGLES, 0, 6);


	// draw UI
	for(const auto& screen : loadedGUIs)
	{
		screen->draw(spriteShader, glyphShader);
	}
	
	glEnable(GL_DEPTH_TEST); // re-enable depth testing

	// check for errors
	Util::glError();

	// swap buffers and poll IO events
	// -------------------------------
	glfwSwapBuffers(gameWindow);
	glfwPollEvents();
}

void Outrospection::runTick()
{
	playerController.acceleratePlayer(controller, deltaTime, camera.yaw);
	playerController.collidePlayer(player, scene.collision);
	playerController.animatePlayer(player);
	playerController.movePlayer(player);

	if (controller.isGamepad)
		camera.playerRotateCameraBy(controller.rightSide * 12.5f, controller.rightForward * 10);
	
	updateCamera();
}

void Outrospection::updateCamera()
{
	// TODO need a better way to determine whether the camera should be auto-updated or not
	camera.calculateCameraPosition(player, scene, playerController.isMoving());
}

void Outrospection::registerCallbacks() const
{
	// Register OpenGL events
	glfwSetFramebufferSizeCallback(gameWindow, framebuffer_size_callback);
	glfwSetCursorPosCallback(gameWindow, mouse_callback);
	glfwSetScrollCallback(gameWindow, scroll_callback);
	glfwSetKeyCallback(gameWindow, key_callback);
	glfwSetErrorCallback(error_callback);
}

void Outrospection::createShaders()
{
	objectShader    = Shader("obj"      , "obj"      );
	billboardShader = Shader("billboard", "lightless");
	skyShader       = Shader("sky"      , "sky"      );
	screenShader    = Shader("screen"   , "screen"   );
	simpleShader    = Shader("simple"   , "simple"   );
	spriteShader    = Shader("sprite"   , "sprite"   );
	glyphShader     = Shader("sprite"   , "glyph"    );

	// set up 2d shader
	const glm::mat4 projection = glm::ortho(0.0f, float(SCR_WIDTH),
	                                        float(SCR_HEIGHT), 0.0f, -1.0f, 1.0f);
	spriteShader.use();
	spriteShader.setMat4("projection", projection);

	glyphShader.use();
	glyphShader.setMat4("projection", projection);
}

// set proper viewport size when window is resized
void Outrospection::framebuffer_size_callback(GLFWwindow*, const int width, const int height)
{
	glViewport(0, 0, width, height);
}

void Outrospection::mouse_callback(GLFWwindow*, const double xPosD, const double yPosD)
{
	Outrospection& orig = get();

	const auto xPos = float(xPosD);
	const auto yPos = float(yPosD);
	
	if (orig.firstMouse) {
		orig.lastMousePos.x = xPos;
		orig.lastMousePos.y = yPos;
		orig.firstMouse = false;
		return; // nothing to calculate because we technically didn't move the mouse
	}

	const float xOffset = xPos - orig.lastMousePos.x;
	const float yOffset = orig.lastMousePos.y - yPos;

	orig.lastMousePos.x = xPos;
	orig.lastMousePos.y = yPos;

	orig.camera.playerRotateCameraBy(xOffset, yOffset);
}

void Outrospection::scroll_callback(GLFWwindow*, const double xoffset, const double yoffset)
{
	Outrospection& orig = get();
	orig.camera.changeDistBy(float(yoffset));
}

void Outrospection::key_callback(GLFWwindow*, const int key, const int scancode, const int action, const int mods)
{
	const GameSettings& gameSettings = get().gameSettings;
	Controller& controller = get().controller;

	if (action == GLFW_RELEASE)
	{
		if (key == gameSettings.keyBindForward.keyCode || key == gameSettings.keyBindBackward.keyCode)
		{
			controller.leftForward = 0.0;
		}
		if (key == gameSettings.keyBindRight.keyCode || key == gameSettings.keyBindLeft.keyCode)
		{
			controller.leftSide = 0.0;
		}
		if (key == gameSettings.keyBindJump.keyCode)
		{
			controller.jump = false;
		}
		if (key == gameSettings.keyBindExit.keyCode)
		{
			controller.pause = false;
		}
		if(key == gameSettings.keyBindTalk.keyCode)
		{
			controller.talk = false;
		}
		if(key == gameSettings.keyBindBreak.keyCode)
		{
			controller.debugBreak = false;
		}
	}
}
void Outrospection::error_callback(const int errorcode, const char* description)
{
	std::cout << "ERROR::GLFW code = " << errorcode << ". " << description << '\n';
}

void setKey(ControllerButton& button, const int keyCode, GLFWwindow* window)
{
	button = glfwGetKey(window, keyCode) == GLFW_PRESS ? button + 1 : false;
}

void Outrospection::updateInput()
{
	int joystick = -1;
	for (int i = GLFW_JOYSTICK_1; i < GLFW_JOYSTICK_LAST; i++)
	{
		if (glfwJoystickPresent(i) == GLFW_TRUE)
		{
			joystick = i;

			controller.isGamepad = true;

#ifdef VERBOSE
				std::cout << "Joystick " << glfwGetJoystickName(joystick)
					<< " detected with ID " << joystick << "! ";
#endif

			break;
		}
	}
								
	if (joystick != -1) {					 // there is a controller
		
		if (glfwJoystickIsGamepad(joystick)) // easy!
		{
#ifdef VERBOSE
				std::cout << "It is a gamepad! Sweet! ";
#endif
			
			GLFWgamepadstate gamepadState;
			glfwGetGamepadState(joystick, &gamepadState);

			// make negative bc flipped!
			controller.leftForward = -Util::valFromJoystickAxis(gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]);
			controller.leftSide = Util::valFromJoystickAxis(gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_X]);

			controller.rightForward = -Util::valFromJoystickAxis(gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]);
			controller.rightSide = Util::valFromJoystickAxis(gamepadState.axes[GLFW_GAMEPAD_AXIS_RIGHT_X]);

			controller.leftTrigger = Util::valFromJoystickAxis(gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER]);

			controller.jump  = gamepadState.buttons[GLFW_GAMEPAD_BUTTON_A]     ? controller.jump  + 1 : false;
			controller.talk  = gamepadState.buttons[GLFW_GAMEPAD_BUTTON_X]     ? controller.talk  + 1 : false;
			controller.pause = gamepadState.buttons[GLFW_GAMEPAD_BUTTON_START] ? controller.pause + 1 : false;
		}
		else // have to manually set everything :c
		{
#ifdef VERBOSE
				std::cout << "It is a non-mapped controller. Hrm. ";
#endif

			int axesCount = -1;

			const float* rawAxes = glfwGetJoystickAxes(joystick, &axesCount);

			if (axesCount < 2) // no sticks, return for now?
			{
				joystick = -1;
			}
			else if (axesCount == 2) // one stick! assumed to be left so we can move around
			{
				controller.leftForward = -Util::valFromJoystickAxis(rawAxes[STICK_LEFT_UP]);
				controller.leftSide = Util::valFromJoystickAxis(rawAxes[STICK_LEFT_SIDE]);

			}
			else if (axesCount >= 4) // two sticks or more
			{
				controller.rightForward = -Util::valFromJoystickAxis(rawAxes[STICK_LEFT_UP]);
				controller.rightSide = Util::valFromJoystickAxis(rawAxes[STICK_LEFT_SIDE]);

				controller.leftForward = -Util::valFromJoystickAxis(rawAxes[STICK_LEFT_UP]);
				controller.leftSide = Util::valFromJoystickAxis(rawAxes[STICK_LEFT_SIDE]);
			}


			int buttonCount = -1;
			const unsigned char* rawButtons = glfwGetJoystickButtons(joystick, &buttonCount);

			if (buttonCount < 4) // not enough buttons for us
			{
				joystick = -1;
			}
			else
			{
				controller.jump = rawButtons[BUTTON_A] ? controller.jump + 1 : false;
				controller.talk = rawButtons[BUTTON_X] ? controller.talk + 1 : false;
				controller.pause = rawButtons[BUTTON_START] ? controller.pause + 1 : false;
				// TODO add more controls lol
			}
		}
	}
	
	if (joystick == -1) // no *usable* controllers are present
	{
#ifdef VERBOSE
			std::cout << "No usable controller is present. ";
#endif

		controller.isGamepad = false;

		float leftForwardInput = 0.0f;
		if (glfwGetKey(gameWindow, gameSettings.keyBindForward.keyCode) == GLFW_PRESS)
		{
			leftForwardInput += 1.0;
		}
		if (glfwGetKey(gameWindow, gameSettings.keyBindBackward.keyCode) == GLFW_PRESS)
		{
			leftForwardInput += -1.0;
		}
		controller.leftForward = leftForwardInput;


		float leftSideInput = 0.0f;
		if (glfwGetKey(gameWindow, gameSettings.keyBindRight.keyCode) == GLFW_PRESS)
		{
			leftSideInput += 1.0;
		}
		if (glfwGetKey(gameWindow, gameSettings.keyBindLeft.keyCode) == GLFW_PRESS)
		{
			leftSideInput += -1.0;
		}
		controller.leftSide = leftSideInput;


		setKey(controller.jump, gameSettings.keyBindJump.keyCode, gameWindow);
		setKey(controller.talk, gameSettings.keyBindTalk.keyCode, gameWindow);
		setKey(controller.pause, gameSettings.keyBindExit.keyCode, gameWindow);
		setKey(controller.debugBreak, gameSettings.keyBindBreak.keyCode, gameWindow);
	}

#ifdef VERBOSE
		std::cout << std::endl;
#endif
}