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
 * Snake Fragment Shader.
 **/

#version 330 core

struct DirLight {
  vec3 direction;
  
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

vec3 CalcDirLight( DirLight light, vec3 normal, vec3 viewDir );

in vec3 Normal;
in vec3 FragCoord;
in vec4 ViewSpace;

uniform DirLight dirLight;
uniform vec4     u_camPos;

out vec4 FragColor;

void main() {
  // Directional Lighting
  
  vec3 viewDirection = normalize ( vec3( u_camPos.x, u_camPos.y, u_camPos.z ) -
                       vec3( ViewSpace.x, ViewSpace.y, ViewSpace.z ) );
  
  vec4 l_snakeColor = vec4( 0.0f, 0.0f, 0.0f, 1.0f );  //! red
  l_snakeColor += vec4( CalcDirLight( dirLight, normalize( Normal ), viewDirection ),
                        0.0f );

  //! Linear fog
  vec3 l_distVector = vec3( ViewSpace ) - vec3( u_camPos );
  float l_dist      = length( l_distVector );

  float l_minFogDist = 2.0f;
  float l_maxFogDist = 17.0f;

  float l_fogFactor = (l_maxFogDist - l_dist) / (l_maxFogDist - l_minFogDist);

  vec4 l_fogColor   = vec4( 0.3f, 0.3f, 0.3f, 1.0f );  //! grey

  l_fogFactor = clamp( l_fogFactor, 0.0f, 1.0f );
  FragColor   = mix( l_fogColor, l_snakeColor, l_fogFactor );
}

//! Calculates the color when using a directional light.
vec3 CalcDirLight( DirLight light, vec3 normal, vec3 viewDir ) {
  vec3 lightDir = normalize( light.direction );

  //! Diffuse shading
  float diff = max( dot( normal, lightDir ), 0.0 );

  //! Specular shading
  vec3 reflectDir = reflect( -lightDir, normal );
  float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), 0.6 );

  //! Combine results
  vec3 ambient  = light.ambient  * vec3( 0.1745f, 0.01175f, 0.01175f );
  vec3 diffuse  = light.diffuse  * diff * vec3( 0.61424f, 0.04136f, 0.04136f );
  vec3 specular = light.specular * spec * vec3( 0.727811f, 0.626959f, 0.626959f )
                  * dot(vec3(0.727811f, 0.626959f, 0.626959f), light.specular);

  return (ambient + diffuse + specular);
}