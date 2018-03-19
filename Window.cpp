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
int Window::m_nTile = 50;


//! Global variables
GLuint g_gridBigShader, g_gridSmallShader, g_snakeShader, g_obstaclesShader;
GLuint g_boundingBoxShader, g_snakeContourShader, g_coinShader;

//! Overclocking on apple to get better fps lul
#ifdef __APPLE__
float Window::m_velocity  = 0.05f;
#else
float Window::m_velocity  = 0.005f;
#endif

float g_yPos      = 0.0f;
int g_move        = 1;
bool g_drawBbox   = true;
float g_rotAngle = 0.0f;

Node *g_gridBig, *g_gridSmall;  //! Big and small grid position transform mtx
Node *g_snake;                  //! Snake transform mtx
Node *g_obstacles;
Node *g_coins;

//! Individual elements' transform mtx
Node *g_headMtx, *g_tailMtx, *g_pyramidMtx, *g_coinMtx, *g_wallMtx;
std::vector< Node * > g_tileBigPos, g_tileSmallPos, g_bodyMtx, g_obstaclesList;
std::vector< Node * >::iterator g_nodeIt;

Node *g_head, *g_body, *g_tail, *g_tileBig, *g_tileSmall, *g_coin, *g_wall;

//! Default camera parameters
glm::vec3 Window::m_camPos( 0.0f, -3.5f, 3.0f );//! e | Position of camera
glm::vec3 g_camLookAt( 0.0f, 1.8f, 0.0f );      //! d | Where camera looks at
glm::vec3 g_camUp( 0.0f, 1.0f, 0.0f );          //! u | What orientation "up" is

glm::vec3 Window::m_lastPoint( 0.0f, 0.0f, 0.0f );  //! For mouse tracking
glm::mat4 Window::m_P;
glm::mat4 Window::m_V;
glm::mat4 Window::m_prevP;
glm::mat4 Window::m_prevV;

glm::mat4 move = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));

