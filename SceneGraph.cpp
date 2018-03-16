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

Transform::Transform( const glm::mat4 &i_mtx ): m_destroyed(false) {
  m_tMtx  = i_mtx;
}

void Transform::addChild( Node *i_child ) {
  m_ptrs.push_back( i_child );
}

//! untested: don't use
void Transform::removeChild() {
  m_ptrs.pop_back();
}

void Transform::draw( const GLuint &i_shaderProgram, const glm::mat4 &i_mtx ) {
  if ( m_destroyed )
    return;
  for( std::list< Node * >::iterator l_it = m_ptrs.begin(); l_it != m_ptrs.end(); ++l_it )
    (*l_it)->draw( i_shaderProgram, i_mtx * m_tMtx );
}

void Transform::update( const glm::mat4 &i_mtx ) {
  m_tMtx  = i_mtx;
}

void Transform::generateBoundingBox() {
  float l_xMin  = m_position.x;
  float l_xMax  = m_position.x + m_size.x;
  float l_yMin  = m_position.y - m_size.y;
  float l_yMax  = m_position.y;
  float l_zMin  = m_position.z;
  float l_zMax  = m_size.z;

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
  m_vertices.clear();
  m_vertices.push_back( l_v1 );   m_vertices.push_back( l_v2 );
  m_vertices.push_back( l_v2 );   m_vertices.push_back( l_v3 );
  m_vertices.push_back( l_v3 );   m_vertices.push_back( l_v4 );
  m_vertices.push_back( l_v4 );   m_vertices.push_back( l_v1 );
  m_vertices.push_back( l_v1 );   m_vertices.push_back( l_v5 );
  m_vertices.push_back( l_v2 );   m_vertices.push_back( l_v6 );
  m_vertices.push_back( l_v3 );   m_vertices.push_back( l_v7 );
  m_vertices.push_back( l_v4 );   m_vertices.push_back( l_v8 );
  m_vertices.push_back( l_v5 );   m_vertices.push_back( l_v6 );
  m_vertices.push_back( l_v6 );   m_vertices.push_back( l_v7 );
  m_vertices.push_back( l_v7 );   m_vertices.push_back( l_v8 );
  m_vertices.push_back( l_v8 );   m_vertices.push_back( l_v5 );

  glGenVertexArrays( 1, &m_VAO );
  glGenBuffers( 1, &m_VBO );
  
  glBindVertexArray( m_VAO );
  glBindBuffer( GL_ARRAY_BUFFER, m_VBO );
  
  glBufferData( GL_ARRAY_BUFFER, m_vertices.size() * sizeof( glm::vec3 ),
                &m_vertices[0], GL_STATIC_DRAW );
  glEnableVertexAttribArray( 0 );
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( glm::vec3 ), (GLvoid*) 0 );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glBindVertexArray( 0 );
}

void Transform::drawBoundingBox( const GLuint &i_shaderProgram, const glm::mat4 &i_mtx ) {
  glm::mat4 l_modelView = i_mtx;
  
  GLuint l_uProjection = glGetUniformLocation( i_shaderProgram, "u_projection" );
  GLuint l_uModelView  = glGetUniformLocation( i_shaderProgram, "u_modelView"  );
  GLuint l_uCamPos     = glGetUniformLocation( i_shaderProgram, "u_camPos"     );
  GLuint l_uDestroyed   = glGetUniformLocation( i_shaderProgram, "u_destroyed"  );

  glUniformMatrix4fv( l_uProjection, 1, GL_FALSE, &Window::m_P[0][0] );
  glUniformMatrix4fv( l_uModelView,  1, GL_FALSE, &l_modelView[0][0] );
  glUniform3f( l_uCamPos,
              Window::m_camPos.x, Window::m_camPos.y, Window::m_camPos.z );
  glUniform1i( l_uDestroyed, this->m_destroyed );

  glBindVertexArray( m_VAO );
  glLineWidth( 1.0f );
  glDrawArrays( GL_LINES, 0, m_vertices.size() );
  glBindVertexArray( 0 );
}

Transform::~Transform() {
  glDeleteVertexArrays( 1, &m_VAO );
  glDeleteBuffers( 1, &m_VBO );
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

Geometry::Geometry( const char *i_fileName ) {
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

