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
 * Scene Graph Manager.
 **/

#include <iostream>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <cstdio>

#include "SceneGraph.h"
#include "Window.h"

Node::~Node() {}

Transform::Transform( const glm::mat4 &i_mtx ): m_tMtx(i_mtx), m_destroyed(false),
                                                m_bboxColor(2), m_type(0) {}

Transform::~Transform() {
  glDeleteVertexArrays( 1, &m_bboxVAO  );
  glDeleteVertexArrays( 1, &m_snakeVAO );

  glDeleteBuffers( 1, &m_bboxVBO  );
  glDeleteBuffers( 1, &m_snakeVBO );
}

void Transform::addChild( Node *i_child ) {
  m_ptrs.push_back( i_child );
}

//! untested: don't use
void Transform::removeChild() {
  m_ptrs.pop_back();
}

void Transform::generateBoundingBox() {
  float l_xMin  = m_position.x;
  float l_xMax  = m_position.x + m_size.x;
  float l_yMin  = m_position.y - m_size.y;
  float l_yMax  = m_position.y;
  float l_zMin  = m_position.z;
  float l_zMax  = m_position.z + m_size.z;

  //! The 8 vertices of the bounding box
  glm::vec3 l_v1( l_xMin, l_yMin, l_zMin );
  glm::vec3 l_v2( l_xMax, l_yMin, l_zMin );
  glm::vec3 l_v3( l_xMax, l_yMax, l_zMin );
  glm::vec3 l_v4( l_xMin, l_yMax, l_zMin );
  glm::vec3 l_v5( l_xMin, l_yMin, l_zMax );
  glm::vec3 l_v6( l_xMax, l_yMin, l_zMax );
  glm::vec3 l_v7( l_xMax, l_yMax, l_zMax );
  glm::vec3 l_v8( l_xMin, l_yMax, l_zMax );

  //! Construct the cuboidal bounding box
  m_bboxVertices.clear();
  m_bboxVertices.push_back( l_v1 );   m_bboxVertices.push_back( l_v2 );
  m_bboxVertices.push_back( l_v2 );   m_bboxVertices.push_back( l_v3 );
  m_bboxVertices.push_back( l_v3 );   m_bboxVertices.push_back( l_v4 );
  m_bboxVertices.push_back( l_v4 );   m_bboxVertices.push_back( l_v1 );
  m_bboxVertices.push_back( l_v1 );   m_bboxVertices.push_back( l_v5 );
  m_bboxVertices.push_back( l_v2 );   m_bboxVertices.push_back( l_v6 );
  m_bboxVertices.push_back( l_v3 );   m_bboxVertices.push_back( l_v7 );
  m_bboxVertices.push_back( l_v4 );   m_bboxVertices.push_back( l_v8 );
  m_bboxVertices.push_back( l_v5 );   m_bboxVertices.push_back( l_v6 );
  m_bboxVertices.push_back( l_v6 );   m_bboxVertices.push_back( l_v7 );
  m_bboxVertices.push_back( l_v7 );   m_bboxVertices.push_back( l_v8 );
  m_bboxVertices.push_back( l_v8 );   m_bboxVertices.push_back( l_v5 );

  glGenVertexArrays( 1, &m_bboxVAO );
  glGenBuffers( 1, &m_bboxVBO );

  glBindVertexArray( m_bboxVAO );
  glBindBuffer( GL_ARRAY_BUFFER, m_bboxVBO );

  glBufferData( GL_ARRAY_BUFFER, m_bboxVertices.size() * sizeof( glm::vec3 ),
                &m_bboxVertices[0], GL_STATIC_DRAW );
  glEnableVertexAttribArray( 0 );
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( glm::vec3 ),
                         (GLvoid*) 0 );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glBindVertexArray( 0 );
}

void Transform::drawBoundingBox( const GLuint    &i_shaderProgram,
                                 const glm::mat4 &i_mtx ) {
  glm::mat4 l_modelView = i_mtx;
  
  GLuint l_uProjection = glGetUniformLocation( i_shaderProgram, "u_projection" );
  GLuint l_uModelView  = glGetUniformLocation( i_shaderProgram, "u_modelView"  );
  GLuint l_uCamPos     = glGetUniformLocation( i_shaderProgram, "u_camPos"     );
  GLuint l_uDestroyed  = glGetUniformLocation( i_shaderProgram, "u_destroyed"  );
  GLuint l_uBBoxColor  = glGetUniformLocation( i_shaderProgram, "u_bboxColor"  );
  
  glUniformMatrix4fv( l_uProjection, 1, GL_FALSE, &Window::m_P[0][0] );
  glUniformMatrix4fv( l_uModelView,  1, GL_FALSE, &l_modelView[0][0] );
  glUniform3f( l_uCamPos,
              Window::m_camPos.x, Window::m_camPos.y, Window::m_camPos.z );
  glUniform1i( l_uDestroyed, this->m_destroyed );
  glUniform1i( l_uBBoxColor, this->m_bboxColor );
  
  glBindVertexArray( m_bboxVAO );
  glLineWidth( 1.0f );
  glDrawArrays( GL_LINES, 0, m_bboxVertices.size() );
  glBindVertexArray( 0 );
}

