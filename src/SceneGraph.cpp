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
 * Scene Graph Manager.
 **/

#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstdio>

#include "SceneGraph.h"
#include "Window.h"

Node::~Node() {}

Transform::Transform(const glm::mat4 &mtx) : m_tMtx(mtx) {}

Transform::~Transform()
{
	glDeleteVertexArrays(1, &m_bboxVAO);
	glDeleteVertexArrays(1, &m_snakeVAO);

	glDeleteBuffers(1, &m_bboxVBO);
	glDeleteBuffers(1, &m_snakeVBO);
}

void Transform::addChild(Node *child)
{
	m_ptrs.push_back(child);
}

// untested: don't use
void Transform::removeChild()
{
	m_ptrs.pop_back();
}

void Transform::generateBoundingBox()
{
	float xMin = m_position.x;
	float xMax = m_position.x + m_size.x;
	float yMin = m_position.y - m_size.y;
	float yMax = m_position.y;
	float zMin = m_position.z;
	float zMax = m_position.z + m_size.z;

	// The 8 vertices of the bounding box
	glm::vec3 v1(xMin, yMin, zMin);
	glm::vec3 v2(xMax, yMin, zMin);
	glm::vec3 v3(xMax, yMax, zMin);
	glm::vec3 v4(xMin, yMax, zMin);
	glm::vec3 v5(xMin, yMin, zMax);
	glm::vec3 v6(xMax, yMin, zMax);
	glm::vec3 v7(xMax, yMax, zMax);
	glm::vec3 v8(xMin, yMax, zMax);

	// Construct the cuboidal bounding box
	m_bboxVertices.clear();
	m_bboxVertices.push_back(v1);	m_bboxVertices.push_back(v2);
	m_bboxVertices.push_back(v2);	m_bboxVertices.push_back(v3);
	m_bboxVertices.push_back(v3);	m_bboxVertices.push_back(v4);
	m_bboxVertices.push_back(v4);	m_bboxVertices.push_back(v1);
	m_bboxVertices.push_back(v1);	m_bboxVertices.push_back(v5);
	m_bboxVertices.push_back(v2);	m_bboxVertices.push_back(v6);
	m_bboxVertices.push_back(v3);	m_bboxVertices.push_back(v7);
	m_bboxVertices.push_back(v4);	m_bboxVertices.push_back(v8);
	m_bboxVertices.push_back(v5);	m_bboxVertices.push_back(v6);
	m_bboxVertices.push_back(v6);	m_bboxVertices.push_back(v7);
	m_bboxVertices.push_back(v7);	m_bboxVertices.push_back(v8);
	m_bboxVertices.push_back(v8);	m_bboxVertices.push_back(v5);

	glGenVertexArrays(1, &m_bboxVAO);
	glGenBuffers(1, &m_bboxVBO);

	glBindVertexArray(m_bboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_bboxVBO);

	glBufferData(GL_ARRAY_BUFFER, m_bboxVertices.size() * sizeof(glm::vec3), &m_bboxVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Transform::drawBoundingBox(const GLuint &shaderProgram, const glm::mat4 &mtx)
{
	glm::mat4 modelView = mtx;
  
	GLuint uProjection = glGetUniformLocation(shaderProgram, "u_projection");
	GLuint uModelView = glGetUniformLocation(shaderProgram, "u_modelView");
	GLuint uCamPos = glGetUniformLocation(shaderProgram, "u_camPos");
	GLuint uDestroyed = glGetUniformLocation(shaderProgram, "u_destroyed");
	GLuint uBBoxColor = glGetUniformLocation(shaderProgram, "u_bboxColor");
	GLuint uFog = glGetUniformLocation(shaderProgram, "u_fog");

	glUniformMatrix4fv(uProjection, 1, GL_FALSE, &Window::m_P[0][0]);
	glUniformMatrix4fv(uModelView, 1, GL_FALSE, &modelView[0][0]);
	glUniform3f(uCamPos, Window::m_camPos.x, Window::m_camPos.y, Window::m_camPos.z);
	glUniform1i(uDestroyed, this->m_destroyed);
	glUniform1i(uBBoxColor, this->m_bboxColor);
	glUniform1i(uFog, Window::m_fog);
  
	glBindVertexArray(m_bboxVAO);
	glLineWidth(1.0f);
	glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_bboxVertices.size()));
	glBindVertexArray(0);
}

