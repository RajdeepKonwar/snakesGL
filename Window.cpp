#include "Window.h"

#define WINDOW_TITLE "snakesGL"

//! Shader paths
#define GRID_VERT_SHADER  "/Users/lukerohrer/Desktop/CSE167/FinalProj/FinalProj/GridShader.vert"
#define GRID_FRAG_SHADER  "/Users/lukerohrer/Desktop/CSE167/FinalProj/FinalProj/GridShader.frag"
#define BGRID_VERT_SHADER  "/Users/lukerohrer/Desktop/CSE167/FinalProj/FinalProj/BGridShader.vert"
#define BGRID_FRAG_SHADER  "/Users/lukerohrer/Desktop/CSE167/FinalProj/FinalProj/BGridShader.frag"
#define SNAKE_VERT_SHADER "/Users/lukerohrer/Desktop/CSE167/FinalProj/FinalProj/SnakeShader.vert"
#define SNAKE_FRAG_SHADER "/Users/lukerohrer/Desktop/CSE167/FinalProj/FinalProj/SnakeShader.frag"

//! Static data members
int Window::m_width;
int Window::m_height;
int Window::m_move  = 0;
int Window::m_nBody = 4;
int Window::m_nTile = 20;

GLuint g_gridShader, g_bGridShader, g_snakeShader;

Node *g_grid;
Node *g_bGrid;
Node *g_snake;
std::vector< Node * > g_tilePos;
std::vector< Node * > g_bigTilePos;
Node *g_headMtx, *g_tailMtx;
std::vector< Node * > g_bodyMtx;
std::vector< Node * >::iterator g_bodyIt;
std::vector< Node * >::iterator g_tileIt;

Node *g_tile, *g_bigTile, *g_head, *g_body, *g_tail;

//! Default camera parameters
glm::vec3 g_camPos(    0.0f, -3.5f, 2.5f );   //! e | Position of camera
glm::vec3 g_camLookAt( 0.0f,  1.5f, 0.0f );   //! d | Where camera looks at
glm::vec3 g_camUp(     0.0f,  1.0f, 0.0f );   //! u | What orientation "up" is

glm::vec3 Window::m_lastPoint( 0.0f, 0.0f, 0.0f );
glm::mat4 Window::m_P;
glm::mat4 Window::m_V;

void Window::initialize_objects() {
  int l_i, l_j;
    
  g_tile      = new Geometry( "/Users/lukerohrer/Desktop/CSE167/FinalProj/FinalProj/SmallTile.obj" );
  g_bigTile   = new Geometry( "/Users/lukerohrer/Desktop/CSE167/FinalProj/FinalProj/BigTile.obj" );
  g_head      = new Geometry( "/Users/lukerohrer/Desktop/CSE167/FinalProj/FinalProj/Head.obj" );
  g_body      = new Geometry( "/Users/lukerohrer/Desktop/CSE167/FinalProj/FinalProj/Body.obj" );
  g_tail      = new Geometry( "/Users/lukerohrer/Desktop/CSE167/FinalProj/FinalProj/Tail.obj" );

  g_grid    = new Transform( glm::mat4( 1.0f ) );
  g_bGrid   = new Transform( glm::mat4( 1.0f ) );

  g_snake   = new Transform( glm::mat4( 1.0f ) ); 
  g_headMtx = new Transform( glm::translate( glm::mat4( 1.0f ),
                                               glm::vec3( 0.0f, 0.8f, 0.0f ) ) );

   for( l_i = -Window::m_nTile; l_i <= Window::m_nTile; l_i++ ) {
     for( l_j = -Window::m_nTile; l_j <= Window::m_nTile; l_j++ ) {
         g_tilePos.push_back( new Transform( glm::translate( glm::mat4( 1.0f ), glm::vec3( l_j * 2.0f, l_i * 2.0f, 0.0f ) ) ));
         g_bigTilePos.push_back( new Transform( glm::translate( glm::mat4( 1.0f ), glm::vec3( l_j * 2.0f, l_i * 2.0f, -0.01f ) ) ));
     }
   }
    
  for( g_tileIt = g_tilePos.begin(); g_tileIt != g_tilePos.end(); ++g_tileIt ) {
      static_cast< Transform * >(g_grid)->addChild( *g_tileIt );
      static_cast< Transform * >(*g_tileIt)->addChild( g_tile );
  }
  
  for( g_tileIt = g_bigTilePos.begin(); g_tileIt != g_bigTilePos.end(); ++g_tileIt ) {
    static_cast< Transform * >(g_bGrid)->addChild( *g_tileIt );
    static_cast< Transform * >(*g_tileIt)->addChild( g_bigTile );
  }

  for( l_i = 0; l_i < Window::m_nBody; l_i++ )
    g_bodyMtx.push_back( new Transform( glm::translate( glm::mat4( 1.0f ),
                         glm::vec3( 0.0f, -0.5f * (float) l_i, 0.0f ) ) ) );

  g_tailMtx   = new Transform( glm::translate( glm::mat4( 1.0f ),
                glm::vec3( 0.0f, -0.5f * (float) Window::m_nBody, 0.0f ) ) );

  static_cast< Transform * >(g_snake)->addChild( g_headMtx );
  static_cast< Transform * >(g_headMtx)->addChild( g_head );

  for( g_bodyIt = g_bodyMtx.begin(); g_bodyIt != g_bodyMtx.end(); ++g_bodyIt ) {
    static_cast< Transform * >(g_snake)->addChild( *g_bodyIt );
    static_cast< Transform * >(*g_bodyIt)->addChild( g_body );
  }

  static_cast< Transform * >(g_snake)->addChild( g_tailMtx );
  static_cast< Transform * >(g_tailMtx)->addChild( g_tail );

  //! Load the shader programs
  g_gridShader  = LoadShaders( GRID_VERT_SHADER,  GRID_FRAG_SHADER  );
  g_bGridShader = LoadShaders( BGRID_VERT_SHADER, BGRID_FRAG_SHADER );
  g_snakeShader = LoadShaders( SNAKE_VERT_SHADER, SNAKE_FRAG_SHADER );

}

