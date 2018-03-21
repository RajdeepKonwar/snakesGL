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
 * Obstacles Fragment Shader.
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

in vec3 WorldPos;
in vec3 WorldNormal;

const vec3 DiffuseLight = vec3( 0.15f, 0.05f, 0.0f );
const vec3 RimColor     = vec3( 0.2f, 0.2f, 0.2f );

const float gamma       = 1.0f / 0.3f;

uniform DirLight dirLight;
uniform vec4     u_camPos;
uniform int      u_obstacleType;
uniform bool     u_fog;

out vec4 FragColor;

void main() {
  // Directional Lighting
  vec3 viewDirection = normalize ( vec3( u_camPos.x, u_camPos.y, u_camPos.z ) -
                       vec3( ViewSpace.x, ViewSpace.y, ViewSpace.z ) );

  vec4 l_obstacleColor = vec4( 0.0f, 0.0f, 0.0f, 1.0f );  
  l_obstacleColor += vec4( CalcDirLight( dirLight, normalize( Normal ),
                                         viewDirection ), 0.0f );

  // Rim shading
  vec3 l_diffuse    = DiffuseLight * max( 0, dot( dirLight.direction,
                                                  WorldNormal ) );
  float l_rim       = 1 - max( dot( viewDirection, WorldNormal ), 0 );
  l_rim             = smoothstep( 0.6, 1.0, l_rim );
  vec3 l_finalRim   = RimColor * vec3( l_rim, l_rim, l_rim );
  
  vec3 l_finalColor = l_finalRim + l_diffuse + vec3( l_obstacleColor );
  
  vec3 l_finalColorGamma  = vec3( pow( l_finalColor.r, gamma ),
                                  pow( l_finalColor.g, gamma ),
                                  pow( l_finalColor.b, gamma ) );

  //! Linear fog
  vec3 l_distVector = vec3( ViewSpace ) - vec3( u_camPos );
  float l_dist      = length( l_distVector );

  float l_minFogDist = 2.0f;
  float l_maxFogDist = 17.0f;

  float l_fogFactor = (l_maxFogDist - l_dist) / (l_maxFogDist - l_minFogDist);
  
  vec4 l_fogColor   = vec4( 0.3f, 0.3f, 0.3f, 1.0f );  //! grey

  l_fogFactor = clamp( l_fogFactor, 0.0f, 1.0f );
  if (u_fog)
    FragColor   = mix( l_fogColor, vec4( l_finalColorGamma, 1 ), l_fogFactor );
  else
    FragColor   = vec4( l_finalColorGamma, 1 );
}

//! Calculates the color when using a directional light.
vec3 CalcDirLight( DirLight light, vec3 normal, vec3 viewDir ) {
  vec3 lightDir = normalize( light.direction );

  // Diffuse shading
  float diff = max( dot( normal, lightDir ), 0.0 );

  // Specular shading
  vec3 reflectDir = reflect( -lightDir, normal );
  float spec      = pow( max( dot( viewDir, reflectDir ), 0.0 ), 0.9 );

  //! Combine results
  vec3 ambient, diffuse, specular;

  //! pyramid
  if( u_obstacleType == 1 ) {
    ambient   = light.ambient  * vec3( 0.135, 0.2225, 0.1575 );
    diffuse   = light.diffuse  * diff * vec3( 0.54, 0.89, 0.63 );
    specular  = light.specular * spec * vec3( 0.5, 0.5, 0.5 ) *
                dot( vec3( 0.5, 0.5, 0.5 ), light.specular );
  }

  //! coin
  else if( u_obstacleType == 2 ) {
    ambient   = light.ambient  * vec3( 0.24725, 0.1995, 0.0745 );
    diffuse   = light.diffuse  * diff * vec3( 0.75164, 0.60648, 0.22648 );
    specular  = light.specular * spec * vec3( 0.628281, 0.555802, 0.366065 ) *
                dot( vec3( 0.628281, 0.555802, 0.366065 ), light.specular );
  }

  //! wall
  else if( u_obstacleType == 3 ) {
    ambient   = light.ambient  * vec3( 0.25, 0.25, 0.25 );
    diffuse   = light.diffuse  * diff * vec3( 0.4, 0.4, 0.4 );
    specular  = light.specular * spec * vec3( 0.774597, 0.774597, 0.774597 ) *
                dot( vec3( 0.774597, 0.774597, 0.774597), light.specular );
  }

  return (ambient + diffuse + specular);
}