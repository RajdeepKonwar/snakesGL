/**
 * @file This file is part of snakesGL.
 *
 * @section LICENSE
 * MIT License
 *
 * Copyright (c) 2018 Rajdeep Konwar, Luke Rohrer
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

#define GLFW_INCLUDE_GLEXT
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
#include "Bezier.h"
#include "SceneGraph.h"
#include "Shader.h"

class Window {
private:
  static float randGenX();
  static float randGenY();

public:
  static int        m_width;
  static int        m_height;
  static int        m_move;
  static int        m_nBody;
  static int        m_nTile;

  static float      m_velocity;

  static glm::vec3  m_camPos;
  static glm::vec3  m_lastPoint;

  static glm::mat4  m_P;        //! P for projection
  static glm::mat4  m_V;        //! V for view

  static bool m_fog;

  static void initializeObjects();
  static void cleanUp();
  static GLFWwindow * createWindow( int i_width,
                                    int i_height );
  
  static bool checkCollision( Node *i_one,
                              Node *i_two );
  static void performCollisions();

  static void displayCallback( GLFWwindow *i_window );
  static void idleCallback();
  static void resizeCallback( GLFWwindow *window,
                              int         i_width,
                              int         i_height );

  static void keyCallback( GLFWwindow *i_window,
                           int         i_key,
                           int         i_scancode,
                           int         i_action,
                           int         i_mods );

  static void cursorPosCallback( GLFWwindow *i_window,
                                 double      i_xPos,
                                 double      i_yPos );
  static void mouseButtonCallback( GLFWwindow *i_window,
                                   int         i_button,
                                   int         i_action,
                                   int         i_mods );
  static void scrollCallback( GLFWwindow *i_window,
                              double      i_xOffset,
                              double      i_yOffset );
};

#endif