void Transform::generateSnakeContour()
{
	static float headPos[4] = { 0.78f, 0.78f, 1.8f, 0.78f };

	for (int i = 0; i < 4; i++)
		headPos[i] += Window::m_velocity;

	// Construct the head contours
	glm::vec3 vh1(-1.0f, headPos[0], 0.01f);
	glm::vec3 vh2( 1.0f, headPos[1], 0.01f);
	glm::vec3 vh3( 0.0f, headPos[2], 0.01f);
	glm::vec3 vh4( 0.0f, headPos[3], 0.76f);

	m_snakeVertices.clear();
	m_snakeVertices.push_back(vh1);		m_snakeVertices.push_back(vh3);
	m_snakeVertices.push_back(vh3);		m_snakeVertices.push_back(vh2);
	m_snakeVertices.push_back(vh1);		m_snakeVertices.push_back(vh4);
	m_snakeVertices.push_back(vh4);		m_snakeVertices.push_back(vh2);

	static float bodyPos[11] = { -0.53f, -0.53f, -0.53f,  0.27f,  0.27f, 0.44f,
								 -1.53f, -1.53f, -1.53f, -2.53f, -2.53f };

	for (int i = 0; i < 11; i++)
		bodyPos[i] += Window::m_velocity;

	// Construct the first body part attached to head
	glm::vec3 v1(-0.5f, bodyPos[0], 0.01f);
	glm::vec3 v2( 0.5f, bodyPos[1], 0.01f);
	glm::vec3 v3( 0.0f, bodyPos[2], 0.51f);
	glm::vec3 v4(-0.5f, bodyPos[3], 0.01f);
	glm::vec3 v5( 0.5f, bodyPos[4], 0.01f);
	glm::vec3 v6( 0.0f, bodyPos[5], 0.51f);

	m_snakeVertices.push_back(v4);		m_snakeVertices.push_back(vh1);
	m_snakeVertices.push_back(v5);		m_snakeVertices.push_back(vh2);

	m_snakeVertices.push_back(v1);		m_snakeVertices.push_back(v3);
	m_snakeVertices.push_back(v3);		m_snakeVertices.push_back(v2);
	m_snakeVertices.push_back(v1);		m_snakeVertices.push_back(v4);
	m_snakeVertices.push_back(v2);		m_snakeVertices.push_back(v5);
	m_snakeVertices.push_back(v4);		m_snakeVertices.push_back(v6);
	m_snakeVertices.push_back(v6);		m_snakeVertices.push_back(v5);

	// Second body part
	glm::vec3 v7(-0.5f, bodyPos[6], 0.01f);
	glm::vec3 v8( 0.5f, bodyPos[7], 0.01f);
	glm::vec3 v9( 0.0f, bodyPos[8], 0.51f);

	m_snakeVertices.push_back(v7);		m_snakeVertices.push_back(v9);
	m_snakeVertices.push_back(v9);		m_snakeVertices.push_back(v8);
	m_snakeVertices.push_back(v7);		m_snakeVertices.push_back(v1);
	m_snakeVertices.push_back(v8);		m_snakeVertices.push_back(v2);

	// Third body part
	glm::vec3 v10(-0.5f, bodyPos[9],  0.01f);
	glm::vec3 v11( 0.5f, bodyPos[10], 0.01f);

	m_snakeVertices.push_back(v10);		m_snakeVertices.push_back(v7);
	m_snakeVertices.push_back(v11);		m_snakeVertices.push_back(v8);

	glGenVertexArrays(1, &m_snakeVAO);
	glGenBuffers(1, &m_snakeVBO);

	glBindVertexArray(m_snakeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_snakeVBO);

	glBufferData(GL_ARRAY_BUFFER, m_snakeVertices.size() * sizeof(glm::vec3), &m_snakeVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Transform::drawSnakeContour(const GLuint &shaderProgram, const glm::mat4 &mtx)
{
	GLuint uProjection = glGetUniformLocation(shaderProgram, "u_projection");
	glUniformMatrix4fv(uProjection, 1, GL_FALSE, &Window::m_P[0][0]);

	glm::mat4 modelView = mtx;
	GLuint uModelView = glGetUniformLocation(shaderProgram, "u_modelView");
	glUniformMatrix4fv(uModelView, 1, GL_FALSE, &modelView[0][0]);

	GLuint uCamPos = glGetUniformLocation(shaderProgram, "u_camPos");
	glUniform3f(uCamPos, Window::m_camPos.x, Window::m_camPos.y, Window::m_camPos.z);

	GLuint uFog = glGetUniformLocation(shaderProgram, "u_fog");
	glUniform1i(uFog, Window::m_fog);

	glBindVertexArray(m_snakeVAO);
	glLineWidth(2.0f);
	glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_snakeVertices.size()));
	glBindVertexArray(0);
}

void Transform::draw(const GLuint &shaderProgram, const glm::mat4 &mtx)
{
	if (m_destroyed)
		return;

	for (const auto &node : m_ptrs)
		node->draw(shaderProgram, mtx * m_tMtx);
}

void Transform::update(const glm::mat4 &mtx)
{
	m_tMtx = mtx;
}

