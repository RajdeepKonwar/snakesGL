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
 * Shader manager.
 **/

#include <cstdio>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

#define GLFW_INCLUDE_GLEXT
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>

#include "Shader.h"

GLuint LoadShaders( const char *i_vertexFilePath,
                    const char *i_fragmentFilePath ) {
  //! Create the shaders
  GLuint VertexShaderID   = glCreateShader( GL_VERTEX_SHADER );
  GLuint FragmentShaderID = glCreateShader( GL_FRAGMENT_SHADER );

  //! Read the Vertex Shader code from the file
  std::string VertexShaderCode;
  std::ifstream VertexShaderStream( i_vertexFilePath, std::ios::in );

  if( VertexShaderStream.is_open() ) {
    std::string Line = "";

    while( getline( VertexShaderStream, Line ) )
      VertexShaderCode += "\n" + Line;

    VertexShaderStream.close();
  } else {
    printf( "Impossible to open %s. Check to make sure the file exists and you passed in the right filepath!\n",
            i_vertexFilePath );
    printf( "The current working directory is:" );
    //! Please for the love of whatever deity/ies you believe in never do something like the next line of code,
    //! Especially on non-Windows systems where you can have the system happily execute "rm -rf ~"

    int ret = 0;

#ifdef _WIN32
    ret = system( "CD" );
#else
    ret = system( "pwd" );
#endif

    getchar();
    return ret;
  }

  //! Read the Fragment Shader code from the file
  std::string FragmentShaderCode;
  std::ifstream FragmentShaderStream( i_fragmentFilePath, std::ios::in );

  if( FragmentShaderStream.is_open() ) {
    std::string Line = "";

    while( getline( FragmentShaderStream, Line ) )
      FragmentShaderCode += "\n" + Line;

    FragmentShaderStream.close();
  }

  GLint Result = GL_FALSE;
  int InfoLogLength;

  //! Compile Vertex Shader
  printf( "\nCompiling shader : %s\n", i_vertexFilePath );
  char const * VertexSourcePointer = VertexShaderCode.c_str();
  glShaderSource( VertexShaderID, 1, &VertexSourcePointer , NULL );
  glCompileShader( VertexShaderID );

  //! Check Vertex Shader
  glGetShaderiv( VertexShaderID, GL_COMPILE_STATUS, &Result );
  glGetShaderiv( VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength );

  if( InfoLogLength > 0 ) {
    std::vector< char > VertexShaderErrorMessage( InfoLogLength + 1 );
    glGetShaderInfoLog( VertexShaderID, InfoLogLength, NULL,
                        &VertexShaderErrorMessage[0] );
    printf( "%s\n", &VertexShaderErrorMessage[0] );
  } else {
    printf( "Successfully compiled vertex shader!\n" );
  }

  //! Compile Fragment Shader
  printf( "Compiling shader : %s\n", i_fragmentFilePath );
  char const * FragmentSourcePointer = FragmentShaderCode.c_str();
  glShaderSource( FragmentShaderID, 1, &FragmentSourcePointer , NULL );
  glCompileShader( FragmentShaderID );

  //! Check Fragment Shader
  glGetShaderiv( FragmentShaderID, GL_COMPILE_STATUS, &Result );
  glGetShaderiv( FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength );

  if( InfoLogLength > 0 ) {
    std::vector< char > FragmentShaderErrorMessage( InfoLogLength + 1 );
    glGetShaderInfoLog( FragmentShaderID, InfoLogLength, NULL,
                        &FragmentShaderErrorMessage[0] );
    printf( "%s\n", &FragmentShaderErrorMessage[0] );
  } else {
    printf( "Successfully compiled fragment shader!\n" );
  }

  //! Link the program
  printf( "Linking program\n" );
  GLuint ProgramID = glCreateProgram();
  glAttachShader( ProgramID, VertexShaderID );
  glAttachShader( ProgramID, FragmentShaderID );
  glLinkProgram( ProgramID );

  //! Check the program
  glGetProgramiv( ProgramID, GL_LINK_STATUS, &Result );
  glGetProgramiv( ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength );

  if( InfoLogLength > 0 ) {
    std::vector< char > ProgramErrorMessage( InfoLogLength + 1 );
    glGetProgramInfoLog( ProgramID, InfoLogLength, NULL,
                         &ProgramErrorMessage[0] );
    printf( "%s\n", &ProgramErrorMessage[0] );
  }

  glDetachShader( ProgramID, VertexShaderID );
  glDetachShader( ProgramID, FragmentShaderID );

  glDeleteShader( VertexShaderID );
  glDeleteShader( FragmentShaderID );

  return ProgramID;
}