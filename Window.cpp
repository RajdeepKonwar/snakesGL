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
 * Window, scene and objects manager.
 **/

#include <fstream>
#include "Window.h"

#define WINDOW_TITLE "snakesGL"
#define CONFIG_FILE  "./snakesGL.conf"

//! Static data members
int Window::m_width;
int Window::m_height;
int Window::m_move  = 0;
int Window::m_nBody = 3;
int Window::m_nTile = 40;
bool Window::m_fog = true;

//! Global variables
GLuint g_gridBigShader,     g_gridSmallShader,    g_snakeShader, g_obstaclesShader;
GLuint g_boundingBoxShader, g_snakeContourShader, g_velocityShader, g_bezierShader;

//! Overclocking on apple to get better fps lul
#ifdef __APPLE__
float Window::m_velocity  = 0.05f;
#else
float Window::m_velocity  = 0.005f;
#endif

float g_yPos      = 0.0f;
int g_move        = 1;
bool g_drawBbox   = false;
float g_rotAngle  = 0.0f;
int g_nPyramids   = 50;
int g_nCoins      = 1;
int g_nWalls      = 50;

Node *g_gridBig, *g_gridSmall;  //! Big and small grid position transform mtx
Node *g_snake;                  //! Snake transform mtx
Node *g_obstacles;

//! Individual elements' transform mtx
Node *g_headMtx, *g_tailMtx, **g_pyramidMtx, **g_coinMtx, **g_wallMtx;
std::vector< Node * > g_tileBigPos, g_tileSmallPos, g_bodyMtx, g_obstaclesList;
std::vector< Node * >::iterator g_nodeIt;

Node *g_head, *g_body, *g_tail, *g_tileBig, *g_tileSmall, *g_coin, *g_wall;

Bezier * patch[4];

//! Default camera parameters
//glm::vec3 Window::m_camPos( 0.0f, 1.8f, 5.0f );//! e | Position of camera (top)
glm::vec3 Window::m_camPos( 0.0f, -3.5f, 3.5f );//! e | Position of camera
glm::vec3 g_camLookAt( 0.0f, 2.5f, 0.0f );      //! d | Where camera looks at
glm::vec3 g_camUp( 0.0f, 1.0f, 0.0f );          //! u | What orientation "up" is

glm::vec3 Window::m_lastPoint( 0.0f, 0.0f, 0.0f );  //! For mouse tracking
glm::mat4 Window::m_P;
glm::mat4 Window::m_V;
glm::mat4 Window::m_prevP;
glm::mat4 Window::m_prevV;

float Window::randGenX() {
  int l_randMax = 8;
  int l_randMin = -8;
  int l_rand    = rand() % (l_randMax - l_randMin + 1) + l_randMin;

  return (2.0f * (float) l_rand);
}

float Window::randGenY() {
  int l_randMax = Window::m_nTile;
  int l_randMin = 2;
  int l_rand    = rand() % (l_randMax - l_randMin + 1) + l_randMin;

  return (2.0f * (float) l_rand);
}