//! functions as constructor
void Window::initializeObjects() {
  int l_i, l_j;

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
  std::string l_coinVertShader,         l_coinFragShader;
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
    else if( l_varName.compare( "coin_vert_shader" ) == 0 )
      l_coinVertShader  = l_varValue;
    else if( l_varName.compare( "coin_frag_shader" ) == 0 )
      l_coinFragShader  = l_varValue;

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
  
  //! Transform nodes
  g_gridBig   = new Transform( glm::mat4( 1.0f ) );
  g_gridSmall = new Transform( glm::mat4( 1.0f ) );
  g_snake     = new Transform( glm::mat4( 1.0f ) );
  g_headMtx   = new Transform( glm::translate( glm::mat4( 1.0f ),
                                               glm::vec3( 0.0f, 0.8f, 0.0f ) ) );
  g_obstacles = new Transform( glm::mat4(1.0f) );
  g_coins     = new Transform( glm::mat4(1.0f) );
  g_coinMtx = new Transform( glm::translate( glm::mat4(1.0f),
                                            glm::vec3( 0.0f, 20.0f, 0.0f ) ) );
  g_wallMtx = new Transform( glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 30.0f, 0.0f)));
  static_cast< Geometry* >(g_wall)->m_obstacleType = 2;

  //! Initialize snake contour
  static_cast< Transform * >(g_snake)->generateSnakeContour( Window::m_nBody );

  //! Using head as pyramid obstacle
  glm::mat4 l_moveRotMtx  = glm::translate( glm::mat4(1.0f),
                                            glm::vec3( 0.0f, 6.0f, 0.0f ) ) *
                            glm::rotate( glm::mat4( 1.0f ),
                                         glm::radians( -45.0f ),
                                         glm::vec3( 0.0f, 0.0f, 1.0f ) );
  g_pyramidMtx  = new Transform ( l_moveRotMtx );
  
  static_cast< Transform* >(g_pyramidMtx)->m_type = 1;
  static_cast< Transform* >(g_coinMtx)->m_type = 2;
  static_cast< Transform* >(g_wallMtx)->m_type = 3;
  
  static_cast< Transform* >(g_headMtx)->m_bboxColor = 1;

  static_cast< Transform* >(g_obstacles)->addChild( g_pyramidMtx );
  static_cast< Transform* >(g_pyramidMtx)->addChild( g_head );
  
  static_cast< Transform* >(g_obstacles)->addChild( g_wallMtx );
  static_cast< Transform* >(g_wallMtx)->addChild( g_wall );
  
  static_cast< Transform* >(g_coins)->addChild( g_coinMtx );
  static_cast< Transform* >(g_coinMtx)->addChild( g_coin );

  //! Bounding boxes' initial positions and sizes
  static_cast< Transform* >( g_headMtx )->m_position = glm::vec3( -1.0f,
                                                                   1.8f,
                                                                   0.0f );
  static_cast< Transform* >( g_headMtx )->m_size     = glm::vec3(  2.0f,
                                                                   1.5f,
                                                                   0.75f );
  
  static_cast< Transform* >( g_pyramidMtx )->m_position = glm::vec3( -0.7f,
                                                                      6.7f,
                                                                      0.01f );
  static_cast< Transform* >( g_pyramidMtx )->m_size     = glm::vec3(  1.4f,
                                                                      1.4f,
                                                                      0.75f );
  
  static_cast< Transform* >( g_wallMtx )->m_position = glm::vec3( -0.7f,
                                                                    30.7f,
                                                                    0.01f );
  static_cast< Transform* >( g_wallMtx )->m_size     = glm::vec3(  1.4f,
                                                                    1.4f,
                                                                    1.0f );
  
  static_cast< Transform* >( g_coinMtx )->m_position    = glm::vec3( -0.5f,
                                                                     20.1f,
                                                                     0.1726f);
  static_cast< Transform* >( g_coinMtx )->m_size        = glm::vec3( 1.0f,
                                                                     0.2f,
                                                                     1.3f );

  //! Arrange tiles to form grid
  for( l_i = -1; l_i <= Window::m_nTile; l_i++ ) {
    //for( l_j = -Window::m_nTile; l_j <= Window::m_nTile; l_j++ ) {
    for( l_j = -8; l_j <= 8; l_j++ ) {
      g_tileBigPos.push_back( new Transform( glm::translate( glm::mat4( 1.0f ),
                                             glm::vec3( l_j * 2.0f,
                                                        l_i * 2.0f,
                                                        -0.1f ) ) ) );
      g_tileSmallPos.push_back( new Transform( glm::translate( glm::mat4( 1.0f ),
                                               glm::vec3( l_j * 2.0f,
                                                          l_i * 2.0f,
                                                          0.0f ) ) ) );
     }
   }

  for( g_nodeIt = g_tileBigPos.begin(); g_nodeIt != g_tileBigPos.end();
        ++g_nodeIt ) {
    static_cast< Transform * >(g_gridBig)->addChild( *g_nodeIt );
    static_cast< Transform * >(*g_nodeIt)->addChild( g_tileBig );
  }

  for( g_nodeIt = g_tileSmallPos.begin(); g_nodeIt != g_tileSmallPos.end();
        ++g_nodeIt ) {
    static_cast< Transform * >(g_gridSmall)->addChild( *g_nodeIt );
    static_cast< Transform * >(*g_nodeIt)->addChild( g_tileSmall );
  }

  //! Snake body parts
  for( l_i = 0; l_i < Window::m_nBody; l_i++ )
    g_bodyMtx.push_back( new Transform( glm::translate( glm::mat4( 1.0f ),
                         glm::vec3( 0.0f, -1.0f * (float) l_i, 0.0f ) ) ) );

  g_tailMtx = new Transform( glm::translate( glm::mat4( 1.0f ),
                  glm::vec3( 0.0f,
                             -1.0f * (float) Window::m_nBody + 0.5f,
                             0.0f ) ) );

  static_cast< Transform * >(g_snake)->addChild( g_headMtx );
  static_cast< Transform * >(g_headMtx)->addChild( g_head );

  for( g_nodeIt = g_bodyMtx.begin(); g_nodeIt != g_bodyMtx.end(); ++g_nodeIt ) {
    static_cast< Transform * >(g_snake)->addChild( *g_nodeIt );
    static_cast< Transform * >(*g_nodeIt)->addChild( g_body );
  }

  static_cast< Transform * >(g_snake)->addChild( g_tailMtx );
  static_cast< Transform * >(g_tailMtx)->addChild( g_tail );

  //! Populate obstacles list
  g_obstaclesList.push_back( g_headMtx );
  g_obstaclesList.push_back( g_pyramidMtx );
  g_obstaclesList.push_back( g_coinMtx );
  g_obstaclesList.push_back( g_wallMtx );

  //! Initialize bounding boxes
  for( g_nodeIt = g_obstaclesList.begin(); g_nodeIt != g_obstaclesList.end();
       ++g_nodeIt )
    static_cast< Transform * >(*g_nodeIt)->generateBoundingBox();

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
  g_coinShader          = LoadShaders( l_coinVertShader.c_str(),
                                       l_coinFragShader.c_str()         );
}

//! Treat this as a destructor function. Delete dynamically allocated memory here.
void Window::cleanUp() {
  delete g_snake;
  delete g_gridBig;
  delete g_gridSmall;
  delete g_headMtx;
  delete g_tailMtx;
  delete g_obstacles;
  delete g_pyramidMtx;

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

  glDeleteProgram( g_gridBigShader   );
  glDeleteProgram( g_gridSmallShader );
  glDeleteProgram( g_snakeShader     );
  glDeleteProgram( g_obstaclesShader   );
  glDeleteProgram( g_boundingBoxShader );
  glDeleteProgram( g_snakeContourShader );
}

