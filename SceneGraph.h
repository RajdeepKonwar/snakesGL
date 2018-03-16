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
 * Window, scene and objects manager.
 **/

#ifndef _SCENE_GRAPH_H_
#define _SCENE_GRAPH_H_

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include <OpenGL/glext.h>
#include <OpenGL/gl.h> // Remove this line in future projects
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

#include <list>
#include <vector>

//! abstract Node class
class Node {
public:
  //! pure virtual destructor
  virtual ~Node() = 0;

  //! pure virtual functions
  virtual void draw( const GLuint &i_shaderProgram, const glm::mat4 &i_mtx ) = 0;
  virtual void update( const glm::mat4 &i_mtx ) = 0;
};

//! derived Transform class
class Transform : public Node {
private:
  GLuint                   m_VAO, m_VBO;
  glm::mat4                m_tMtx;
  std::list< Node * >      m_ptrs;
  std::vector< glm::vec3 > m_vertices;

public:
  Transform( const glm::mat4 &i_mtx );
  ~Transform();
  
  glm::vec3 m_position, m_size;
  bool m_destroyed;

  void addChild( Node *i_child );
  void removeChild();

  void generateBoundingBox();
  void drawBoundingBox( const GLuint    &i_shaderProgram,
                        const glm::mat4 &i_mtx );

  void draw( const GLuint &i_shaderProgram, const glm::mat4 &i_mtx );
  void update( const glm::mat4 &i_mtx );
};

//! derived Geometry class
class Geometry : public Node {
private:
  GLuint                  m_VAO, m_VBO, m_NBO, m_EBO;
  std::vector< GLfloat >  m_vertices, m_normals;
  std::vector< GLuint >   m_indices;

  void load( const char *i_fileName );

public:
  Geometry( const char *i_fileName );
  ~Geometry();

  void draw( const GLuint &i_shaderProgram, const glm::mat4 &i_mtx );
  void update( const glm::mat4 &i_mtx );
};


#endif
