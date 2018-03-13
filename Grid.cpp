#include <iostream>
#include <fstream>
#include "Grid.h"
#include "Window.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Grid::Grid() : m_toWorld( glm::rotate( glm::mat4( 1.0f ),
                                       -90.0f / 180.0f * glm::pi< GLfloat >(),
                                       glm::vec3( 1.0f, 0.0f, 0.0f ) ) *
                          glm::mat4( 1.0f ) ) {
  glGenVertexArrays( 1, &m_VAO );
  glGenBuffers( 1, &m_VBO );
  glGenBuffers( 1, &m_EBO );

  glBindVertexArray( m_VAO );

  glBindBuffer( GL_ARRAY_BUFFER, m_VBO );
  glBufferData( GL_ARRAY_BUFFER, sizeof( gridVertices ), gridVertices,
                GL_STATIC_DRAW );

  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( gridIndices ), gridIndices,
                GL_STATIC_DRAW);

  //! position attribute
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof( GLfloat ),
                         (void*) 0 );
  glEnableVertexAttribArray( 0 );

  //! color attribute
  glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof( GLfloat ),
                         (void*) (3 * sizeof( GLfloat )) );
  glEnableVertexAttribArray( 1 );

  //! texture coord attribute
  glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof( GLfloat ),
                         (void*) (6 * sizeof( GLfloat )) );
  glEnableVertexAttribArray( 2 );

  // glBindBuffer( GL_ARRAY_BUFFER, 0 );
  // glBindVertexArray( 0 );

  m_textureID = LoadGrid();
}

Grid::~Grid() {
  glDeleteVertexArrays( 1, &m_VAO );
  glDeleteBuffers( 1, &m_VBO );
}

unsigned int Grid::LoadGrid() {
  unsigned int l_textureID;
  glGenTextures( 1, &l_textureID );
  glBindTexture( GL_TEXTURE_2D, l_textureID );

  //! Set the texture wrapping/filtering options (on the currently bound texture object)
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

  //! Load and generate the texture
  int l_width, l_height, l_nrChannels;
  unsigned char *l_data = stbi_load( "blue_tile.jpg", &l_width,
                                      &l_height, &l_nrChannels, 0 );

  if( l_data ) {
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, l_width,
                  l_height, 0, GL_RGB, GL_UNSIGNED_BYTE, l_data );
    glGenerateMipmap( GL_TEXTURE_2D );
  } else
    std::cerr << "Failed to load texture!" << std::endl;

  stbi_image_free( l_data );

  return l_textureID;
}

void Grid::draw( GLuint i_shaderProgram ) {
  glm::mat4 l_modelView = Window::m_V * this->m_toWorld;

  m_uProjection = glGetUniformLocation( i_shaderProgram, "projection" );
  m_uModelView  = glGetUniformLocation( i_shaderProgram, "modelView" );

  glUniformMatrix4fv( m_uProjection, 1, GL_FALSE, &Window::m_P[0][0] );
  glUniformMatrix4fv( m_uModelView,  1, GL_FALSE, &l_modelView[0][0] );

  glBindTexture( GL_TEXTURE_2D, m_textureID );
  glBindVertexArray( m_VAO );
  glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );

  glBindVertexArray( 0 );
}