//! functions as constructor
void Window::initializeObjects() {
  //! Seed the randomizer
  srand( time( nullptr ) );

  //! Parse config file for shader and obj paths
  std::ifstream l_confFn( CONFIG_FILE, std::ios::in );
  if( !l_confFn.is_open() ) {
    std::cerr << "Error: cannot open " << CONFIG_FILE << std::endl;
    exit( EXIT_FAILURE );
  }

  std::string l_lineBuf;
  std::string l_gridBigVertShader,      l_gridBigFragShader;
  std::string l_gridSmallVertShader,    l_gridSmallFragShader;
  std::string l_snakeVertShader,        l_snakeFragShader;
  std::string l_obstaclesVertShader,    l_obstaclesFragShader;
  std::string l_boundingBoxVertShader,  l_boundingBoxFragShader;
  std::string l_snakeContourVertShader, l_snakeContourFragShader;
  std::string l_bezierVertShader, l_bezierFragShader;
  std::string l_head, l_body, l_tail,   l_tileBig, l_tileSmall, l_coin, l_wall;

  while( getline( l_confFn, l_lineBuf ) ) {
    size_t l_k = -1, l_l;

    while( (++l_k < l_lineBuf.length()) && (l_lineBuf[l_k] == ' ') );

    if( (l_k >= l_lineBuf.length()) || (l_lineBuf[l_k] == '#') )
      continue;

    l_l = l_k - 1;

    while( (++l_l < l_lineBuf.length()) && (l_lineBuf[l_l] != '=') );

    if( l_l >= l_lineBuf.length() )
      continue;

    std::string l_varName       = l_lineBuf.substr( l_k, l_l - l_k );
    std::string l_varValue      = l_lineBuf.substr( l_l + 1 );

    if( l_varName.compare( "grid_big_vert_shader" ) == 0 )
      l_gridBigVertShader       = l_varValue;
    else if( l_varName.compare( "grid_big_frag_shader" ) == 0 )
      l_gridBigFragShader       = l_varValue;
    else if( l_varName.compare( "grid_small_vert_shader" ) == 0 )
      l_gridSmallVertShader     = l_varValue;
    else if( l_varName.compare( "grid_small_frag_shader" ) == 0 )
      l_gridSmallFragShader     = l_varValue;
    else if( l_varName.compare( "snake_vert_shader" ) == 0 )
      l_snakeVertShader         = l_varValue;
    else if( l_varName.compare( "snake_frag_shader" ) == 0 )
      l_snakeFragShader         = l_varValue;
    else if( l_varName.compare( "obstacles_vert_shader" ) == 0 )
      l_obstaclesVertShader     = l_varValue;
    else if( l_varName.compare( "obstacles_frag_shader" ) == 0 )
      l_obstaclesFragShader     = l_varValue;
    else if( l_varName.compare( "bounding_box_vert_shader" ) == 0 )
      l_boundingBoxVertShader   = l_varValue;
    else if( l_varName.compare( "bounding_box_frag_shader" ) == 0 )
      l_boundingBoxFragShader   = l_varValue;
    else if( l_varName.compare( "snake_contour_vert_shader" ) == 0 )
      l_snakeContourVertShader  = l_varValue;
    else if( l_varName.compare( "snake_contour_frag_shader" ) == 0 )
      l_snakeContourFragShader  = l_varValue;
    else if( l_varName.compare( "bezier_vert_shader" ) == 0 )
      l_bezierVertShader  = l_varValue;
    else if( l_varName.compare( "bezier_frag_shader" ) == 0 )
      l_bezierFragShader  = l_varValue;

    else if( l_varName.compare( "head" ) == 0 )
      l_head                    = l_varValue;
    else if( l_varName.compare( "body" ) == 0 )
      l_body                    = l_varValue;
    else if( l_varName.compare( "tail" ) == 0 )
      l_tail                    = l_varValue;
    else if( l_varName.compare( "tile_big" ) == 0 )
      l_tileBig                 = l_varValue;
    else if( l_varName.compare( "tile_small" ) == 0 )
      l_tileSmall               = l_varValue;
    else if( l_varName.compare( "coin" ) == 0 )
      l_coin               = l_varValue;
    else if( l_varName.compare( "wall" ) == 0 )
      l_wall               = l_varValue;
    else
      std::cout << "\nUnknown setting (" << l_varName << "). Ignored."
                << std::endl;
  }

  l_confFn.close();

  //! Geometry nodes
  g_head      = new Geometry( l_head.c_str()      );
  g_body      = new Geometry( l_body.c_str()      );
  g_tail      = new Geometry( l_tail.c_str()      );
  g_tileSmall = new Geometry( l_tileSmall.c_str() );
  g_tileBig   = new Geometry( l_tileBig.c_str()   );
  g_coin      = new Geometry( l_coin.c_str()      );
  g_wall      = new Geometry( l_wall.c_str()      );

  //! Set geometry obstacle type (for color, 1 by default)
  static_cast< Geometry * >(g_coin)->m_obstacleType = 2;
  static_cast< Geometry * >(g_wall)->m_obstacleType = 3;

  //! Group nodes
  g_gridBig   = new Transform( glm::mat4( 1.0f ) );
  g_gridSmall = new Transform( glm::mat4( 1.0f ) );
  g_snake     = new Transform( glm::mat4( 1.0f ) );
  g_obstacles = new Transform( glm::mat4( 1.0f ) );

  //! Transform modes
  g_headMtx   = new Transform( glm::translate( glm::mat4( 1.0f ),
                                               glm::vec3( 0.0f, 0.8f, 0.0f ) ) );

  //! Head's bounding box is white
  static_cast< Transform * >(g_headMtx)->m_bboxColor = 1;

  //! Bounding boxes' initial positions and sizes
  static_cast< Transform * >( g_headMtx )->m_position   = glm::vec3( -1.0f,
                                                                      1.8f,
                                                                      0.01f );
  static_cast< Transform * >( g_headMtx )->m_size       = glm::vec3(  2.0f,
                                                                      1.5f,
                                                                      0.75f );

  //! Add head to snake
  static_cast< Transform * >(g_snake)->addChild( g_headMtx );
  static_cast< Transform * >(g_headMtx)->addChild( g_head );

  //! Add head to obstacles list as first item (for collision detection)
  g_obstaclesList.push_back( g_headMtx );

  //! Snake body parts' transform mtx
  for( int l_i = 0; l_i < Window::m_nBody; l_i++ )
    g_bodyMtx.push_back( new Transform( glm::translate( glm::mat4( 1.0f ),
                         glm::vec3( 0.0f, -1.0f * (float) l_i, 0.0f ) ) ) );

  //! Add body parts to snake
  for( g_nodeIt = g_bodyMtx.begin(); g_nodeIt != g_bodyMtx.end(); ++g_nodeIt ) {
    static_cast< Transform * >(g_snake)->addChild( *g_nodeIt );
    static_cast< Transform * >(*g_nodeIt)->addChild( g_body );
  }

  //! Tail transform mtx
  g_tailMtx = new Transform( glm::translate( glm::mat4( 1.0f ),
                  glm::vec3( 0.0f,
                             -1.0f * (float) Window::m_nBody + 0.5f,
                             0.0f ) ) );

  //! Add tail to snake
  static_cast< Transform * >(g_snake)->addChild( g_tailMtx );
  static_cast< Transform * >(g_tailMtx)->addChild( g_tail );

  //! Initialize snake contour (white)
  static_cast< Transform * >(g_snake)->generateSnakeContour();

  //! Pyramids transform mtx
  g_pyramidMtx  = new Node * [g_nPyramids];
  for( int l_k = 0; l_k < g_nPyramids; l_k++ ) {
    float l_randX = randGenX();
    float l_randY = randGenY();

    //! Reuse head (rotated by 45) as pyramid obstacle
    glm::mat4 l_moveRotMtx  = glm::translate( glm::mat4( 1.0f ),
                                              glm::vec3( (float) l_randX,
                                                         (float) l_randY,
                                                         0.0f ) ) *
                              glm::rotate( glm::mat4( 1.0f ),
                                           glm::radians( -45.0f ),
                                           glm::vec3( 0.0f, 0.0f, 1.0f ) );
    g_pyramidMtx[l_k] = new Transform( l_moveRotMtx );

    //! Type for collision detection
    static_cast< Transform * >(g_pyramidMtx[l_k])->m_type       = 1;

    //! Bounding boxes' initial positions and sizes
    static_cast< Transform * >( g_pyramidMtx[l_k] )->m_position = glm::vec3(
                                                        -0.7f + (float) l_randX,
                                                         0.7f + (float) l_randY,
                                                         0.01f );
    static_cast< Transform * >( g_pyramidMtx[l_k] )->m_size     = glm::vec3(
                                                                        1.4f,
                                                                        1.4f,
                                                                        0.75f );

    //! Add pyramid as child of obstacles
    static_cast< Transform * >(g_obstacles)->addChild( g_pyramidMtx[l_k] );
    static_cast< Transform * >(g_pyramidMtx[l_k])->addChild( g_head );

    //! Add to obstacles list (for collision detection)
    g_obstaclesList.push_back( g_pyramidMtx[l_k] );
  }

  //! Coins transform mtx
  g_coinMtx  = new Node * [g_nCoins];
  for( int l_k = 0; l_k < g_nCoins; l_k++ ) {
    float l_randX = randGenX();
    float l_randY = randGenY();

    g_coinMtx[l_k]  = new Transform( glm::translate( glm::mat4( 1.0f ),
                                                     glm::vec3( (float) l_randX,
                                                                (float) l_randY,
                                                                0.0f ) ) );

    //! Type for collision detection
    static_cast< Transform * >(g_coinMtx[l_k])->m_type  = 2;

    //! Bounding boxes' initial positions and sizes
    static_cast< Transform * >( g_coinMtx[l_k] )->m_position  = glm::vec3(
                                                        -0.5f + (float) l_randX,
                                                         0.1f + (float) l_randY,
                                                         0.1726f );
    static_cast< Transform * >( g_coinMtx[l_k] )->m_size      = glm::vec3( 1.0f,
                                                                           0.2f,
                                                                           1.15f );

    //! Add coin as child of obstacles
    static_cast< Transform * >(g_obstacles)->addChild( g_coinMtx[l_k] );
    static_cast< Transform * >(g_coinMtx[l_k])->addChild( g_coin );

    //! Add to obstacles list (for collision detection)
    g_obstaclesList.push_back( g_coinMtx[l_k] );
  }

  //! Walls transform mtx
  g_wallMtx  = new Node * [g_nWalls];
  for( int l_k = 0; l_k < g_nWalls; l_k++ ) {
    float l_randX;
    float l_randY = randGenY();

    do {
      l_randX = randGenX();
    } while( l_randX == 0.0f );

    g_wallMtx[l_k]  = new Transform( glm::translate( glm::mat4( 1.0f ),
                                     glm::vec3( (float) l_randX, (float) l_randY,
                                                0.0f ) ) );

    //! Type for collision detection
    static_cast< Transform * >(g_wallMtx[l_k])->m_type        = 3;

    //! Bounding boxes' initial positions and sizes
    static_cast< Transform * >( g_wallMtx[l_k] )->m_position  = glm::vec3(
                                                        -0.7f + (float) l_randX,
                                                         0.7f + (float) l_randY,
                                                         0.01f );
    static_cast< Transform * >( g_wallMtx[l_k] )->m_size      = glm::vec3( 1.4f,
                                                                           1.4f,
                                                                           1.0f );

    //! Add wall as child of obstacles
    static_cast< Transform * >(g_obstacles)->addChild( g_wallMtx[l_k] );
    static_cast< Transform * >(g_wallMtx[l_k])->addChild( g_wall );

    //! Add to obstacles list (for collision detection)
    g_obstaclesList.push_back( g_wallMtx[l_k] );
  }

  //! Initialize obstacles' bounding boxes
  for( g_nodeIt = g_obstaclesList.begin(); g_nodeIt != g_obstaclesList.end();
       ++g_nodeIt )
    static_cast< Transform * >(*g_nodeIt)->generateBoundingBox();

  //! Arrange tiles to form grid
  for( int l_i = -1; l_i <= Window::m_nTile; l_i++ ) {
    for( int l_j = -8; l_j <= 8; l_j++ ) {
      g_tileBigPos.push_back( new Transform( glm::translate( glm::mat4( 1.0f ),
                                             glm::vec3(  l_j * 2.0f,
                                                         l_i * 2.0f,
                                                        -0.1f ) ) ) );
      g_tileSmallPos.push_back( new Transform( glm::translate( glm::mat4( 1.0f ),
                                               glm::vec3( l_j * 2.0f,
                                                          l_i * 2.0f,
                                                          0.0f ) ) ) );
     }
   }

  //! Add big tiles to big grid group
  for( g_nodeIt = g_tileBigPos.begin(); g_nodeIt != g_tileBigPos.end();
        ++g_nodeIt ) {
    static_cast< Transform * >(g_gridBig)->addChild( *g_nodeIt );
    static_cast< Transform * >(*g_nodeIt)->addChild( g_tileBig );
  }

  //! Add small tiles to small grid group
  for( g_nodeIt = g_tileSmallPos.begin(); g_nodeIt != g_tileSmallPos.end();
        ++g_nodeIt ) {
    static_cast< Transform * >(g_gridSmall)->addChild( *g_nodeIt );
    static_cast< Transform * >(*g_nodeIt)->addChild( g_tileSmall );
  }

  //! Load the shader programs
  g_gridBigShader       = LoadShaders( l_gridBigVertShader.c_str(),
                                       l_gridBigFragShader.c_str()      );
  g_gridSmallShader     = LoadShaders( l_gridSmallVertShader.c_str(),
                                       l_gridSmallFragShader.c_str()    );
  g_snakeShader         = LoadShaders( l_snakeVertShader.c_str(),
                                       l_snakeFragShader.c_str()        );
  g_obstaclesShader     = LoadShaders( l_obstaclesVertShader.c_str(),
                                       l_obstaclesFragShader.c_str()    );
  g_boundingBoxShader   = LoadShaders( l_boundingBoxVertShader.c_str(),
                                       l_boundingBoxFragShader.c_str()  );
  g_snakeContourShader  = LoadShaders( l_snakeContourVertShader.c_str(),
                                       l_snakeContourFragShader.c_str() );
  g_bezierShader        = LoadShaders( l_bezierVertShader.c_str(),
                                       l_bezierFragShader.c_str()       );
  
  
  glm::vec3 points0[16] = {

    glm::vec3(-4, 12.50, 0.25),    //p0
    glm::vec3(-3.5, 13.0, 0.25),
    glm::vec3(-3, 13.0, 0.25),
    glm::vec3(-2.5, 12.50, 0.25),  //p3
    glm::vec3(-4, 12.25, 0.75),
    glm::vec3(-3.5, 13.50, 0.75),
    glm::vec3(-3, 13.50, 0.75),
    glm::vec3(-2.5, 12.25, 0.75),  //p7
    glm::vec3(-4, 13.0, 1.25),
    glm::vec3(-3.5, 12.50, 1.25),
    glm::vec3(-3, 12.50, 1.25),
    glm::vec3(-2.5, 13.0, 1.25),   //p11
    glm::vec3(-4, 12.50, 1.75),
    glm::vec3(-3.5, 12.0, 1.75),
    glm::vec3(-3, 12.0, 1.75),
    glm::vec3(-2.5, 12.50, 1.75),  //p15
    
  };
  
  glm::vec3 points1[16] = {
    
    glm::vec3(-2.5, 12.50, 0.25),  //p0
    glm::vec3(-2, 12.0, 0.25),
    glm::vec3(-1.5, 13.0, 0.25),
    glm::vec3(-1, 12.50, 0.25),    //p3
    glm::vec3(-2.5, 12.25, 0.75),
    glm::vec3(-2, 11.0, 0.75),
    glm::vec3(-1.5, 13.50, 0.75),
    glm::vec3(-1, 12.25, 0.75),     //p7
    glm::vec3(-2.5, 13.0, 1.25),
    glm::vec3(-2, 13.50, 1.25),
    glm::vec3(-1.5, 12.50, 1.25),
    glm::vec3(-1, 13.0, 1.25),     //p11
    glm::vec3(-2.5, 12.50, 1.75),
    glm::vec3(-2, 13.0, 1.75),
    glm::vec3(-1.5, 12.0, 1.75),
    glm::vec3(-1, 12.50, 1.75),    //p15
    
  };
  
  glm::vec3 points2[16] = {
    
    glm::vec3(-2.5, 12.50, 1.75),  //p0
    glm::vec3(-2, 13.0, 1.75),
    glm::vec3(-1.5, 12.0, 1.75),
    glm::vec3(-1, 12.50, 1.75),    //p3
    glm::vec3(-2.5, 12.0, 2.25),
    glm::vec3(-2, 12.5, 2.25),
    glm::vec3(-1.5, 11.50, 2.25),
    glm::vec3(-1, 12.0, 2.25),     //p7
    glm::vec3(-2.5, 13.0, 2.75),
    glm::vec3(-2, 13.50, 2.75),
    glm::vec3(-1.5, 12.50, 2.75),
    glm::vec3(-1, 13.0, 2.75),     //p11
    glm::vec3(-2.5, 12.50, 3.25),
    glm::vec3(-2, 13.0, 3.25),
    glm::vec3(-1.5, 12.0, 3.25),
    glm::vec3(-1, 12.50, 3.25),    //p15
    
  };
  
  glm::vec3 points3[16] = {
    
    glm::vec3(-4, 12.50, 1.75),      //p0
    glm::vec3(-3.5, 12.0, 1.75),
    glm::vec3(-3, 12.0, 1.75),
    glm::vec3(-2.5, 12.50, 1.75),    //p3
    glm::vec3(-4, 12.0, 2.25),
    glm::vec3(-3.5, 12.5, 2.25),
    glm::vec3(-3, 11.50, 2.25),
    glm::vec3(-2.5, 12.0, 2.25),     //p7
    glm::vec3(-4, 13.0, 2.75),
    glm::vec3(-3.5, 13.50, 2.75),
    glm::vec3(-3, 12.50, 2.75),
    glm::vec3(-2.5, 13.0, 2.75),     //p11
    glm::vec3(-4, 12.50, 3.25),
    glm::vec3(-3.5, 13.0, 3.25),
    glm::vec3(-3, 12.0, 3.25),
    glm::vec3(-2.5, 12.50, 3.25),    //p15
    
  };
  
  patch[0] = new Bezier(points0);
  patch[1] = new Bezier(points1);
  patch[2] = new Bezier(points2);
  patch[3] = new Bezier(points3);
  
  for( int i = 0; i < 4; i++ )
    patch[i]->m_surface = i + 1;
}

