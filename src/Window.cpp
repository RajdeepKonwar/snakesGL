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

#include <fstream>

#ifdef _WIN32
#include <string>
#endif

#include "Window.h"

constexpr auto WINDOW_TITLE = "snakesGL";
constexpr auto CONFIG_FILE = "./snakesGL.conf";

#ifdef __APPLE__
constexpr float SNAKE_SPEED = 0.05f;
#else
constexpr float SNAKE_SPEED = 0.03f;
#endif

constexpr float SPEED_INC = 0.02f;

// Static data members
int Window::m_width;
int Window::m_height;
int Window::m_move = 0;
int Window::m_nBody = 3;
int Window::m_nTile = 20;
bool Window::m_fog = true;

// Global variables
GLuint G_gridBigShader, G_gridSmallShader, G_snakeShader, G_obstaclesShader;
GLuint G_boundingBoxShader, G_snakeContourShader, G_velocityShader, G_bezierShader;

// Overclocking on apple to get better fps lul
#ifdef __APPLE__
float Window::m_velocity  = SNAKE_SPEED_APPLE;
#else
float Window::m_velocity  = SNAKE_SPEED;
#endif

float G_yPos = 0.0f;
bool G_drawBbox = false;
float G_rotAngle = 0.0f;
int G_nPyramids = 80;
int G_nCoins = 5;
int G_nWalls = 60;

Node *G_pGridBig, *G_pGridSmall;	// Big and small grid position transform mtx
Node *G_pSnake;						// Snake transform mtx
Node *G_pObstacles;

// Individual elements' transform mtx
Node *G_pHeadMtx, *G_pTailMtx, **G_pPyramidMtx, **G_pCoinMtx, **G_pWallMtx;
std::vector<Node *> G_pTileBigPos, G_pTileSmallPos, G_pBodyMtx, G_pObstaclesList;
std::vector<Node *>::iterator G_nodeIt;

Node *G_pHead, *G_pBody, *G_pTail, *G_pTileBig, *G_pTileSmall, *G_pCoin, *G_pWall;

Bezier *patch[4];

// Default camera parameters
//glm::vec3 Window::m_camPos(0.0f, 1.8f, 5.0f);		// e | Position of camera (top)
glm::vec3 Window::m_camPos(0.0f, -3.0f, 3.5f);		// e | Position of camera
glm::vec3 G_camLookAt(0.0f, 2.5f, 0.0f);			// d | Where camera looks at
glm::vec3 g_camUp(0.0f, 1.0f, 0.0f);				// u | What orientation "up" is

glm::vec3 Window::m_lastPoint(0.0f, 0.0f, 0.0f);	// For mouse tracking
glm::mat4 Window::m_P;
glm::mat4 Window::m_V;

float Window::randGenX()
{
	int l_randMax =  12;
	int l_randMin = -12;
	int l_rand = rand() % (l_randMax - l_randMin + 1) + l_randMin;

	return (2.0f * static_cast<float>(l_rand));
}

float Window::randGenY()
{
	int l_randMax = Window::m_nTile;
	int l_randMin = 2;
	int l_rand = rand() % (l_randMax - l_randMin + 1) + l_randMin;

	return (2.0f * static_cast<float>(l_rand));
}

