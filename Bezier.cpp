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
 * Bezier Surface.
 **/

#include "Bezier.h"
#include "Window.h"

Bezier::Bezier( const glm::vec3 i_points[16] ) {
  m_G[0] = glm::mat4( glm::vec4( i_points[0].x, i_points[4].x, i_points[8].x,  i_points[12].x ),
                      glm::vec4( i_points[1].x, i_points[5].x, i_points[9].x,  i_points[13].x ),
                      glm::vec4( i_points[2].x, i_points[6].x, i_points[10].x, i_points[14].x ),
                      glm::vec4( i_points[3].x, i_points[7].x, i_points[11].x, i_points[15].x ) );

  m_G[1] = glm::mat4( glm::vec4( i_points[0].y, i_points[4].y, i_points[8].y,  i_points[12].y ),
                      glm::vec4( i_points[1].y, i_points[5].y, i_points[9].y,  i_points[13].y ),
                      glm::vec4( i_points[2].y, i_points[6].y, i_points[10].y, i_points[14].y ),
                      glm::vec4( i_points[3].y, i_points[7].y, i_points[11].y, i_points[15].y ) );

  m_G[2] = glm::mat4( glm::vec4( i_points[0].z, i_points[4].z, i_points[8].z,  i_points[12].z ),
                      glm::vec4( i_points[1].z, i_points[5].z, i_points[9].z,  i_points[13].z ),
                      glm::vec4( i_points[2].z, i_points[6].z, i_points[10].z, i_points[14].z ),
                      glm::vec4( i_points[3].z, i_points[7].z, i_points[11].z, i_points[15].z ) );

  m_B = glm::mat4( glm::vec4( -1.0f,  3.0f, -3.0f, 1.0f ),
                   glm::vec4(  3.0f, -6.0f,  3.0f, 0.0f ),
                   glm::vec4( -3.0f,  3.0f,  0.0f, 0.0f ),
                   glm::vec4(  1.0f,  0.0f,  0.0f, 0.0f ) );

  m_C[0] = m_B * m_G[0] * m_B;
  m_C[1] = m_B * m_G[1] * m_B;
  m_C[2] = m_B * m_G[2] * m_B;

  glm::vec4 l_uVector;
  glm::vec4 l_vVector;
  glm::vec3 l_xOfuv;

  int l_counter = 0;
  int l_rows    = 0;

  float l_u = 0.0f;
  float l_v = 0.0f;

  while( l_rows <= 100 ) {
    while( l_counter < 202 ) {
      l_uVector = glm::vec4( l_u * l_u * l_u, l_u * l_u, l_u, 1 );
      l_vVector = glm::vec4( l_v * l_v * l_v, l_v * l_v, l_v, 1 );

      l_xOfuv   = glm::vec3( glm::dot( l_vVector, m_C[0] * l_uVector ),
                             glm::dot( l_vVector, m_C[1] * l_uVector ),
                             glm::dot( l_vVector, m_C[2] * l_uVector ) );

      m_vertices[l_rows].push_back( l_xOfuv );

      if( l_counter % 2 == 0 )
        l_u += 0.01f;
      else {
        l_u -= 0.01f;
        l_v += 0.01f;
      }

      l_counter++;
    }

    l_counter = 0;
    l_v       = 0.0f;
    l_u       = 0.0f + l_rows * 0.01f;
    l_rows++;
  }

  for( int l_j = 0; l_j <= 100; l_j++ ) {
    glGenVertexArrays( 1, &m_VAO[l_j] );
    glGenBuffers( 1, &m_VBO[l_j] );

    glBindVertexArray( m_VAO[l_j] );
    glBindBuffer( GL_ARRAY_BUFFER, m_VBO[l_j] );
    glBufferData( GL_ARRAY_BUFFER, m_vertices[l_j].size() * sizeof( glm::vec3 ),
                  &m_vertices[l_j][0], GL_STATIC_DRAW );

    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 3, GL_FLOAT,  GL_FALSE, sizeof( glm::vec3 ), 
                           (GLvoid*) 0 );

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray(0);
  }
}

void Bezier::draw( const GLuint &i_shaderProgram ) {
  glUniformMatrix4fv( glGetUniformLocation( i_shaderProgram, "projection" ), 1,
                      GL_FALSE, &Window::m_P[0][0] );
  glUniformMatrix4fv( glGetUniformLocation( i_shaderProgram, "modelView" ),  1,
                      GL_FALSE, &Window::m_V[0][0] );
  glUniform1i( glGetUniformLocation( i_shaderProgram, "u_surface" ), m_surface );
  
  GLuint l_uFog = glGetUniformLocation( i_shaderProgram, "u_fog" );
  glUniform1i( l_uFog, Window::m_fog );
  
  GLuint l_uCamPos = glGetUniformLocation( i_shaderProgram, "u_camPos" );
  glUniform3f( l_uCamPos,
              Window::m_camPos.x, Window::m_camPos.y, Window::m_camPos.z );

  for( int l_i = 0; l_i <= 100; l_i++ ) {
    glBindVertexArray( m_VAO[l_i] );
    glDrawArrays( GL_TRIANGLE_STRIP, 0, static_cast<GLsizei>(m_vertices[l_i].size()) );
    glBindVertexArray( 0 );
  }
}