bool Window::checkCollision( Node *i_one,
                             Node *i_two ) {
  bool l_collisionX = static_cast< Transform* >(i_one)->m_position.x +
                      static_cast< Transform* >(i_one)->m_size.x >=
                      static_cast< Transform* >(i_two)->m_position.x &&
                      static_cast< Transform* >(i_two)->m_position.x +
                      static_cast< Transform* >(i_two)->m_size.x >=
                      static_cast< Transform* >(i_one)->m_position.x;

  bool l_collisionY = static_cast< Transform* >(i_one)->m_position.y +
                      static_cast< Transform* >(i_one)->m_size.y >=
                      static_cast< Transform* >(i_two)->m_position.y &&
                      static_cast< Transform* >(i_two)->m_position.y +
                      static_cast< Transform* >(i_two)->m_size.y >=
                      static_cast< Transform* >(i_one)->m_position.y;
  
  bool l_collisionZ = static_cast< Transform* >(i_one)->m_position.z +
                      static_cast< Transform* >(i_one)->m_size.z >=
                      static_cast< Transform* >(i_two)->m_position.z &&
                      static_cast< Transform* >(i_two)->m_position.z +
                      static_cast< Transform* >(i_two)->m_size.z >=
                      static_cast< Transform* >(i_one)->m_position.z;

  return (l_collisionX && l_collisionY && l_collisionZ);
}

void Window::performCollisions() {
  std::vector< Node* >::iterator l_it;

  for( std::vector< Node* >::iterator l_it = g_obstaclesList.begin() + 1;
       l_it != g_obstaclesList.end(); ++l_it ) {
    if( !static_cast< Transform * >(*l_it)->m_destroyed ) {
      if( checkCollision( g_headMtx, *l_it ) ) {
        if (static_cast< Transform* >(*l_it)->m_type == 3) {
          static_cast< Transform* >(*l_it)->m_bboxColor = 3;
          static_cast< Transform* >(g_headMtx)->m_bboxColor = 3;
          Window::m_velocity = 0.0f;
        }
        else {
          static_cast< Transform* >(*l_it)->m_bboxColor = 3;
          static_cast< Transform* >(*l_it)->m_destroyed = true;
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
    fprintf( stderr, "Either GLFW is not installed or your graphics card does not support modern OpenGL.\n" );
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
  
  //! Using CoinShader, draw the coins
  glUseProgram( g_coinShader );
  g_coins->draw( g_coinShader, Window::m_V );

  //! Using BoundingBoxShader, draw the axis-aligned bounding boxes (AABB)
  glUseProgram( g_boundingBoxShader );
  if( g_drawBbox )
    for( g_nodeIt = g_obstaclesList.begin(); g_nodeIt != g_obstaclesList.end();
          ++g_nodeIt )
      static_cast< Transform * >(*g_nodeIt)->drawBoundingBox( g_boundingBoxShader,
                                                              Window::m_V );

  //! Gets events, including input such as keyboard and mouse or window resizing
  glfwPollEvents();

  //! Swap buffers
  glfwSwapBuffers( i_window );

  Window::m_prevV = Window::m_V;
  Window::m_prevP = Window::m_P;
  
  //! Refresh view matrix with new camera position every display callback
  Window::m_V = glm::lookAt( Window::m_camPos, g_camLookAt, g_camUp );
}

void Window::idleCallback() {
  if( !g_move )
    return;
  
  if (g_rotAngle >= 360.0f) g_rotAngle = 0.0f;
  g_rotAngle += 1.5f;

  //! Move snake in y-direction
  glm::mat4 l_moveMtx = glm::translate( glm::mat4( 1.0f ),
                                        glm::vec3( 0.0f, g_yPos, 0.0f ) );
  static_cast< Transform * >(g_snake)->update( l_moveMtx );

  glm::mat4 l_rotMtx = glm::translate( glm::mat4( 1.0f ),
                                      glm::vec3 ( 0.0f, 20.0f, 0.0f) ) *
                       glm::rotate( glm::mat4(1.0f), glm::radians(g_rotAngle),
                                   glm::vec3(0.0f, 0.0f, 1.0f));
  
  static_cast< Transform* >(g_coinMtx)->update( l_rotMtx );

  //! Update camera pos, lookat and snake pos
  g_yPos             += Window::m_velocity;
  Window::m_camPos.y += Window::m_velocity;
  g_camLookAt.y      += Window::m_velocity;

  //! Update head's bounding box position
  static_cast< Transform* >( g_headMtx )->m_position.y += Window::m_velocity;
  static_cast< Transform * >(g_headMtx)->generateBoundingBox();
  static_cast< Transform * >(g_snake)->generateSnakeContour( Window::m_nBody );

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

      case GLFW_KEY_UP:
      case GLFW_KEY_W:
#ifdef __APPLE__
        Window::m_velocity  = 0.2f;
#else
        Window::m_velocity  = 0.02f;
#endif
        break;

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