// functions as constructor
void Window::initializeObjects()
{
	// Seed the randomizer
	srand(static_cast<unsigned int>(time(nullptr)));

	// Parse config file for shader and obj paths
	std::ifstream confFn(CONFIG_FILE, std::ios::in);
	if (!confFn.is_open())
	{
		std::cerr << "Error: cannot open " << CONFIG_FILE << std::endl;
		exit(EXIT_FAILURE);
	}

	std::string lineBuf;
	std::string gridBigVertShader,		gridBigFragShader;
	std::string gridSmallVertShader,	gridSmallFragShader;
	std::string snakeVertShader,		snakeFragShader;
	std::string obstaclesVertShader,	obstaclesFragShader;
	std::string boundingBoxVertShader,	boundingBoxFragShader;
	std::string snakeContourVertShader,	snakeContourFragShader;
	std::string bezierVertShader,		bezierFragShader;
	std::string head, body, tail, tileBig, tileSmall, coin, wall;

	while (getline(confFn, lineBuf))
	{
		size_t k = -1, l;

		while (++k < lineBuf.length() && lineBuf[k] == ' ');

		if (k >= lineBuf.length() || lineBuf[k] == '#')
			continue;

		l = k - 1;

		while (++l < lineBuf.length() && lineBuf[l] != '=');

		if (l >= lineBuf.length())
			continue;

		std::string varName = lineBuf.substr(k, l - k);
		std::string varValue = lineBuf.substr(l + 1);

		if (!varName.compare("grid_big_vert_shader"))
			gridBigVertShader = varValue;
		else if (!varName.compare("grid_big_frag_shader"))
			gridBigFragShader = varValue;
		else if (!varName.compare("grid_small_vert_shader"))
			gridSmallVertShader = varValue;
		else if (!varName.compare("grid_small_frag_shader"))
			gridSmallFragShader = varValue;
		else if (!varName.compare("snake_vert_shader"))
			snakeVertShader = varValue;
		else if (!varName.compare("snake_frag_shader"))
			snakeFragShader = varValue;
		else if (!varName.compare("obstacles_vert_shader"))
			obstaclesVertShader = varValue;
		else if (!varName.compare("obstacles_frag_shader"))
			obstaclesFragShader = varValue;
		else if (!varName.compare("bounding_box_vert_shader"))
			boundingBoxVertShader = varValue;
		else if (!varName.compare("bounding_box_frag_shader"))
			boundingBoxFragShader = varValue;
		else if (!varName.compare("snake_contour_vert_shader"))
			snakeContourVertShader = varValue;
		else if (!varName.compare("snake_contour_frag_shader"))
			snakeContourFragShader = varValue;
		else if (!varName.compare("bezier_vert_shader"))
			bezierVertShader = varValue;
		else if (!varName.compare("bezier_frag_shader"))
			bezierFragShader = varValue;

		else if (!varName.compare("head"))
			head = varValue;
		else if (!varName.compare("body"))
			body = varValue;
		else if (!varName.compare("tail"))
			tail = varValue;
		else if (!varName.compare("tile_big"))
			tileBig = varValue;
		else if (!varName.compare("tile_small"))
			tileSmall = varValue;
		else if (!varName.compare("coin"))
			coin = varValue;
		else if (!varName.compare("wall"))
			wall = varValue;
		else
			std::cout << "\nUnknown setting (" << varName << "). Ignored.\n";
	}

	confFn.close();

	// Geometry nodes
	G_pHead = new Geometry(head.c_str());
	G_pBody = new Geometry(body.c_str());
	G_pTail = new Geometry(tail.c_str());
	G_pTileSmall = new Geometry(tileSmall.c_str());
	G_pTileBig = new Geometry(tileBig.c_str());
	G_pCoin = new Geometry(coin.c_str());
	G_pWall = new Geometry(wall.c_str());

	// Set geometry obstacle type (for color, 1 by default)
	static_cast<Geometry *>(G_pCoin)->m_obstacleType = 2;
	static_cast<Geometry *>(G_pWall)->m_obstacleType = 3;

	// Group nodes
	G_pGridBig = new Transform(glm::mat4(1.0f));
	G_pGridSmall = new Transform(glm::mat4(1.0f));
	G_pSnake = new Transform(glm::mat4(1.0f));
	G_pObstacles = new Transform(glm::mat4(1.0f));

	// Transform modes
	G_pHeadMtx = new Transform(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.8f, 0.0f)));

	// Head's bounding box is white
	static_cast<Transform *>(G_pHeadMtx)->m_bboxColor = 1;

	// Bounding boxes' initial positions and sizes
	static_cast<Transform *>(G_pHeadMtx)->m_position = glm::vec3(-1.0f, 1.8f, 0.01f);
	static_cast<Transform *>(G_pHeadMtx)->m_size = glm::vec3(2.0f, 1.5f, 0.75f);

	// Add head to snake
	static_cast<Transform *>(G_pSnake)->addChild(G_pHeadMtx);
	static_cast<Transform *>(G_pHeadMtx)->addChild(G_pHead);

	// Add head to obstacles list as first item (for collision detection)
	G_pObstaclesList.push_back(G_pHeadMtx);

	// Snake body parts' transform mtx
	for (int i = 0; i < Window::m_nBody; i++)
		G_pBodyMtx.push_back(new Transform(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f * static_cast<float>(i), 0.0f))));

	// Add body parts to snake
	for (G_nodeIt = G_pBodyMtx.begin(); G_nodeIt != G_pBodyMtx.end(); ++G_nodeIt)
	{
		static_cast<Transform *>(G_pSnake)->addChild(*G_nodeIt);
		static_cast<Transform *>(*G_nodeIt)->addChild(G_pBody);
	}

	// Tail transform mtx
	G_pTailMtx = new Transform(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f * static_cast<float>(Window::m_nBody) + 0.5f, 0.0f)));

	// Add tail to snake
	static_cast<Transform *>(G_pSnake)->addChild(G_pTailMtx);
	static_cast<Transform *>(G_pTailMtx)->addChild(G_pTail);

	// Initialize snake contour (white)
	static_cast<Transform *>(G_pSnake)->generateSnakeContour();

	float randX, randY;

	// Pyramids transform mtx
	G_pPyramidMtx = new Node *[G_nPyramids];
	for (int k = 0; k < G_nPyramids; k++)
	{
		do
		{
			randX = randGenX();
			randY = randGenY();
		} while (randY >= 12.0f && randY <= 14.0f);

		// Reuse head (rotated by 45) as pyramid obstacle
		glm::mat4 l_moveRotMtx = glm::translate(glm::mat4(1.0f), glm::vec3(static_cast<float>(randX), static_cast<float>(randY), 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		G_pPyramidMtx[k] = new Transform( l_moveRotMtx );

		// Type for collision detection
		static_cast<Transform *>(G_pPyramidMtx[k])->m_type = 1;

		// Bounding boxes' initial positions and sizes
		static_cast<Transform *>(G_pPyramidMtx[k])->m_position = glm::vec3(-0.7f + static_cast<float>(randX), 0.7f + static_cast<float>(randY), 0.01f);
		static_cast<Transform *>(G_pPyramidMtx[k])->m_size = glm::vec3(1.4f, 1.4f, 0.75f);

		// Add pyramid as child of obstacles
		static_cast<Transform *>(G_pObstacles)->addChild(G_pPyramidMtx[k]);
		static_cast<Transform *>(G_pPyramidMtx[k])->addChild(G_pHead);

		// Add to obstacles list (for collision detection)
		G_pObstaclesList.push_back(G_pPyramidMtx[k]);
	}

	// Coins transform mtx
	G_pCoinMtx = new Node *[G_nCoins];
	for (int k = 0; k < G_nCoins; k++)
	{
		do
		{
			randX = randGenX();
			randY = randGenY();
		} while (randY >= 12.0f && randY <= 14.0f);

		G_pCoinMtx[k] = new Transform(glm::translate(glm::mat4(1.0f), glm::vec3(static_cast<float>(randX), static_cast<float>(randY), 0.0f)));

		// Type for collision detection
		static_cast<Transform *>(G_pCoinMtx[k])->m_type = 2;

		// Bounding boxes' initial positions and sizes
		static_cast<Transform *>(G_pCoinMtx[k])->m_position = glm::vec3(-0.5f + static_cast<float>(randX), 0.1f + static_cast<float>(randY), 0.1726f);
		static_cast<Transform *>(G_pCoinMtx[k])->m_size = glm::vec3(1.0f, 0.2f, 1.15f);

		// Add coin as child of obstacles
		static_cast<Transform *>(G_pObstacles)->addChild(G_pCoinMtx[k]);
		static_cast<Transform *>(G_pCoinMtx[k])->addChild(G_pCoin);

		// Add to obstacles list (for collision detection)
		G_pObstaclesList.push_back(G_pCoinMtx[k]);
	}

	// Walls transform mtx
	G_pWallMtx = new Node *[G_nWalls + 1];
	for (int k = 0; k < G_nWalls; k++)
	{
		do
		{
			randX = randGenX();
			randY = randGenY();
		} while (randX == 0.0f || (randY >= 12.0f && randY <= 14.0f));

		G_pWallMtx[k] = new Transform(glm::translate(glm::mat4(1.0f), glm::vec3(static_cast<float>(randX), static_cast<float>(randY), 0.0f)));

		// Type for collision detection
		static_cast<Transform *>(G_pWallMtx[k])->m_type = 3;

		// Bounding boxes' initial positions and sizes
		static_cast<Transform *>(G_pWallMtx[k])->m_position = glm::vec3(-0.7f + static_cast<float>(randX), 0.7f + static_cast<float>(randY), 0.01f);
		static_cast<Transform *>(G_pWallMtx[k])->m_size = glm::vec3(1.4f, 1.4f, 1.0f);

		// Add wall as child of obstacles
		static_cast<Transform *>(G_pObstacles)->addChild(G_pWallMtx[k]);
		static_cast<Transform *>(G_pWallMtx[k])->addChild(G_pWall);

		// Add to obstacles list (for collision detection)
		G_pObstaclesList.push_back(G_pWallMtx[k]);
	}

	// Last wall in way to stop motion
	G_pWallMtx[G_nWalls] = new Transform(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2 * Window::m_nTile, 0.0f)));

	// Type for collision detection
	static_cast<Transform *>(G_pWallMtx[G_nWalls])->m_type = 3;

	// Bounding boxes' initial positions and sizes
	static_cast<Transform *>(G_pWallMtx[G_nWalls])->m_position = glm::vec3(-0.7f, 0.7f + 2 * Window::m_nTile, 0.01f);
	static_cast<Transform *>(G_pWallMtx[G_nWalls])->m_size = glm::vec3(1.4f, 1.4f, 1.0f);

	// Add wall as child of obstacles
	static_cast<Transform *>(G_pObstacles)->addChild(G_pWallMtx[G_nWalls]);
	static_cast<Transform *>(G_pWallMtx[G_nWalls])->addChild(G_pWall);

	// Add to obstacles list (for collision detection)
	G_pObstaclesList.push_back(G_pWallMtx[G_nWalls]);

	// Initialize obstacles' bounding boxes
	for (G_nodeIt = G_pObstaclesList.begin(); G_nodeIt != G_pObstaclesList.end(); ++G_nodeIt)
		static_cast<Transform *>(*G_nodeIt)->generateBoundingBox();

	// Arrange tiles to form grid
	for (int i = -1; i <= Window::m_nTile; i++)
	{
		for( int j = -8; j <= 8; j++ )
		{
			G_pTileBigPos.push_back(new Transform(glm::translate(glm::mat4(1.0f), glm::vec3(j * 2.0f, i * 2.0f, -0.1f))));
			G_pTileSmallPos.push_back(new Transform(glm::translate(glm::mat4(1.0f), glm::vec3(j * 2.0f, i * 2.0f, 0.0f))));
		}
	}

	// Add big tiles to big grid group
	for (G_nodeIt = G_pTileBigPos.begin(); G_nodeIt != G_pTileBigPos.end(); ++G_nodeIt)
	{
		static_cast<Transform *>(G_pGridBig)->addChild(*G_nodeIt);
		static_cast<Transform *>(*G_nodeIt)->addChild(G_pTileBig);
	}

	// Add small tiles to small grid group
	for (G_nodeIt = G_pTileSmallPos.begin(); G_nodeIt != G_pTileSmallPos.end(); ++G_nodeIt)
	{
		static_cast<Transform *>(G_pGridSmall)->addChild(*G_nodeIt);
		static_cast<Transform *>(*G_nodeIt)->addChild(G_pTileSmall);
	}

	// Bezier surface 1 control points
	glm::vec3 points0[16] = {	glm::vec3( -4,   12.50, 0.25 ),   // p0
								glm::vec3( -3.5, 13.0,  0.25 ),
								glm::vec3( -3,   13.0,  0.25 ),
								glm::vec3( -2.5, 12.50, 0.25 ),   // p3
								glm::vec3( -4,   12.25, 0.75 ),
								glm::vec3( -3.5, 13.50, 0.75 ),
								glm::vec3( -3,   13.50, 0.75 ),
								glm::vec3( -2.5, 12.25, 0.75 ),   // p7
								glm::vec3( -4,   13.0,  1.25 ),
								glm::vec3( -3.5, 12.50, 1.25 ),
								glm::vec3( -3,   12.50, 1.25 ),
								glm::vec3( -2.5, 13.0,  1.25 ),   // p11
								glm::vec3( -4,   12.50, 1.75 ),
								glm::vec3( -3.5, 12.0,  1.75 ),
								glm::vec3( -3,   12.0,  1.75 ),
								glm::vec3( -2.5, 12.50, 1.75 ),   // p15
							};

	// Bezier surface 2 control points
	glm::vec3 points1[16] = {	glm::vec3( -2.5, 12.50, 0.25 ),   // p0
								glm::vec3( -2,   12.0,  0.25 ),
								glm::vec3( -1.5, 13.0,  0.25 ),
								glm::vec3( -1,   12.50, 0.25 ),   // p3
								glm::vec3( -2.5, 12.25, 0.75 ),
								glm::vec3( -2,   11.0,  0.75 ),
								glm::vec3( -1.5, 13.50, 0.75 ),
								glm::vec3( -1,   12.25, 0.75 ),   // p7
								glm::vec3( -2.5, 13.0,  1.25 ),
								glm::vec3( -2,   13.50, 1.25 ),
								glm::vec3( -1.5, 12.50, 1.25 ),
								glm::vec3( -1,   13.0,  1.25 ),   // p11
								glm::vec3( -2.5, 12.50, 1.75 ),
								glm::vec3( -2,   13.0,  1.75 ),
								glm::vec3( -1.5, 12.0,  1.75 ),
								glm::vec3( -1,   12.50, 1.75 ),   // p15
							};

	// Bezier surface 3 control points
	glm::vec3 points2[16] = {	glm::vec3( -2.5, 12.50, 1.75 ),   // p0
								glm::vec3( -2,   13.0,  1.75 ),
								glm::vec3( -1.5, 12.0,  1.75 ),
								glm::vec3( -1,   12.50, 1.75 ),   // p3
								glm::vec3( -2.5, 12.0,  2.25 ),
								glm::vec3( -2,   12.5,  2.25 ),
								glm::vec3( -1.5, 11.50, 2.25 ),
								glm::vec3( -1,   12.0,  2.25 ),   // p7
								glm::vec3( -2.5, 13.0,  2.75 ),
								glm::vec3( -2,   13.50, 2.75 ),
								glm::vec3( -1.5, 12.50, 2.75 ),
								glm::vec3( -1,   13.0,  2.75 ),   // p11
								glm::vec3( -2.5, 12.50, 3.25 ),
								glm::vec3( -2,   13.0,  3.25 ),
								glm::vec3( -1.5, 12.0,  3.25 ),
								glm::vec3( -1,   12.50, 3.25 ),   // p15
							};

	// Bezier surface 4 control points
	glm::vec3 points3[16] = {	glm::vec3( -4,   12.50, 1.75 ),   // p0
								glm::vec3( -3.5, 12.0,  1.75 ),
								glm::vec3( -3,   12.0,  1.75 ),
								glm::vec3( -2.5, 12.50, 1.75 ),   // p3
								glm::vec3( -4,   12.0,  2.25 ),
								glm::vec3( -3.5, 12.5,  2.25 ),
								glm::vec3( -3,   11.50, 2.25 ),
								glm::vec3( -2.5, 12.0,  2.25 ),   // p7
								glm::vec3( -4,   13.0,  2.75 ),
								glm::vec3( -3.5, 13.50, 2.75 ),
								glm::vec3( -3,   12.50, 2.75 ),
								glm::vec3( -2.5, 13.0,  2.75 ),   // p11
								glm::vec3( -4,   12.50, 3.25 ),
								glm::vec3( -3.5, 13.0,  3.25 ),
								glm::vec3( -3,   12.0,  3.25 ),
								glm::vec3( -2.5, 12.50, 3.25 ),   // p15
							};

	// Create 4 Bezier patches (C0 and C1 continuous)
	patch[0] = new Bezier(points0);
	patch[1] = new Bezier(points1);
	patch[2] = new Bezier(points2);
	patch[3] = new Bezier(points3);

	// Surface color info
	for (int i = 0; i < 4; i++)
		patch[i]->m_surface = i + 1;

	// Load the shader programs
	G_gridBigShader = LoadShaders(gridBigVertShader.c_str(), gridBigFragShader.c_str());
	G_gridSmallShader = LoadShaders(gridSmallVertShader.c_str(), gridSmallFragShader.c_str());
	G_snakeShader = LoadShaders(snakeVertShader.c_str(), snakeFragShader.c_str());
	G_obstaclesShader = LoadShaders(obstaclesVertShader.c_str(), obstaclesFragShader.c_str());
	G_boundingBoxShader = LoadShaders(boundingBoxVertShader.c_str(), boundingBoxFragShader.c_str());
	G_snakeContourShader = LoadShaders(snakeContourVertShader.c_str(), snakeContourFragShader.c_str());
	G_bezierShader = LoadShaders(bezierVertShader.c_str(), bezierFragShader.c_str());
}

