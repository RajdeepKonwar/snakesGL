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

Bezier::Bezier(const glm::vec3 points[16])
{
	m_G[0] = glm::mat4( glm::vec4( points[0].x, points[4].x, points[8].x,  points[12].x ),
						glm::vec4( points[1].x, points[5].x, points[9].x,  points[13].x ),
						glm::vec4( points[2].x, points[6].x, points[10].x, points[14].x ),
						glm::vec4( points[3].x, points[7].x, points[11].x, points[15].x ) );

	m_G[1] = glm::mat4( glm::vec4( points[0].y, points[4].y, points[8].y,  points[12].y ),
						glm::vec4( points[1].y, points[5].y, points[9].y,  points[13].y ),
						glm::vec4( points[2].y, points[6].y, points[10].y, points[14].y ),
						glm::vec4( points[3].y, points[7].y, points[11].y, points[15].y ) );

	m_G[2] = glm::mat4( glm::vec4( points[0].z, points[4].z, points[8].z,  points[12].z ),
						glm::vec4( points[1].z, points[5].z, points[9].z,  points[13].z ),
						glm::vec4( points[2].z, points[6].z, points[10].z, points[14].z ),
						glm::vec4( points[3].z, points[7].z, points[11].z, points[15].z ) );

	m_B = glm::mat4(glm::vec4( -1.0f,  3.0f, -3.0f, 1.0f ),
					glm::vec4(  3.0f, -6.0f,  3.0f, 0.0f ),
					glm::vec4( -3.0f,  3.0f,  0.0f, 0.0f ),
					glm::vec4(  1.0f,  0.0f,  0.0f, 0.0f ));

	m_C[0] = m_B * m_G[0] * m_B;
	m_C[1] = m_B * m_G[1] * m_B;
	m_C[2] = m_B * m_G[2] * m_B;

	int counter = 0;
	int rows = 0;

	float u = 0.0f;
	float v = 0.0f;

	while (rows <= 100)
	{
		while (counter < 202)
		{
			glm::vec4 uVector = glm::vec4(u * u * u, u * u, u, 1);
			glm::vec4 vVector = glm::vec4(v * v * v, v * v, v, 1);

			glm::vec3 xOfuv	= glm::vec3(glm::dot(vVector, m_C[0] * uVector),
										glm::dot(vVector, m_C[1] * uVector),
										glm::dot(vVector, m_C[2] * uVector));

			m_vertices[rows].push_back(xOfuv);

			if (counter % 2 == 0)
				u += 0.01f;
			else
			{
				u -= 0.01f;
				v += 0.01f;
			}

			counter++;
		}

		counter = 0;
		v = 0.0f;
		u = 0.0f + rows * 0.01f;
		rows++;
	}

	for (int j = 0; j <= 100; j++)
	{
		glGenVertexArrays(1, &m_VAO[j]);
		glGenBuffers(1, &m_VBO[j]);

		glBindVertexArray(m_VAO[j]);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO[j]);
		glBufferData(GL_ARRAY_BUFFER, m_vertices[j].size() * sizeof(glm::vec3), &m_vertices[j][0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
}

void Bezier::draw(const GLuint &shaderProgram)
{
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &Window::m_P[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelView"), 1, GL_FALSE, &Window::m_V[0][0]);
	glUniform1i(glGetUniformLocation(shaderProgram, "u_surface"), m_surface);

	GLuint uFog = glGetUniformLocation(shaderProgram, "u_fog");
	glUniform1i(uFog, Window::m_fog);

	GLuint uCamPos = glGetUniformLocation(shaderProgram, "u_camPos");
	glUniform3f(uCamPos, Window::m_camPos.x, Window::m_camPos.y, Window::m_camPos.z);

	for (int i = 0; i <= 100; i++)
	{
		glBindVertexArray(m_VAO[i]);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, static_cast<GLsizei>(m_vertices[i].size()));
		glBindVertexArray(0);
	}
}