void Geometry::load(const char *fileName)
{
	std::ifstream in(fileName);
	if (!in.is_open())
	{
		std::cerr << "Error loading file " << fileName << std::endl;
		exit(EXIT_FAILURE);
	}

	std::string line, next;
	while (getline(in, line))
	{
		// normals
		if (line[0] == 'v' && line[1] == 'n')
		{
			std::istringstream ss(line);
			std::vector<std::string> tokens;

			while (ss)
			{
				if (!getline(ss, next, ' ') || tokens.size() == 4)
					break;

				tokens.push_back(next);
			}

			float n1 = static_cast<float>(atof(tokens[1].c_str()));
			float n2 = static_cast<float>(atof(tokens[2].c_str()));
			float n3 = static_cast<float>(atof(tokens[3].c_str()));

			float mag = sqrtf(pow(n1, 2.0f) + pow(n2, 2.0f) + pow(n3, 2.0f));
			n1 = (n1 / mag) * 0.5f + 0.5f;
			n2 = (n2 / mag) * 0.5f + 0.5f;
			n3 = (n3 / mag) * 0.5f + 0.5f;

			// Populate normals
			m_normals.push_back(n1);
			m_normals.push_back(n2);
			m_normals.push_back(n3);
		}

		// vertices
		else if (line[0] == 'v' && line[1] == ' ')
		{
			std::istringstream ss(line);
			std::vector<std::string> tokens;

			while (ss)
			{
				if (!getline(ss, next, ' ') || tokens.size() == 4)
					break;

				tokens.push_back(next);
			}

			// Populate vertices
			float val = static_cast<float>(atof(tokens[1].c_str()));
			m_vertices.push_back(val);

			val = static_cast<float>(atof(tokens[2].c_str()));
			m_vertices.push_back(val);

			val = static_cast<float>(atof(tokens[3].c_str()));
			m_vertices.push_back(val);
		}

		// faces
		else if (line[0] == 'f')
		{
			std::istringstream ss(line);
			std::vector<std::string> tokens;

			while (ss)
			{
				if (!getline(ss, next, ' ') || tokens.size() == 4)
					break;

				tokens.push_back( next );
			}

			for (int i = 1; i < 4; i++)
			{
				size_t pos = tokens[i].find("//");
				int index = atoi((tokens[i].substr(0, pos)).c_str()) - 1;

				// Populate face-indices
				m_indices.push_back(index);
			}
		}
	}

	in.close();
}

Geometry::Geometry(const char *fileName)
{
	// parse and load the obj file
	load(fileName);

	glGenVertexArrays(1, &m_VAO);

	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_EBO);
	glGenBuffers(1, &m_NBO);

	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(GLfloat), &m_vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)0);

	glBindBuffer(GL_ARRAY_BUFFER, m_NBO);
	glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(GLfloat), &m_normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(GLuint), &m_indices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

Geometry::~Geometry()
{
	glDeleteVertexArrays(1, &m_VAO);

	glDeleteBuffers(1, &m_VBO);
	glDeleteBuffers(1, &m_NBO);
	glDeleteBuffers(1, &m_EBO);
}

void Geometry::draw(const GLuint &shaderProgram, const glm::mat4 &mtx)
{
	glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.direction"), 0.0f, 1.0f, 1.0f);
	glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.ambient"), 0.2f, 0.2f, 0.2f);
	glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.diffuse"), 1.0f, 1.0f, 1.0f);
	glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.specular"), 0.3f, 0.3f, 0.3f);

	glUniform3f(glGetUniformLocation(shaderProgram, "dirLight2.direction"), 0.0f, 0.1f, 1.2f);
	glUniform3f(glGetUniformLocation(shaderProgram, "dirLight2.ambient"), 0.8f, 0.8f, 0.8f);
	glUniform3f(glGetUniformLocation(shaderProgram, "dirLight2.diffuse"), 0.6f, 0.6f, 0.6f);
	glUniform3f(glGetUniformLocation(shaderProgram, "dirLight2.specular"), 0.7f, 0.7f, 0.7f);

	GLuint uProjection = glGetUniformLocation(shaderProgram, "u_projection");
	glUniformMatrix4fv(uProjection, 1, GL_FALSE, &Window::m_P[0][0]);

	glm::mat4 modelView = mtx;
	GLuint uModelView = glGetUniformLocation(shaderProgram, "u_modelView");
	glUniformMatrix4fv(uModelView, 1, GL_FALSE, &modelView[0][0]);

	GLuint uCamPos = glGetUniformLocation(shaderProgram, "u_camPos");
	glUniform3f(uCamPos, Window::m_camPos.x, Window::m_camPos.y, Window::m_camPos.z);

	GLuint uObstacleType = glGetUniformLocation(shaderProgram, "u_obstacleType");
	glUniform1i(uObstacleType, this->m_obstacleType);

	GLuint uFog = glGetUniformLocation(shaderProgram, "u_fog");
	glUniform1i(uFog, Window::m_fog);

	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size() * sizeof(GLuint)), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

void Geometry::update(const glm::mat4 &mtx) {}