void Transform::generateSnakeContour() {
  int l_i;
  static float s_headPos[4]   = { 0.78f, 0.78f, 1.8f, 0.78f };

  for( l_i = 0; l_i < 4; l_i++ )
    s_headPos[l_i] += Window::m_velocity;

  //! Construct the head contours
  glm::vec3 l_vh1( -1.0f, s_headPos[0], 0.01f );
  glm::vec3 l_vh2(  1.0f, s_headPos[1], 0.01f );
  glm::vec3 l_vh3(  0.0f, s_headPos[2], 0.01f );
  glm::vec3 l_vh4(  0.0f, s_headPos[3], 0.76f );

  m_snakeVertices.clear();
  m_snakeVertices.push_back( l_vh1 );  m_snakeVertices.push_back( l_vh3 );
  m_snakeVertices.push_back( l_vh3 );  m_snakeVertices.push_back( l_vh2 );
  m_snakeVertices.push_back( l_vh1 );  m_snakeVertices.push_back( l_vh4 );
  m_snakeVertices.push_back( l_vh4 );  m_snakeVertices.push_back( l_vh2 );

  static float s_bodyPos[11]  = { -0.53f, -0.53f, -0.53f,  0.27f,  0.27f, 0.44f,
                                  -1.53f, -1.53f, -1.53f, -2.53f, -2.53f };

  for( l_i = 0; l_i < 11; l_i++ )
    s_bodyPos[l_i] += Window::m_velocity;

  //! Construct the first body part attached to head
  glm::vec3 l_v1( -0.5f, s_bodyPos[0], 0.01f );
  glm::vec3 l_v2(  0.5f, s_bodyPos[1], 0.01f );
  glm::vec3 l_v3(  0.0f, s_bodyPos[2], 0.51f );
  glm::vec3 l_v4( -0.5f, s_bodyPos[3], 0.01f );
  glm::vec3 l_v5(  0.5f, s_bodyPos[4], 0.01f );
  glm::vec3 l_v6(  0.0f, s_bodyPos[5], 0.51f );

  m_snakeVertices.push_back( l_v4 );   m_snakeVertices.push_back( l_vh1 );
  m_snakeVertices.push_back( l_v5 );   m_snakeVertices.push_back( l_vh2 );

  m_snakeVertices.push_back( l_v1 );   m_snakeVertices.push_back( l_v3 );
  m_snakeVertices.push_back( l_v3 );   m_snakeVertices.push_back( l_v2 );
  m_snakeVertices.push_back( l_v1 );   m_snakeVertices.push_back( l_v4 );
  m_snakeVertices.push_back( l_v2 );   m_snakeVertices.push_back( l_v5 );
  m_snakeVertices.push_back( l_v4 );   m_snakeVertices.push_back( l_v6 );
  m_snakeVertices.push_back( l_v6 );   m_snakeVertices.push_back( l_v5 );

  //! Second body part
  glm::vec3 l_v7(  -0.5f, s_bodyPos[6],  0.01f );
  glm::vec3 l_v8(   0.5f, s_bodyPos[7],  0.01f );
  glm::vec3 l_v9(   0.0f, s_bodyPos[8],  0.51f );

  m_snakeVertices.push_back( l_v7 );   m_snakeVertices.push_back( l_v9 );
  m_snakeVertices.push_back( l_v9 );   m_snakeVertices.push_back( l_v8 );
  m_snakeVertices.push_back( l_v7 );   m_snakeVertices.push_back( l_v1 );
  m_snakeVertices.push_back( l_v8 );   m_snakeVertices.push_back( l_v2 );

  //! Third body part
  glm::vec3 l_v10( -0.5f, s_bodyPos[9],  0.01f );
  glm::vec3 l_v11(  0.5f, s_bodyPos[10], 0.01f );

  m_snakeVertices.push_back( l_v10 );  m_snakeVertices.push_back( l_v7 );
  m_snakeVertices.push_back( l_v11 );  m_snakeVertices.push_back( l_v8 );

  glGenVertexArrays( 1, &m_snakeVAO );
  glGenBuffers( 1, &m_snakeVBO );

  glBindVertexArray( m_snakeVAO );
  glBindBuffer( GL_ARRAY_BUFFER, m_snakeVBO );

  glBufferData( GL_ARRAY_BUFFER, m_snakeVertices.size() * sizeof( glm::vec3 ),
                &m_snakeVertices[0], GL_STATIC_DRAW );
  glEnableVertexAttribArray( 0 );
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( glm::vec3 ),
                         (GLvoid*) 0 );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glBindVertexArray( 0 );
}