//! Treat this as a destructor function. Delete dynamically allocated memory here.
void Window::clean_up() {
  delete g_grid;
  delete g_snake;

  for( g_tileIt = g_tilePos.begin(); g_tileIt != g_tilePos.end(); ++g_tileIt )
    delete *g_tileIt;
    
  delete g_headMtx;
  for( g_bodyIt = g_bodyMtx.begin(); g_bodyIt != g_bodyMtx.end(); ++g_bodyIt )
    delete *g_bodyIt;
  delete g_tailMtx;

  delete g_tile;
  delete g_head;
  delete g_body;
  delete g_tail;

  glDeleteProgram( g_gridShader  );
  glDeleteProgram( g_snakeShader );
}

GLFWwindow* Window::create_window( int i_width,
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
  Window::resize_callback( l_window, i_width, i_height );

  return l_window;
}

void Window::resize_callback( GLFWwindow* i_window,
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
    Window::m_V = glm::lookAt( g_camPos, g_camLookAt, g_camUp );
  }
}

void Window::idle_callback() {

}

void Window::display_callback( GLFWwindow* i_window ) {
  //! Clear the color and depth buffers
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  // glDepthMask( GL_FALSE );
  glUseProgram( g_bGridShader );
  g_bGrid->draw( g_bGridShader, Window::m_V);

  //! Use GridShader
  glUseProgram( g_gridShader );
  g_grid->draw( g_gridShader, Window::m_V );

  // glDepthMask( GL_TRUE );

  //! Use SnakeShader
  glUseProgram( g_snakeShader );
  g_snake->draw( g_snakeShader, Window::m_V );

  //! Gets events, including input such as keyboard and mouse or window resizing
  glfwPollEvents();

  //! Swap buffers
  glfwSwapBuffers( i_window );

  //! Refresh view matrix with new camera position every display callback
  Window::m_V = glm::lookAt( g_camPos, g_camLookAt, g_camUp );
}

void Window::key_callback( GLFWwindow *i_window,
                           int         i_key,
                           int         i_scancode,
                           int         i_action,
                           int         i_mods ) {
  if( i_action == GLFW_PRESS || i_action == GLFW_REPEAT ) {
    switch( i_key ) {
      case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose( i_window, GL_TRUE );
        break;
    }
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

void Window::cursor_pos_callback( GLFWwindow *i_window, double i_xPos,
                                  double i_yPos ) {
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
                          * glm::vec4( g_camPos, 1.0f );
        g_camPos  = glm::vec3( l_tmp.x, l_tmp.y, l_tmp.z );
      }

      break;

    case 2:   //! Panning (Not implemented)
      break;
  }
}

void Window::mouse_button_callback( GLFWwindow *i_window, int i_button,
                                    int i_action, int i_mods ) {
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

void Window::scroll_callback( GLFWwindow *i_window, double i_xOffset,
                              double i_yOffset ) {
  //! Avoid scrolling out of cubemap
  if( ((int) i_yOffset == -1) && (g_camPos.x > 900.0f || g_camPos.y > 900.0f
                                                      || g_camPos.z > 900.0f) )
    return;

  //! Reposition camera to new location
  glm::vec3 l_dir = g_camPos - g_camLookAt;
  glm::normalize( l_dir );
  g_camPos  -= l_dir * (float) i_yOffset * 0.1f;
}