// Treat this as a destructor function. Delete dynamically allocated memory here.
void Window::cleanUp()
{
	delete G_pSnake;
	delete G_pGridBig;
	delete G_pGridSmall;
	delete G_pObstacles;
	delete G_pHeadMtx;
	delete G_pTailMtx;

	delete G_pPyramidMtx;
	delete G_pCoinMtx;
	delete G_pWallMtx;

	for (G_nodeIt = G_pTileBigPos.begin(); G_nodeIt != G_pTileBigPos.end(); ++G_nodeIt)
		delete *G_nodeIt;
	for (G_nodeIt = G_pTileSmallPos.begin(); G_nodeIt != G_pTileSmallPos.end(); ++G_nodeIt)
		delete *G_nodeIt;
	for (G_nodeIt = G_pBodyMtx.begin(); G_nodeIt != G_pBodyMtx.end(); ++G_nodeIt)
		delete *G_nodeIt;

	delete G_pHead;
	delete G_pBody;
	delete G_pTail;
	delete G_pTileBig;
	delete G_pTileSmall;
	delete G_pCoin;
	delete G_pWall;

	glDeleteProgram(G_gridBigShader);
	glDeleteProgram(G_gridSmallShader);
	glDeleteProgram(G_snakeShader);
	glDeleteProgram(G_obstaclesShader);
	glDeleteProgram(G_boundingBoxShader);
	glDeleteProgram(G_snakeContourShader);
	glDeleteProgram(G_bezierShader);
}