void Transform::drawSnakeContour( const GLuint    &i_shaderProgram,
                                  const glm::mat4 &i_mtx ) {
  glm::mat4 l_modelView = i_mtx;
  
  GLuint l_uProjection = glGetUniformLocation( i_shaderProgram, "u_projection" );
  GLuint l_uModelView  = glGetUniformLocation( i_shaderProgram, "u_modelView"  );
  GLuint l_uCamPos     = glGetUniformLocation( i_shaderProgram, "u_camPos"     );

  glUniformMatrix4fv( l_uProjection, 1, GL_FALSE, &Window::m_P[0][0] );
  glUniformMatrix4fv( l_uModelView,  1, GL_FALSE, &l_modelView[0][0] );
  glUniform3f( l_uCamPos,
              Window::m_camPos.x, Window::m_camPos.y, Window::m_camPos.z );

  glBindVertexArray( m_snakeVAO );
  glLineWidth( 2.0f );
  glDrawArrays( GL_LINES, 0, m_snakeVertices.size() );
  glBindVertexArray( 0 );
}

void Transform::draw( const GLuint &i_shaderProgram, const glm::mat4 &i_mtx ) {
  if ( m_destroyed )
    return;

  for( std::list< Node * >::iterator l_it = m_ptrs.begin(); l_it != m_ptrs.end();
        ++l_it )
    (*l_it)->draw( i_shaderProgram, i_mtx * m_tMtx );
}

void Transform::update( const glm::mat4 &i_mtx ) {
  m_tMtx  = i_mtx;
}

void Geometry::load( const char *i_fileName ) {
  float                 l_val, l_n1, l_n2, l_n3, l_mag;
  std::string           l_line, l_next;
  size_t                l_pos;
  unsigned int          l_i, l_index;
  std::vector< float >  l_x, l_y, l_z;

  std::ifstream l_in( i_fileName );
  if( !l_in.is_open() ) {
    std::cerr << "Error loading file " << i_fileName << std::endl;
    exit( EXIT_FAILURE );
  }

  while( getline( l_in, l_line ) ) {
    //! normals
    if( l_line[0] == 'v' && l_line[1] == 'n' ) {
      std::istringstream l_ss( l_line );
      std::vector< std::string > l_tokens;

      while( l_ss ) {
        if( !getline( l_ss, l_next, ' ' ) || l_tokens.size() == 4 )
          break;

        l_tokens.push_back( l_next );
      }

      l_n1  = atof( l_tokens[1].c_str() );
      l_n2  = atof( l_tokens[2].c_str() );
      l_n3  = atof( l_tokens[3].c_str() );

      l_mag = sqrtf( pow( l_n1, 2.0f ) + pow( l_n2, 2.0f ) + pow( l_n3, 2.0f ) );
      l_n1  = (l_n1 / l_mag) * 0.5f + 0.5f;
      l_n2  = (l_n2 / l_mag) * 0.5f + 0.5f;
      l_n3  = (l_n3 / l_mag) * 0.5f + 0.5f;

      //! Populate normals
      m_normals.push_back( l_n1 );
      m_normals.push_back( l_n2 );
      m_normals.push_back( l_n3 );
    }

    //! vertices
    else if ( l_line[0] == 'v' && l_line[1] == ' ' ) {
      std::istringstream l_ss( l_line );
      std::vector< std::string > l_tokens;

      while( l_ss ) {
        if( !getline( l_ss, l_next, ' ' ) || l_tokens.size() == 4 )
          break;

        l_tokens.push_back( l_next );
      }

      //! Populate vertices
      l_val = atof( l_tokens[1].c_str() );
      m_vertices.push_back( l_val );
      l_x.push_back( l_val );

      l_val = atof( l_tokens[2].c_str() );
      m_vertices.push_back( l_val );
      l_y.push_back( l_val );

      l_val = atof( l_tokens[3].c_str() );
      m_vertices.push_back( l_val );
      l_z.push_back( l_val );
    }

    //! faces
    else if( l_line[0] == 'f' ) {
      std::istringstream l_ss( l_line );
      std::vector< std::string > l_tokens;

      while( l_ss ) {
        if( !getline( l_ss, l_next, ' ' ) || l_tokens.size() == 4 )
          break;

        l_tokens.push_back( l_next );
      }

      for( l_i = 1; l_i < 4; l_i++ ) {
        l_pos = l_tokens[l_i].find( "//" );
        l_index = atoi( (l_tokens[l_i].substr( 0, l_pos )).c_str() ) - 1;

        //! Populate face-indices
        m_indices.push_back( l_index );
      }
    }
  }

  l_in.close();
}

