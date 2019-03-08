/**
 * @file This file is part of snakesGL.
 *
 * @section LICENSE
 * MIT License
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
 * Obstacles Vertex Shader.
 **/

#version 330 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;

uniform mat4 u_projection;
uniform mat4 u_modelView;

out vec3 Normal;
out vec3 FragCoord;
out vec4 ViewSpace;

out vec3 WorldPos;
out vec3 WorldNormal;

void main() {
  gl_Position = u_projection * u_modelView * vec4( a_pos, 1.0f );
  ViewSpace   = u_modelView * vec4( a_pos, 1.0f );
  FragCoord   = a_pos;
  Normal      = a_normal;
  WorldPos    = mat3( u_modelView ) * a_pos ;
  WorldNormal = normalize( mat3( u_modelView ) * a_normal );
}