// Since everything is on the grid, no need of collision-check in z-direction
bool Window::checkCollision(Node *first, Node *second)
{
	bool collisionX = static_cast<Transform *>(first)->m_position.x + static_cast<Transform *>(first)->m_size.x >= static_cast<Transform *>(second)->m_position.x &&
					  static_cast<Transform *>(second)->m_position.x + static_cast<Transform *>(second)->m_size.x >= static_cast<Transform *>(first)->m_position.x;

	bool collisionY = static_cast<Transform *>(first)->m_position.y + static_cast<Transform *>(first)->m_size.y >= static_cast<Transform *>(second)->m_position.y &&
					  static_cast<Transform *>(second)->m_position.y + static_cast<Transform *>(second)->m_size.y >= static_cast<Transform *>(first)->m_position.y;

	return collisionX && collisionY;
}

// Perform inter-object collision-checks
void Window::performCollisions()
{
	for (std::vector<Node *>::iterator it = G_pObstaclesList.begin() + 1; it != G_pObstaclesList.end(); ++it)
	{
		// Only check for undestroyed obstacles
		if (!static_cast<Transform *>(*it)->m_destroyed)
		{
			// Check each obstacle wrt head
			if (checkCollision(G_pHeadMtx, *it))
			{
				/** Collision with wall
					*  Set both head and wall bbox to red, stop motion of snake
					**/
				if (static_cast<Transform *>(*it)->m_type == 3)
				{
					static_cast<Transform *>(*it)->m_bboxColor = 3;
					static_cast<Transform *>(G_pHeadMtx)->m_bboxColor = 3;
					Window::m_velocity = 0.0f;
				}

				// Set obstacle's bbox to red and destroy it (don't display)
				else
				{
					static_cast<Transform *>(*it)->m_bboxColor = 3;
					static_cast<Transform *>(*it)->m_destroyed = true;
				}
			}
		}
	}
}

