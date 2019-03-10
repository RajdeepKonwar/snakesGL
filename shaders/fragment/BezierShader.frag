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
 * Bezier Surface Fragment Shader.
 **/

#version 330 core

in vec3 FragCoord;
in vec4 ViewSpace;

uniform vec4 u_camPos;
uniform bool u_fog;
uniform int u_surface;

out vec4 FragColor;

void main()
{
	//! Linear fog
	vec3 distVector = vec3(ViewSpace) - vec3(u_camPos);
	float dist = length(distVector);

	float minFogDist = 2.0f;
	float maxFogDist = 17.0f;

	float fogFactor = (maxFogDist - dist) / (maxFogDist - minFogDist);
	vec4 fogColor = vec4(0.3f, 0.3f, 0.3f, 1.0f);

	vec4 bezierColor = vec4(0.0f, 0.6f, 0.7f, 1.0f);
	switch (u_surface)
	{
	case 1:
		bezierColor = vec4(0.0f, 0.63f, 0.95f, 1.0f);
		break;
	case 2:
		bezierColor = vec4(1.0f, 0.73f, 0.0f, 1.0f);
		break;
	case 3:
		bezierColor = vec4(0.48f, 0.73f, 0.0f, 1.0f);
		break;
	case 4:
		bezierColor = vec4(0.96f, 0.325f, 0.08f, 1.0f);
		break;
	}

	fogFactor = clamp(fogFactor, 0.0f, 1.0f);
	if (u_fog)
		FragColor = mix(fogColor, bezierColor, fogFactor);
	else
		FragColor = bezierColor;
}