//! Treat this as a destructor function. Delete dynamically allocated memory here.
void Window::cleanUp() {
  delete g_snake;
  delete g_gridBig;
  delete g_gridSmall;
  delete g_obstacles;
  delete g_headMtx;
  delete g_tailMtx;

  // for( int l_i = 0; l_i < g_nPyramids; l_i++ )
  //   delete[] g_pyramidMtx[l_i];
  // delete g_pyramidMtx;
  // for( int l_i = 0; l_i < g_nCoins; l_i++ )
  //   delete[] g_coinMtx[l_i];
  // delete g_coinMtx;
  // for( int l_i = 0; l_i < g_nWalls; l_i++ )
  //   delete[] g_wallMtx[l_i];
  // delete g_wallMtx;

  for( g_nodeIt = g_tileBigPos.begin(); g_nodeIt != g_tileBigPos.end(); ++g_nodeIt )
    delete *g_nodeIt;
  for( g_nodeIt = g_tileSmallPos.begin(); g_nodeIt != g_tileSmallPos.end();
       ++g_nodeIt )
    delete *g_nodeIt;
  for( g_nodeIt = g_bodyMtx.begin(); g_nodeIt != g_bodyMtx.end(); ++g_nodeIt )
    delete *g_nodeIt;

  delete g_head;
  delete g_body;
  delete g_tail;
  delete g_tileBig;
  delete g_tileSmall;
  delete g_coin;
  delete g_wall;

  glDeleteProgram( g_gridBigShader      );
  glDeleteProgram( g_gridSmallShader    );
  glDeleteProgram( g_snakeShader        );
  glDeleteProgram( g_obstaclesShader    );
  glDeleteProgram( g_boundingBoxShader  );
  glDeleteProgram( g_snakeContourShader );
}

