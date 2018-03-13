#include "main.h"

GLFWwindow* g_window;

void error_callback( int error, const char* description ) {
  //! Print error
  fputs( description, stderr );
}

void setup_callbacks() {
  //! Set the error callback
  glfwSetErrorCallback( error_callback );

  //! Set the key callback
  glfwSetKeyCallback( g_window, Window::key_callback );

  //! Set cursor position callback
  glfwSetCursorPosCallback( g_window, Window::cursor_pos_callback );

  //! Set mouse button callback
  glfwSetMouseButtonCallback( g_window, Window::mouse_button_callback );

  //! Set scroll callback
  glfwSetScrollCallback( g_window, Window::scroll_callback );

  //! Set the window resize callback
  glfwSetFramebufferSizeCallback( g_window, Window::resize_callback );
}

void setup_glew() {
  //! Initialize GLEW. Not needed on OSX systems.
  #ifndef __APPLE__
  GLenum err = glewInit();
  if( GLEW_OK != err ) {
    /* Problem: glewInit failed, something is seriously wrong. */
    fprintf( stderr, "Error: %s\n", glewGetErrorString( err ) );
    glfwTerminate();
  }

  fprintf( stdout, "Current GLEW version: %s\n", glewGetString( GLEW_VERSION ) );
  #endif
}

void setup_opengl_settings() {
#ifndef __APPLE__
  //! Setup GLEW. Don't do this on OSX systems.
  setup_glew();
#endif
  //! Enable depth buffering
  glEnable( GL_DEPTH_TEST );

  //! Related to shaders and z value comparisons for the depth buffer
  glDepthFunc( GL_LEQUAL );

  //! Set polygon drawing mode to fill front and back of each polygon
  //! You can also use the paramter of GL_LINE instead of GL_FILL to see wireframes
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

  //! Disable backface culling to render both sides of polygons
  // glDisable( GL_CULL_FACE );

  //! Set clear color
  glClearColor( 0.3f, 0.3f, 0.3f, 1.0f );
  glClearDepth(1);
}

void print_versions() {
  //! Get info of GPU and supported OpenGL version
  printf( "Renderer: %s\n", glGetString( GL_RENDERER ) );
  printf( "OpenGL version supported %s\n", glGetString( GL_VERSION ) );

  //! If the shading language symbol is defined
#ifdef GL_SHADING_LANGUAGE_VERSION
  std::printf( "Supported GLSL version is %s.\n",
               (char *) glGetString( GL_SHADING_LANGUAGE_VERSION ) );
#endif
}

int main( int    i_argc,
          char **i_argv ) {
  //! Create the GLFW window
  g_window = Window::create_window( 1600, 900 );

  //! Print OpenGL and GLSL versions
  print_versions();

  //! Setup callbacks
  setup_callbacks();

  //! Setup OpenGL settings, including lighting, materials, etc.
  setup_opengl_settings();

  //! Initialize objects/pointers for rendering
  Window::initialize_objects();

  //! Loop while GLFW window should stay open
  while( !glfwWindowShouldClose( g_window ) ) {
    //! Main render display callback. Rendering of objects is done here.
    Window::display_callback( g_window );

    //! Idle callback. Updating objects, etc. can be done here.
    Window::idle_callback();
  }

  Window::clean_up();

  //! Destroy the window
  glfwDestroyWindow( g_window );

  //! Terminate GLFW
  glfwTerminate();

  return EXIT_SUCCESS;
}