GLFWwindow* Window::createWindow(int width, int height)
{
	// Initialize GLFW
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW\n";
		return nullptr;
	}

	// 4x antialiasing
	glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__ // Because Apple hates comforming to standards
	// Ensure that minimum OpenGL version is 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	// Enable forward compatibility and allow a modern OpenGL context
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// Create the GLFW window
	GLFWwindow *window = glfwCreateWindow(width, height, WINDOW_TITLE, nullptr, nullptr);

	// Check if the window could not be created
	if (!window)
	{
		std::cerr << "Failed to open GLFW window.\n";
		std::cerr << "Either GLFW is not installed or your graphics card does not support modern OpenGL.\n";
		glfwTerminate();
		return nullptr;
	}

	// Make the context of the window
	glfwMakeContextCurrent(window);

	// Set swap interval to 1
	glfwSwapInterval(1);

	// Get the width and height of the framebuffer to properly resize the window
	glfwGetFramebufferSize(window, &width, &height);

	// Call the resize callback to make sure things get drawn immediately
	Window::resizeCallback(window, width, height);

	return window;
}

void Window::displayCallback(GLFWwindow *window)
{
	// Clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Using GridBigShader, draw black background
	glUseProgram(G_gridBigShader);
	G_pGridBig->draw(G_gridBigShader, Window::m_V);

	// Using GridSmallShader, draw small white tiles
	glUseProgram(G_gridSmallShader);
	G_pGridSmall->draw(G_gridSmallShader, Window::m_V);

	// Using SnakeShader, draw snake
	glUseProgram(G_snakeShader);
	G_pSnake->draw(G_snakeShader, Window::m_V);

	// Using SnakeContourShader, draw outline of snake
	glUseProgram(G_snakeContourShader);
	static_cast<Transform *>(G_pSnake)->drawSnakeContour(G_snakeContourShader, Window::m_V);

	// Using ObstaclesShader, draw the obstacles
	glUseProgram(G_obstaclesShader);
	G_pObstacles->draw(G_obstaclesShader, Window::m_V);

	// Using BoundingBoxShader, draw the axis-aligned bounding boxes (AABB)
	glUseProgram(G_boundingBoxShader);
	if (G_drawBbox)
		for (const auto &obstacle : G_pObstaclesList)
			static_cast<Transform *>(obstacle)->drawBoundingBox(G_boundingBoxShader, Window::m_V);

	// Using BezierShader, draw the 4 Bezier surfaces
	glUseProgram(G_bezierShader);
	for (int i = 0; i < 4; i++)
		patch[i]->draw(G_bezierShader);

	// Gets events, including input such as keyboard and mouse or window resizing
	glfwPollEvents();

	// Swap buffers
	glfwSwapBuffers(window);

	// Refresh view matrix with new camera position every display callback
	Window::m_V = glm::lookAt(Window::m_camPos, G_camLookAt, g_camUp);
}

