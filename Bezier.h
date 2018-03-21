//
//  Bezier.h
//  Assignment3
//
//  Created by Luke Rohrer on 3/1/18.
//  Copyright © 2018 Luke Rohrer. All rights reserved.
//

#ifndef Bezier_h
#define Bezier_h

#include <stdio.h>
#define GLFW_INCLUDE_GLEXT
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
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
#include <vector>


class Bezier {
    
    private:

        glm::vec3 points[16];
        glm::mat4 G[3];
        glm::mat4 C[3];
        glm::mat4 B;
  
        std::vector< glm::vec3 > vertices[101];
  
    public:
        int m_surface;
        Bezier(glm::vec3 points[16]);
  
        GLuint VAO[101], VBO[101];
  
        void draw(GLuint shaderProgram);

    
};

#endif /* Bezier_h */
