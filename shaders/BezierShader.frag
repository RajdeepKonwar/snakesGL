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
 * Grid Big Fragment Shader.
 **/

#version 330 core

in vec3 FragCoord;
in vec4 ViewSpace;

uniform vec4 u_camPos;
uniform bool u_fog;

out vec4 FragColor;

void main() {
  //! Linear fog
  vec3 l_distVector = vec3( ViewSpace ) - vec3( u_camPos );
  float l_dist      = length( l_distVector );

  float l_minFogDist = 2.0f;
  float l_maxFogDist = 17.0f;

  float l_fogFactor = (l_maxFogDist - l_dist) / (l_maxFogDist - l_minFogDist);
  vec4 l_fogColor   = vec4( 0.3f, 0.3f, 0.3f, 1.0f );
  vec4 l_bezierColor  = vec4( 0.0f, 0.6f, 0.7f, 1.0f );

  l_fogFactor = clamp( l_fogFactor, 0.0f, 1.0f );
  if (u_fog)
    FragColor   = mix( l_fogColor, l_bezierColor, l_fogFactor );
  else
    FragColor   = l_bezierColor;
}