bool Window::checkCollision( Node *i_one,
                             Node *i_two ) {
  bool l_collisionX = static_cast< Transform * >(i_one)->m_position.x +
                      static_cast< Transform * >(i_one)->m_size.x >=
                      static_cast< Transform * >(i_two)->m_position.x &&
                      static_cast< Transform * >(i_two)->m_position.x +
                      static_cast< Transform * >(i_two)->m_size.x >=
                      static_cast< Transform * >(i_one)->m_position.x;

  bool l_collisionY = static_cast< Transform * >(i_one)->m_position.y +
                      static_cast< Transform * >(i_one)->m_size.y >=
                      static_cast< Transform * >(i_two)->m_position.y &&
                      static_cast< Transform * >(i_two)->m_position.y +
                      static_cast< Transform * >(i_two)->m_size.y >=
                      static_cast< Transform * >(i_one)->m_position.y;

  return l_collisionX && l_collisionY;
}

void Window::performCollisions() {
  std::vector< Node* >::iterator l_it;

  for( std::vector< Node* >::iterator l_it = g_obstaclesList.begin() + 1;
       l_it != g_obstaclesList.end(); ++l_it ) {
    //! Only check for undestroyed obstacles
    if( !static_cast< Transform * >(*l_it)->m_destroyed ) {
      //! Check each obstacle wrt head
      if( checkCollision( g_headMtx, *l_it ) ) {
        /** Collision with wall
         *  Set both head and wall bbox to red, stop motion of snake
         **/
        if( static_cast< Transform * >(*l_it)->m_type == 3 ) {
          static_cast< Transform * >(*l_it)->m_bboxColor     = 3;
          static_cast< Transform * >(g_headMtx)->m_bboxColor = 3;
          Window::m_velocity = 0.0f;
        }

        //! Set obstacle's bbox to red and destroy it (don't display)
        else {
          static_cast< Transform * >(*l_it)->m_bboxColor = 3;
          static_cast< Transform * >(*l_it)->m_destroyed = true;
        }
      }
    }
  }
}

