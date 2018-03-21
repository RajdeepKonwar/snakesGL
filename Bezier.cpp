//
//  Bezier.cpp
//  Assignment3
//
//  Created by Luke Rohrer on 3/1/18.
//  Copyright © 2018 Luke Rohrer. All rights reserved.
//

#include "Bezier.h"
#include "Window.h"

Bezier::Bezier(glm::vec3 points[16])
{
  
  G[0] = glm::mat4(     glm::vec4(points[0].x, points[4].x, points[8].x, points[12].x),
                        glm::vec4(points[1].x, points[5].x, points[9].x, points[13].x),
                        glm::vec4(points[2].x, points[6].x, points[10].x, points[14].x),
                        glm::vec4(points[3].x, points[7].x, points[11].x, points[15].x));
 
  G[1] = glm::mat4(     glm::vec4(points[0].y, points[4].y, points[8].y, points[12].y),
                        glm::vec4(points[1].y, points[5].y, points[9].y, points[13].y),
                        glm::vec4(points[2].y, points[6].y, points[10].y, points[14].y),
                        glm::vec4(points[3].y, points[7].y, points[11].y, points[15].y));
  
  G[2] = glm::mat4(     glm::vec4(points[0].z, points[4].z, points[8].z, points[12].z),
                        glm::vec4(points[1].z, points[5].z, points[9].z, points[13].z),
                        glm::vec4(points[2].z, points[6].z, points[10].z, points[14].z),
                        glm::vec4(points[3].z, points[7].z, points[11].z, points[15].z));
    
  B = glm::mat4(    glm::vec4(-1.0f, 3.0f, -3.0f, 1.0f),
                    glm::vec4(3.0f, -6.0f, 3.0f, 0.0f),
                    glm::vec4(-3.0f, 3.0f, 0.0f, 0.0f),
                    glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
  
  C[0] = B * G[0] * B;
  C[1] = B * G[1] * B;
  C[2] = B * G[2] * B;
    
    glm::vec4 uVector;
    glm::vec4 vVector;
    glm::vec3 x_of_u_v;
    
//    for (float u = 0.0f; u <= 0.9f; u += 0.1f) {
//      for (float v = 0.0f; v <= 0.9f; v += 0.1f) {
  
  int counter = 0;
  int rows = 0;
  
  float u = 0.0f;
  float v = 0.0f;
  
  while (rows <= 100 ) {
    while (counter < 202) {
        uVector = glm::vec4(u*u*u, u*u, u, 1);
        vVector = glm::vec4(v*v*v, v*v, v, 1);
      
        x_of_u_v = glm::vec3(glm::dot(vVector, C[0] * uVector),
                             glm::dot(vVector, C[1] * uVector),
                             glm::dot(vVector, C[2] * uVector));
      
        vertices[rows].push_back(x_of_u_v);
      
      if ( counter % 2 == 0 ){
        u += 0.01f;
      }
      else {
        u -= 0.01f;
        v += 0.01f;
      }
      counter += 1;
    }
    counter = 0;
    v = 0.0f;
    u = 0.0f + rows * 0.01f;
    rows += 1;
  }
  
    
    //taken and modified from OBJObject.cpp
  
  for (int j = 0; j <= 100; j++) {
    
    glGenVertexArrays(1, &VAO[j]);
    glGenBuffers(1, &VBO[j]);

    glBindVertexArray(VAO[j]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[j]);
    glBufferData(GL_ARRAY_BUFFER, vertices[j].size()*sizeof(glm::vec3), &vertices[j][0], GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,
                          3,
                          GL_FLOAT, // What type these components are
                          GL_FALSE, // GL_TRUE means the values should be normalized. GL_FALSE means they shouldn't
                          sizeof(glm::vec3), // Offset between consecutive indices. Since each of our vertices have 3 floats, they should have the size of 3 floats in between
                          (GLvoid*)0); // Offset of the first vertex's component. In our case it's 0 since we don't pad the vertices array with anything.
    
    // Unbind the currently bound buffer so that we don't accidentally make unwanted changes to it.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Unbind the VAO now so we don't accidentally tamper with it.
    // NOTE: You must NEVER unbind the element array buffer associated with a VAO!
    glBindVertexArray(0);
  }
  
  
}

void Bezier::draw(GLuint shaderProgram) {
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &Window::m_P[0][0]);
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelView"), 1, GL_FALSE, &Window::m_V[0][0]);
  glUniform1i( glGetUniformLocation( shaderProgram, "u_surface" ), m_surface );
  
  GLuint l_uFog = glGetUniformLocation( shaderProgram, "u_fog" );
  glUniform1i( l_uFog, Window::m_fog );
  
  GLuint l_uCamPos     = glGetUniformLocation( shaderProgram, "u_camPos"     );
  glUniform3f( l_uCamPos,
              Window::m_camPos.x, Window::m_camPos.y, Window::m_camPos.z );

  for (int i = 0; i <= 100; i++) {
    glBindVertexArray(VAO[i]);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices[i].size());
    glBindVertexArray(0);
  }
  
}