void Window::idleCallback()
{
	// Update coin rotation angle
	if (G_rotAngle >= 360.0f)
		G_rotAngle = 0.0f;
	G_rotAngle += 1.5f;

	// Move snake in y-direction
	glm::mat4 moveMtx = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, G_yPos, 0.0f));
	static_cast<Transform *>(G_pSnake)->update(moveMtx);

	for (int i = 0; i < G_nCoins; i++)
	{
		// Find tile center
		float xCntr = static_cast<Transform *>(G_pCoinMtx[i])->m_position.x + static_cast<Transform *>(G_pCoinMtx[i])->m_size.x / 2.0f;
		float yCntr = static_cast<Transform *>(G_pCoinMtx[i])->m_position.y - static_cast<Transform *>(G_pCoinMtx[i])->m_size.y / 2.0f;

		// Rotate coins
		glm::mat4 rotMtx = glm::translate(glm::mat4(1.0f), glm::vec3(xCntr, yCntr, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(G_rotAngle), glm::vec3(0.0f, 0.0f, -1.0f));
		static_cast<Transform *>(G_pCoinMtx[i])->update(rotMtx);

		// Update coin's bounding box
		static_cast<Transform *>(G_pCoinMtx[i])->m_position.x = static_cast<float>(0.5f * cos(M_PI - glm::radians(G_rotAngle)) + 0.01f);
		static_cast<Transform *>(G_pCoinMtx[i])->m_position.y = static_cast<float>(0.5f * sin(M_PI - glm::radians(G_rotAngle)) + 0.01f);

		// Keep lower left corner such that x is negative and y is positive
		if (static_cast<Transform *>(G_pCoinMtx[i])->m_position.x > 0.0f)
			static_cast<Transform *>(G_pCoinMtx[i])->m_position.x *= -1.0f;
		if (static_cast<Transform *>(G_pCoinMtx[i])->m_position.y < 0.0f)
			static_cast<Transform *>(G_pCoinMtx[i])->m_position.y *= -1.0f;

		// Update box size accordingly
		// static_cast< Transform * >(G_pCoinMtx[i])->m_position.x += xCntr;
		static_cast<Transform *>(G_pCoinMtx[i])->m_size.x = abs(2.0f * static_cast<Transform *>(G_pCoinMtx[i])->m_position.x);
		static_cast<Transform *>(G_pCoinMtx[i])->m_size.y = abs(2.0f * static_cast<Transform *>(G_pCoinMtx[i])->m_position.y);
		static_cast<Transform *>(G_pCoinMtx[i])->m_position.y += 14.1f;
		static_cast<Transform *>(G_pCoinMtx[i])->generateBoundingBox();
	}

	// Update camera pos, lookat and snake pos
	G_yPos += Window::m_velocity;
	Window::m_camPos.y += Window::m_velocity;
	G_camLookAt.y += Window::m_velocity;

	// Update head's bounding box position
	static_cast<Transform *>(G_pHeadMtx)->m_position.y += Window::m_velocity;
	static_cast<Transform *>(G_pHeadMtx)->generateBoundingBox();
	static_cast<Transform *>(G_pSnake)->generateSnakeContour();

	// Perform collision check
	performCollisions();
}

