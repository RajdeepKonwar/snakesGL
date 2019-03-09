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
 * Window class.
 **/

#ifndef WINDOW_H
#define WINDOW_H

#include <iostream>
#include <ctime>
#include <cmath>

#define GLFW_INCLUDE_GLEXT
#ifdef __APPLE__
	#define GLFW_INCLUDE_GLCOREARB
#else
	#include <GL/glew.h>
#endif

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

#include <GLFW/glfw3.h>

#include "Bezier.h"
#include "SceneGraph.h"
#include "Shader.h"

constexpr auto WINDOW_TITLE = "snakesGL";
constexpr auto CONFIG_FILE = "./snakesGL.conf";

class Window
{
public:
	static void initializeObjects();
	static void cleanUp();
	static GLFWwindow* createWindow(int width, int height);
  
	static bool checkCollision(Node *first, Node *second);
	static void performCollisions();

	static void displayCallback(GLFWwindow *window);
	static void idleCallback();
	static void resizeCallback(GLFWwindow *window, int width, int height);

	static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

	static void cursorPosCallback(GLFWwindow *window, double xPos, double yPos);
	static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
	static void scrollCallback(GLFWwindow *window, double xOffset, double yOffset);

private:
	static float randGenX();
	static float randGenY();

public:
	static int m_width;
	static int m_height;
	static int m_move;
	static int m_nBody;
	static int m_nTile;

	static float m_velocity;

	static glm::vec3 m_camPos;
	static glm::vec3 m_lastPoint;

	static glm::mat4 m_P;	// P for projection
	static glm::mat4 m_V;	// V for view

	static bool m_fog;
};

#endif