Geometry::Geometry( const char *i_fileName ) : m_obstacleType(1) {
  //! parse and load the obj file
  load( i_fileName );

  glGenVertexArrays( 1, &m_VAO );

  glGenBuffers( 1, &m_VBO );
  glGenBuffers( 1, &m_EBO );
  glGenBuffers( 1, &m_NBO );

  glBindVertexArray( m_VAO );

  glBindBuffer( GL_ARRAY_BUFFER, m_VBO );
  glBufferData( GL_ARRAY_BUFFER, m_vertices.size() * sizeof( GLfloat ),
                &m_vertices[0], GL_STATIC_DRAW );
  glEnableVertexAttribArray( 0 );
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( GLfloat ),
                         (GLvoid *) 0 );

  glBindBuffer( GL_ARRAY_BUFFER, m_NBO );
  glBufferData( GL_ARRAY_BUFFER, m_normals.size() * sizeof( GLfloat ),
                &m_normals[0], GL_STATIC_DRAW );
  glEnableVertexAttribArray( 1 );
  glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( GLfloat ),
                         (GLvoid *) 0 );

  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_EBO );
  glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof( GLuint ),
                &m_indices[0], GL_STATIC_DRAW );

  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glBindVertexArray( 0 );
}

Geometry::~Geometry() {
  glDeleteVertexArrays( 1, &m_VAO );

  glDeleteBuffers( 1, &m_VBO );
  glDeleteBuffers( 1, &m_NBO );
  glDeleteBuffers( 1, &m_EBO );
}

void Geometry::draw( const GLuint &i_shaderProgram, const glm::mat4 &i_mtx ) {
  glm::mat4 l_modelView = i_mtx;

  GLuint l_uProjection = glGetUniformLocation( i_shaderProgram, "u_projection" );
  GLuint l_uModelView  = glGetUniformLocation( i_shaderProgram, "u_modelView"  );
  GLuint l_uCamPos     = glGetUniformLocation( i_shaderProgram, "u_camPos"     );

  glUniform3f( glGetUniformLocation( i_shaderProgram, "dirLight.direction" ),
                                                            0.0f, 1.0f, 1.0f );
  glUniform3f( glGetUniformLocation( i_shaderProgram, "dirLight.ambient"   ),
                                                            0.2f, 0.2f, 0.2f );
  glUniform3f( glGetUniformLocation( i_shaderProgram, "dirLight.diffuse"   ),
                                                            1.0f, 1.0f, 1.0f );
  glUniform3f( glGetUniformLocation( i_shaderProgram, "dirLight.specular"  ),
                                                            0.3f, 0.3f, 0.3f );

  glUniform3f( glGetUniformLocation( i_shaderProgram, "dirLight2.direction" ),
              0.0f, 0.1f, 1.2f );
  glUniform3f( glGetUniformLocation( i_shaderProgram, "dirLight2.ambient"   ),
              0.8f, 0.8f, 0.8f );
  glUniform3f( glGetUniformLocation( i_shaderProgram, "dirLight2.diffuse"   ),
              0.6f, 0.6f, 0.6f );
  glUniform3f( glGetUniformLocation( i_shaderProgram, "dirLight2.specular"  ),
              0.7f, 0.7f, 0.7f );

  GLuint l_uObstacleType = glGetUniformLocation( i_shaderProgram, "u_obstacleType" );
  glUniform1i( l_uObstacleType, this->m_obstacleType );

  glUniformMatrix4fv( l_uProjection, 1, GL_FALSE, &Window::m_P[0][0] );
  glUniformMatrix4fv( l_uModelView,  1, GL_FALSE, &l_modelView[0][0] );
  glUniform3f( l_uCamPos,
               Window::m_camPos.x, Window::m_camPos.y, Window::m_camPos.z );

  glBindVertexArray( m_VAO );
  glDrawElements( GL_TRIANGLES, m_indices.size() * sizeof( GLuint ),
                  GL_UNSIGNED_INT, 0 );

  glBindVertexArray( 0 );
}

void Geometry::update( const glm::mat4 &i_mtx ) {}