void Window::resizeCallback(GLFWwindow *window, int width, int height)
{
#ifdef __APPLE__
	glfwGetFramebufferSize(window, &width, &height);	// In case your Mac has a retina display
#endif

	Window::m_width = width;
	Window::m_height = height;

	// Set the viewport size. This is the only matrix that OpenGL maintains for us in modern OpenGL!
	glViewport(0, 0, width, height);

	if (height > 0)
	{
		Window::m_P = glm::perspective(45.0f, static_cast<float>(width) / static_cast<float>(height), 0.1f, 2000.0f);
		Window::m_V = glm::lookAt(Window::m_camPos, G_camLookAt, g_camUp);
	}
}

void Window::keyCallback(GLFWwindow *window, int key, int scanCode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		switch (key)
		{
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window, GL_TRUE);
				break;

			// Toggle fog
			case GLFW_KEY_F:
				Window::m_fog = !Window::m_fog;
				break;

			// Toggle Bounding boxes
			case GLFW_KEY_B:
				G_drawBbox = !G_drawBbox;
				break;

			// Accelerate
			case GLFW_KEY_UP:
			case GLFW_KEY_W:
				Window::m_velocity = SNAKE_SPEED + SPEED_INC;
				break;

			// Slow down
			case GLFW_KEY_DOWN:
			case GLFW_KEY_S:
				Window::m_velocity = SNAKE_SPEED - SPEED_INC;
				break;
		}
	}

	else if (action == GLFW_RELEASE)
	{
		Window::m_velocity = SNAKE_SPEED;
	}
}