GLFWwindow* Window::createWindow( int i_width,
                                  int i_height ) {
  //! Initialize GLFW
  if( !glfwInit() ) {
    fprintf( stderr, "Failed to initialize GLFW\n" );
    return NULL;
  }

  //! 4x antialiasing
  glfwWindowHint( GLFW_SAMPLES, 4 );

#ifdef __APPLE__ //! Because Apple hates comforming to standards
  //! Ensure that minimum OpenGL version is 3.3
  glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );

  //! Enable forward compatibility and allow a modern OpenGL context
  glfwWindowHint( GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE );
  glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE                  );
#endif

  //! Create the GLFW window
  GLFWwindow *l_window = glfwCreateWindow( i_width, i_height, WINDOW_TITLE,
                                           nullptr, nullptr );

  //! Check if the window could not be created
  if( !l_window ) {
    fprintf( stderr, "Failed to open GLFW window.\n" );
    fprintf( stderr, "Either GLFW is not installed or your graphics card does \
                      not support modern OpenGL.\n" );
    glfwTerminate();
    return nullptr;
  }

  //! Make the context of the window
  glfwMakeContextCurrent( l_window );

  //! Set swap interval to 1
  glfwSwapInterval( 1 );

  //! Get the width and height of the framebuffer to properly resize the window
  glfwGetFramebufferSize( l_window, &i_width, &i_height );

  //! Call the resize callback to make sure things get drawn immediately
  Window::resizeCallback( l_window, i_width, i_height );

  return l_window;
}

