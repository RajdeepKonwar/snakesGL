#ifndef GRID_H
#define GRID_H

#define GLFW_INCLUDE_GLEXT
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
//! Use of degrees is deprecated. Use radians instead.
#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

class Grid {
private:
  glm::mat4 m_toWorld;

  GLuint    m_textureID;
  GLuint    m_VAO, m_VBO, m_EBO;
  GLuint    m_uProjection, m_uModelView;

  unsigned int LoadGrid();

public:
  Grid();
  ~Grid();

  void draw( GLuint i_shaderProgram );
};

const float gridVertices[] = {
  // positions          // colors           // texture coords
    0.5f,  0.5f, 0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,        // top right
    0.5f, -0.5f, -0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,        // bottom right
  -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,         // bottom left
  -0.5f,  0.5f, 0.5f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f          // top left 
};

const int gridIndices[] = {
  0, 1, 3,  //! first triangle
  1, 2, 3   //! second triangle
};

#endif