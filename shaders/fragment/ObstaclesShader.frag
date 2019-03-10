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
 * Obstacles Fragment Shader.
 **/

#version 330 core

struct DirLight
{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

in vec3 Normal;
in vec3 FragCoord;
in vec4 ViewSpace;

in vec3 WorldPos;
in vec3 WorldNormal;

const vec3 DiffuseLight = vec3(0.15f, 0.05f, 0.0f);
const vec3 RimColor = vec3(0.2f, 0.2f, 0.2f);

const float gamma = 1.0f / 0.3f;

uniform DirLight dirLight;
uniform vec4 u_camPos;
uniform int u_obstacleType;
uniform bool u_fog;

out vec4 FragColor;

void main()
{
	// Directional Lighting
	vec3 viewDirection = normalize(vec3(u_camPos.x, u_camPos.y, u_camPos.z) - vec3(ViewSpace.x, ViewSpace.y, ViewSpace.z));

	vec4 obstacleColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	obstacleColor += vec4(CalcDirLight(dirLight, normalize(Normal), viewDirection), 0.0f);

	// Rim shading
	vec3 diffuse = DiffuseLight * max(0, dot(dirLight.direction, WorldNormal));
	float rim = 1.0f - max(dot(viewDirection, WorldNormal), 0.0f);
	rim = smoothstep(0.6f, 1.0f, rim);
	vec3 finalRim = RimColor * vec3(rim, rim, rim);

	vec3 finalColor = finalRim + diffuse + vec3(obstacleColor);

	vec3 finalColorGamma  = vec3(pow(finalColor.r, gamma), pow(finalColor.g, gamma), pow(finalColor.b, gamma));

	//! Linear fog
	vec3 distVector = vec3(ViewSpace) - vec3(u_camPos);
	float dist = length(distVector);

	float minFogDist = 2.0f;
	float maxFogDist = 17.0f;

	float fogFactor = (maxFogDist - dist) / (maxFogDist - minFogDist);
  
	vec4 fogColor = vec4(0.3f, 0.3f, 0.3f, 1.0f);		//! grey

	fogFactor = clamp(fogFactor, 0.0f, 1.0f);
	if (u_fog)
		FragColor = mix(fogColor, vec4(finalColorGamma, 1), fogFactor);
	else
		FragColor = vec4(finalColorGamma, 1);
}

//! Calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(light.direction);

	// Diffuse shading
	float diff = max(dot(normal, lightDir), 0.0f);

	// Specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 0.9f);

	//! Combine results
	vec3 ambient, diffuse, specular;

	//! pyramid
	if (u_obstacleType == 1)
	{
		ambient = light.ambient * vec3(0.135f, 0.2225f, 0.1575f);
		diffuse = light.diffuse * diff * vec3(0.54f, 0.89f, 0.63f);
		specular = light.specular * spec * vec3(0.5f, 0.5f, 0.5f) * dot(vec3(0.5f, 0.5f, 0.5f), light.specular);
	}

	//! coin
	else if (u_obstacleType == 2)
	{
		ambient = light.ambient * vec3(0.24725f, 0.1995f, 0.0745f);
		diffuse = light.diffuse * diff * vec3(0.75164f, 0.60648f, 0.22648f);
		specular = light.specular * spec * vec3(0.628281f, 0.555802f, 0.366065f) * dot(vec3(0.628281f, 0.555802f, 0.366065f), light.specular);
	}

	//! wall
	else if (u_obstacleType == 3)
	{
		ambient = light.ambient * vec3(0.25f, 0.25f, 0.25f);
		diffuse = light.diffuse * diff * vec3(0.4f, 0.4f, 0.4f);
		specular = light.specular * spec * vec3(0.774597f, 0.774597f, 0.774597f) * dot(vec3(0.774597f, 0.774597f, 0.774597f), light.specular);
	}

	return (ambient + diffuse + specular);
}