void Window::displayCallback( GLFWwindow* i_window ) {
  GLuint l_uPrevProjection  = glGetUniformLocation( g_velocityShader,
                                                    "u_prevProjection" );
  GLuint l_uPrevModelView   = glGetUniformLocation( g_velocityShader,
                                                    "u_prevModelView"  );
  GLuint l_uProjection      = glGetUniformLocation( g_velocityShader,
                                                    "u_projection" );
  GLuint l_uModelView       = glGetUniformLocation( g_velocityShader,
                                                    "u_modelView"  );
  
  glUniformMatrix4fv( l_uPrevProjection, 1, GL_FALSE, &Window::m_prevP[0][0] );
  glUniformMatrix4fv( l_uPrevModelView,  1, GL_FALSE, &Window::m_prevV[0][0] );
  glUniformMatrix4fv( l_uProjection,     1, GL_FALSE, &Window::m_P[0][0]     );
  glUniformMatrix4fv( l_uModelView,      1, GL_FALSE, &Window::m_V[0][0]     );
  
  glUseProgram( g_velocityShader );
  
  //! Clear the color and depth buffers
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  //! Using GridBigShader, draw black background
  glUseProgram( g_gridBigShader );
  g_gridBig->draw( g_gridBigShader, Window::m_V);

  //! Using GridSmallShader, draw small white tiles
  glUseProgram( g_gridSmallShader );
  g_gridSmall->draw( g_gridSmallShader, Window::m_V );

  //! Using SnakeShader, draw snake
  glUseProgram( g_snakeShader );
  g_snake->draw( g_snakeShader, Window::m_V );

  //! Using SnakeContourShader, draw outline of snake
  glUseProgram( g_snakeContourShader );
  static_cast< Transform * >(g_snake)->drawSnakeContour( g_snakeContourShader,
                                                         Window::m_V );

  //! Using ObstaclesShader, draw the obstacles
  glUseProgram( g_obstaclesShader );
  g_obstacles->draw( g_obstaclesShader, Window::m_V );

  //! Using BoundingBoxShader, draw the axis-aligned bounding boxes (AABB)
  glUseProgram( g_boundingBoxShader );
  if( g_drawBbox )
    for( g_nodeIt = g_obstaclesList.begin(); g_nodeIt != g_obstaclesList.end();
          ++g_nodeIt )
      static_cast< Transform * >(*g_nodeIt)->drawBoundingBox( g_boundingBoxShader,
                                                              Window::m_V );
  
  glUseProgram( g_bezierShader );
  for (int i = 0; i < 4; i++) {
    patch[i]->draw( g_bezierShader );
  }
  //! Gets events, including input such as keyboard and mouse or window resizing
  glfwPollEvents();

  //! Swap buffers
  glfwSwapBuffers( i_window );
  
  Window::m_prevV = Window::m_V;
  
  //! Refresh view matrix with new camera position every display callback
  Window::m_V = glm::lookAt( Window::m_camPos, g_camLookAt, g_camUp );
}

