#pragma once
#include "glad/glad.h"

#include <iostream>

extern const char* vertexShaderSource;
extern const char* fragmentShaderSource;

extern GLuint VAO, VBO, FBO, RBO, texture_id, shaderProgram;

void create_framebuffer(int width, int height);

void bind_framebuffer();

void unbind_framebuffer();

// Can this be done with a callback?
void rescale_framebuffer(float width, float height);

void create_shader();

void create_triangle();
