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
#include "Grid.h"

class Window {
public:
  static int        m_width;
  static int        m_height;
  static int        m_move;

  static float      m_angle;
  static int        m_swing;
  static int        m_nBody;
  static int        m_nTile;

  static glm::vec3  m_lastPoint;
  static glm::mat4  m_P;        //! P for projection
  static glm::mat4  m_V;        //! V for view

  static void initialize_objects();
  static void clean_up();
  static GLFWwindow * create_window( int width,
                                     int height );

  static void resize_callback( GLFWwindow *window,
                               int         i_width,
                               int         i_height );
  static void idle_callback();
  static void display_callback( GLFWwindow *i_window );

  static void key_callback( GLFWwindow *i_window,
                            int         i_key,
                            int         i_scancode,
                            int         i_action,
                            int i_mods );
  static void cursor_pos_callback( GLFWwindow *i_window, double i_xPos,
                                   double i_yPos );
  static void mouse_button_callback( GLFWwindow *i_window, int i_button,
                                     int i_action, int i_mods );
  static void scroll_callback( GLFWwindow *i_window, double i_xOffset,
                               double i_yOffset );
};

#endif