void Window::idleCallback() {
  if( !g_move )
    return;

  if( g_rotAngle >= 360.0f )
    g_rotAngle = 0.0f;
  g_rotAngle += 1.5f;

  //! Move snake in y-direction
  glm::mat4 l_moveMtx = glm::translate( glm::mat4( 1.0f ),
                                        glm::vec3( 0.0f, g_yPos, 0.0f ) );
  static_cast< Transform * >(g_snake)->update( l_moveMtx );

  for( int l_i = 0; l_i < g_nCoins; l_i++ ) {
    //! Find tile center
    float l_xCntr = static_cast< Transform * >(g_coinMtx[l_i])->m_position.x +
                    static_cast< Transform * >(g_coinMtx[l_i])->m_size.x / 2.0f;
    float l_yCntr = static_cast< Transform * >(g_coinMtx[l_i])->m_position.y -
                    static_cast< Transform * >(g_coinMtx[l_i])->m_size.y / 2.0f;

    //! Rotate coins
    glm::mat4 l_rotMtx  = glm::translate( glm::mat4( 1.0f ),
                                          glm::vec3( l_xCntr, l_yCntr, 0.0f ) ) *
                          glm::rotate( glm::mat4( 1.0f ), glm::radians( g_rotAngle ),
                                       glm::vec3( 0.0f, 0.0f, -1.0f ) );
    static_cast< Transform * >(g_coinMtx[l_i])->update( l_rotMtx );

    //! Update coin's bounding box
    static_cast< Transform * >(g_coinMtx[l_i])->m_position.x = 0.5f *
                               cos( M_PI - glm::radians( g_rotAngle ) ) + 0.01f;
    static_cast< Transform * >(g_coinMtx[l_i])->m_position.y = 0.5f *
                               sin( M_PI - glm::radians( g_rotAngle ) ) + 0.01f;

    //! Keep lower left corner such that x is negative and y is positive
    if( static_cast< Transform * >(g_coinMtx[l_i])->m_position.x > 0.0f )
      static_cast< Transform * >(g_coinMtx[l_i])->m_position.x *= -1.0f;
    if( static_cast< Transform * >(g_coinMtx[l_i])->m_position.y < 0.0f )
      static_cast< Transform * >(g_coinMtx[l_i])->m_position.y *= -1.0f;

    //! Update box size accordingly
    // static_cast< Transform * >(g_coinMtx[l_i])->m_position.x += l_xCntr;
    static_cast< Transform * >(g_coinMtx[l_i])->m_size.x = abs( 2.0f *
                            static_cast< Transform * >(g_coinMtx[l_i])->m_position.x );
    static_cast< Transform * >(g_coinMtx[l_i])->m_size.y = abs( 2.0f *
                            static_cast< Transform * >(g_coinMtx[l_i])->m_position.y );
    static_cast< Transform * >(g_coinMtx[l_i])->m_position.y += 14.1f;
    static_cast< Transform * >(g_coinMtx[l_i])->generateBoundingBox();
  }

  //! Update camera pos, lookat and snake pos
  g_yPos             += Window::m_velocity;
  Window::m_camPos.y += Window::m_velocity;
  g_camLookAt.y      += Window::m_velocity;

  //! Update head's bounding box position
  static_cast< Transform * >(g_headMtx)->m_position.y += Window::m_velocity;
  static_cast< Transform * >(g_headMtx)->generateBoundingBox();
  static_cast< Transform * >(g_snake)->generateSnakeContour();

  //! Perform collision check
  performCollisions();
}

void Window::resizeCallback( GLFWwindow* i_window,
                             int         i_width,
                             int         i_height ) {
#ifdef __APPLE__
  glfwGetFramebufferSize( i_window, &i_width, &i_height );  //! In case your Mac has a retina display
#endif

  Window::m_width   = i_width;
  Window::m_height  = i_height;

  //! Set the viewport size. This is the only matrix that OpenGL maintains for us in modern OpenGL!
  glViewport( 0, 0, i_width, i_height );

  if( i_height > 0 ) {
    Window::m_P = glm::perspective( 45.0f, (float) i_width / (float) i_height,
                                    0.1f, 2000.0f );
    Window::m_V = glm::lookAt( Window::m_camPos, g_camLookAt, g_camUp );
    
    Window::m_prevP = Window::m_P;
    Window::m_prevV = Window::m_V;
  }
}

