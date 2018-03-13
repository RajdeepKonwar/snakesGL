#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

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
  glm::mat4           m_tMtx;
  std::list< Node * > m_ptrs;

public:
  Transform( const glm::mat4 &i_mtx );

  void addChild( Node *i_child );
  void removeChild();

  void draw( const GLuint &i_shaderProgram, const glm::mat4 &i_mtx );
  void update( const glm::mat4 &i_mtx );
};

//! derived Geometry class
class Geometry : public Node {
private:
  GLuint                  m_VAO, m_VBO, m_NBO, m_EBO;
  GLuint                  m_uProjection, m_uModelView;
  std::vector< GLfloat >  m_vertices, m_normals;
  std::vector< GLuint >   m_indices;

  void load( const char *i_fileName );

public:
  Geometry( const char *i_fileName );
  ~Geometry();

  void draw( const GLuint &i_shaderProgram, const glm::mat4 &i_mtx );
  void update( const glm::mat4 &i_mtx );
};

//! derived Group class
class Group : public Node {
private:
  std::list< Node * > m_ptrs;

public:
  void addChild( Node *i_child );
  void removeChild();

  void draw( const GLuint &i_shaderProgram, const glm::mat4 &i_mtx );
  void update( const glm::mat4 &i_mtx );
};

#endif
