// Preprocessor Directives
#pragma once

// System Headers
#include <glad/glad.h>

//#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <GLFW/glfw3.h>

// Our own headers
#include "shader.hpp"
#include "scene.h"

// Basic C++ and C headers
#include <iostream>
#include <string>
#include <limits>

#include <math.h>      


# define M_PI           3.14159265358979323846  /* pi */

// Reference: https://github.com/nothings/stb/blob/master/stb_image.h#L4
// To use stb_image, add this in *one* C++ source file.
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>




void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void update_image_texture();
void draw_texture();
unsigned int loadTexture(const char *path);

// settings
unsigned int SCR_WIDTH = 1280;
unsigned int SCR_HEIGHT = 720;

// Image to display
std::vector<glm::u8vec3> image;
// Texture buffer id
unsigned int textureColorbuffer;
// Id of buffers
unsigned int quadVAO, quadVBO;

// The scene object for raytracing
scene* myScene;