glm::vec3 trackBallMapping(glm::vec3 point)
{
	glm::vec3 v;
	float     d;

	v.x = (2.0f * point.x - static_cast<float>(Window::m_width)) / static_cast<float>(Window::m_width);
	v.y = (static_cast<float>(Window::m_height) - 2.0f * point.y) / static_cast<float>(Window::m_height);
	v.z = 0.0f;

	d = glm::length(v);
	d = (d < 1.0f) ? d : 1.0f;
	v.z = sqrtf(1.001f - powf(d, 2.0f));
	glm::normalize(v);

	return v;
}

void Window::cursorPosCallback(GLFWwindow *window, double xPos, double yPos)
{
	glm::vec3 direction, currPoint, rotAxis;
	float     vel, rotAngle;

	switch (Window::m_move)
	{
		case 1:   // Rotation
			currPoint = trackBallMapping(glm::vec3(static_cast<float>(xPos), static_cast<float>(yPos), 0.0f));
			direction = currPoint - Window::m_lastPoint;
			vel = glm::length(direction);

			if (vel > 0.0001f)
			{
				rotAxis = glm::cross(Window::m_lastPoint, currPoint);
				rotAngle = vel * 0.01f;

				// Update camera position
				glm::vec4 l_tmp = glm::rotate(glm::mat4(1.0f), -rotAngle, rotAxis) * glm::vec4(Window::m_camPos, 1.0f);
				m_camPos = glm::vec3(l_tmp.x, l_tmp.y, l_tmp.z);
			}

			break;

		case 2:   // Panning (Not implemented)
			break;
	}
}

void Window::mouseButtonCallback(GLFWwindow *window, int i_button, int action, int mods)
{
	double xPos, yPos;

	if (action == GLFW_PRESS)
	{
		switch (i_button)
		{
			case GLFW_MOUSE_BUTTON_LEFT:
				Window::m_move = 1;

				glfwGetCursorPos(window, &xPos, &yPos);
				Window::m_lastPoint = trackBallMapping(glm::vec3(static_cast<float>(xPos), static_cast<float>(yPos), 0.0f));
				break;

			case GLFW_MOUSE_BUTTON_RIGHT:
				Window::m_move = 2;
				break;
		}
	}

	else if (action == GLFW_RELEASE)
		Window::m_move  = 0;
}

void Window::scrollCallback(GLFWwindow *window, double xOffset, double yOffset)
{
	// Avoid scrolling out of cubemap
	if ((static_cast<int>(yOffset) == -1) && (Window::m_camPos.x > 900.0f || Window::m_camPos.y > 900.0f || Window::m_camPos.z > 900.0f))
		return;

	// Reposition camera to new location
	glm::vec3 dir = Window::m_camPos - G_camLookAt;
	glm::normalize(dir);
	Window::m_camPos -= dir * static_cast<float>(yOffset) * 0.1f;
}
