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
 * Window, scene and objects manager.
 **/

#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include <OpenGL/glext.h>
#include <OpenGL/gl.h> // Remove this line in future projects
#else
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

// Use of degrees is deprecated. Use radians instead.
#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <list>
#include <vector>

// Abstract node class
class Node
{
public:
	// Pure virtual destructor
	virtual ~Node() = 0;

	// Pure virtual functions
	virtual void draw(const GLuint &shaderProgram, const glm::mat4 &mtx) = 0;
	virtual void update(const glm::mat4 &mtx) = 0;
};

// Derived transform class
class Transform : public Node
{
public:
	Transform(const glm::mat4 &mtx);
	~Transform();

	void addChild(Node *child);
	void removeChild();

	void generateBoundingBox();
	void drawBoundingBox(const GLuint &shaderProgram, const glm::mat4 &mtx);

	void generateSnakeContour();
	void drawSnakeContour(const GLuint &shaderProgram, const glm::mat4 &mtx);

	void draw(const GLuint &shaderProgram, const glm::mat4 &mtx);
	void update(const glm::mat4 &mtx);

public:
	bool m_destroyed = false;
	int m_bboxColor = 2;			// 1 for white, 2 for green, 3 for red
	int m_type = 0;					// 0 for head, 1 for pyramid, 2 for coin, 3 for wall
	glm::vec3 m_position, m_size;

private:
	GLuint					m_bboxVAO, m_bboxVBO, m_snakeVAO, m_snakeVBO;
	glm::mat4				m_tMtx;
	std::list<Node *>		m_ptrs;
	std::vector<glm::vec3>	m_bboxVertices, m_snakeVertices;
};

// derived Geometry class
class Geometry : public Node
{
public:
	Geometry(const char *fileName);
	~Geometry();

	void draw(const GLuint &shaderProgram, const glm::mat4 &mtx);
	void update(const glm::mat4 &mtx);

private:
	void load(const char *fileName);

public:
	int m_obstacleType = 1;		// 1 for pyramid, 2 for coin, 3 for wall

private:
	GLuint					m_VAO, m_VBO, m_NBO, m_EBO;
	std::vector<GLfloat>	m_vertices, m_normals;
	std::vector<GLuint>		m_indices;
};

#endif
