/**
 * @file This file is part of snakesGL.
 *
 * @section LICENSE
 * GNU General Public License v2.0
 *
 * Copyright (c) 2018-2019 Rajdeep Konwar, Luke Rohrer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @section DESCRIPTION
 * main program.
 **/

#include "snakesGL.h"

GLFWwindow *G_window;

void errorCallback(int error, const char *description)
{
	// Print error
	std::cerr << description << std::endl;
}

void setupCallbacks()
{
	// Set the error callback
	glfwSetErrorCallback(errorCallback);

	// Set the key callback
	glfwSetKeyCallback(G_window, Window::keyCallback);

	// Set cursor position callback
	glfwSetCursorPosCallback(G_window, Window::cursorPosCallback);

	// Set mouse button callback
	glfwSetMouseButtonCallback(G_window, Window::mouseButtonCallback);

	// Set scroll callback
	glfwSetScrollCallback(G_window, Window::scrollCallback);

	// Set the window resize callback
	glfwSetFramebufferSizeCallback(G_window, Window::resizeCallback);
}

void setupGlew()
{
	// Initialize GLEW. Not needed on OSX systems.
#ifndef __APPLE__
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
		glfwTerminate();
	}

	std::cout << "Current GLEW version: " << glewGetString(GLEW_VERSION) << std::endl;
#endif
}

void setupOpenGLSettings()
{
#ifndef __APPLE__
	// Setup GLEW. Don't do this on OSX systems.
	setupGlew();
#endif

	// Enable depth buffering
	glEnable(GL_DEPTH_TEST);

	// Related to shaders and z value comparisons for the depth buffer
	glDepthFunc(GL_LEQUAL);

	// Set polygon drawing mode to fill front and back of each polygon
	// You can also use the paramter of GL_LINE instead of GL_FILL to see wireframes
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Disable backface culling to render both sides of polygons
	// glDisable( GL_CULL_FACE );

	// Set clear color
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);	// grey

	// Clear the z-buffer
	glClearDepth(1);
}

void printVersions()
{
	// Get info of GPU and supported OpenGL version
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "OpenGL version supported " << glGetString(GL_VERSION) << std::endl;

	// If the shading language symbol is defined
#ifdef GL_SHADING_LANGUAGE_VERSION
	std::cout << "Supported GLSL version is " << (char *)glGetString(GL_SHADING_LANGUAGE_VERSION) << ".\n";
#endif
}

// Display FPS (Frames per second)
void showFPS()
{
	float currTicks = static_cast<float>(clock());
	static float prevTicks = static_cast<float>(clock());
	float frameTime = currTicks - prevTicks;

	constexpr int numSamples = 100;
	static float frameTimes[numSamples];
	static int currFrame = 0;
	frameTimes[currFrame % numSamples] = frameTime;
	prevTicks = currTicks;

	int count = 0;
	if (currFrame < numSamples)
		count = currFrame;
	else
		count = numSamples;

	float frameTimeAvg = 0.0f;
	for (int i = 0; i < count; i++)
		frameTimeAvg += frameTimes[i];
	frameTimeAvg /= count;

	float fps;
	if (frameTimeAvg > 0.0f)
		fps = (100000.0f) / frameTimeAvg;
	else
		fps = 60.0f;

	currFrame++;
	std::cout << "\r" << fps << std::flush;
}

int main(int argc, char **argv)
{
	constexpr int windowWidth = 1280;
	constexpr int windowHeight = 720;

	// Create the GLFW window
	G_window = Window::createWindow(windowWidth, windowHeight);

	// Print OpenGL and GLSL versions
	printVersions();

	// Setup callbacks
	setupCallbacks();

	// Setup OpenGL settings, including lighting, materials, etc.
	setupOpenGLSettings();

	// Initialize objects/pointers for rendering
	Window::initializeObjects();

	// Loop while GLFW window should stay open
	while (!glfwWindowShouldClose(G_window))
	{
		// Main render display callback. Rendering of objects is done here.
		Window::displayCallback(G_window);

		// Idle callback. Updating objects, etc. can be done here.
		Window::idleCallback();
		showFPS();
	}

	std::cout << std::endl;
	Window::cleanUp();

	// Destroy the window
	glfwDestroyWindow(G_window);

	// Terminate GLFW
	glfwTerminate();

	return EXIT_SUCCESS;
}
