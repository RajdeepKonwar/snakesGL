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

#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <iostream>

#define GLFW_INCLUDE_GLEXT
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
#include "SceneGraph.h"
#include "Shader.h"

class Window {
public:
  static int        m_width;
  static int        m_height;
  static int        m_move;
  static int        m_nBody;
  static int        m_nTile;

  static glm::vec3  m_camPos;
  static glm::vec3  m_lastPoint;

  static glm::mat4  m_P;        //! P for projection
  static glm::mat4  m_V;        //! V for view

  static void initialize_objects();
  static void clean_up();
  static GLFWwindow * create_window( int width,
                                     int height );

  static void display_callback( GLFWwindow *i_window );
  static void idle_callback();
  static void resize_callback( GLFWwindow *window,
                               int         i_width,
                               int         i_height );

  static void key_callback( GLFWwindow *i_window,
                            int         i_key,
                            int         i_scancode,
                            int         i_action,
                            int i_mods );

  static void cursor_pos_callback( GLFWwindow *i_window,
                                   double      i_xPos,
                                   double      i_yPos );
  static void mouse_button_callback( GLFWwindow *i_window,
                                     int         i_button,
                                     int         i_action,
                                     int         i_mods );
  static void scroll_callback( GLFWwindow *i_window,
                               double      i_xOffset,
                               double      i_yOffset );
};

#endif