void Window::keyCallback( GLFWwindow *i_window,
                          int         i_key,
                          int         i_scancode,
                          int         i_action,
                          int         i_mods ) {
  if( i_action == GLFW_PRESS || i_action == GLFW_REPEAT ) {
    switch( i_key ) {
      case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose( i_window, GL_TRUE );
        break;

      //! Toggle fog
      case GLFW_KEY_F:
        Window::m_fog = !Window::m_fog;
        break;

      //! Toggle Bounding boxes
      case GLFW_KEY_B:
        g_drawBbox = !g_drawBbox;
        break;

      //! Accelerate
      case GLFW_KEY_UP:
      case GLFW_KEY_W:
         if( Window::m_velocity == 0.0f )
           break;

#ifdef __APPLE__
        Window::m_velocity  = 0.2f;
#else
        Window::m_velocity  = 0.02f;
#endif
        break;

      //! Slow down
      case GLFW_KEY_DOWN:
      case GLFW_KEY_S:
#ifdef __APPLE__
        Window::m_velocity  = 0.05f;
#else
        Window::m_velocity  = 0.005f;
#endif
        break;
    }
  }

  else if( i_action == GLFW_RELEASE ) {
    if( Window::m_velocity == 0.0f )
      return;
    
#ifdef __APPLE__
        Window::m_velocity  = 0.05f;
#else
        Window::m_velocity  = 0.005f;
#endif
  }
}

glm::vec3 trackBallMapping( glm::vec3 i_point ) {
  glm::vec3 l_v;
  float     l_d;

  l_v.x = (2.0f * i_point.x - (float) Window::m_width)  / (float) Window::m_width;
  l_v.y = ((float) Window::m_height - 2.0f * i_point.y) / (float) Window::m_height;
  l_v.z = 0.0f;

  l_d   = glm::length( l_v );
  l_d   = (l_d < 1.0f) ? l_d : 1.0f;
  l_v.z = sqrtf( 1.001f - powf( l_d, 2.0f ) );
  glm::normalize( l_v );

  return l_v;
}

void Window::cursorPosCallback( GLFWwindow *i_window,
                                double      i_xPos,
                                double      i_yPos ) {
  glm::vec3 l_direction, l_currPoint, l_rotAxis;
  float     l_vel, l_rotAngle;

  switch( Window::m_move ) {
    case 1:   //! Rotation
      l_currPoint = trackBallMapping( glm::vec3( (float) i_xPos, (float) i_yPos,
                                      0.0f ) );
      l_direction = l_currPoint - Window::m_lastPoint;
      l_vel       = glm::length( l_direction );

      if( l_vel > 0.0001f ) {
        l_rotAxis   = glm::cross( Window::m_lastPoint, l_currPoint );
        l_rotAngle  = l_vel * 0.01f;

        //! Update camera position
        glm::vec4 l_tmp = glm::rotate( glm::mat4( 1.0f ), -l_rotAngle, l_rotAxis )
                          * glm::vec4( Window::m_camPos, 1.0f );
        m_camPos  = glm::vec3( l_tmp.x, l_tmp.y, l_tmp.z );
      }

      break;

    case 2:   //! Panning (Not implemented)
      break;
  }
}

void Window::mouseButtonCallback( GLFWwindow *i_window,
                                  int         i_button,
                                  int         i_action,
                                  int         i_mods ) {
  double l_xPos, l_yPos;

  if( i_action == GLFW_PRESS ) {
    switch( i_button ) {
      case GLFW_MOUSE_BUTTON_LEFT:
        Window::m_move  = 1;

        glfwGetCursorPos( i_window, &l_xPos, &l_yPos );
        Window::m_lastPoint = trackBallMapping( glm::vec3( (float) l_xPos,
                                                           (float) l_yPos,
                                                           0.0f ) );
        break;

      case GLFW_MOUSE_BUTTON_RIGHT:
        Window::m_move  = 2;
        break;
    }
  }

  else if( i_action == GLFW_RELEASE ) {
    Window::m_move  = 0;
  }
}

void Window::scrollCallback( GLFWwindow *i_window,
                             double      i_xOffset,
                             double      i_yOffset ) {
  //! Avoid scrolling out of cubemap
  if( ((int) i_yOffset == -1) &&
      (Window::m_camPos.x > 900.0f || Window::m_camPos.y > 900.0f
                                              || Window::m_camPos.z > 900.0f) )
    return;

  //! Reposition camera to new location
  glm::vec3 l_dir = Window::m_camPos - g_camLookAt;
  glm::normalize( l_dir );
  Window::m_camPos  -= l_dir * (float) i_yOffset * 0.1f;
}
