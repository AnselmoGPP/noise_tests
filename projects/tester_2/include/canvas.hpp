#ifndef CANVAS_HPP
#define CANVAS_HPP

#include <array>

/// Create VAO (Vertex Array Object)
unsigned int createVAO();

/// Create VBO (Vertex Buffer Object)
unsigned int createVBO(unsigned long num_bytes, void* pointerToData, int GL_XXX_DRAW);

/// Create EBO (Element Buffer Object)
unsigned int createEBO(unsigned long num_bytes, void* pointerToData, int GL_XXX_DRAW);

/// Configure VAO
void configVAO(unsigned VAO, unsigned VBO, unsigned EBO, int *sizes, unsigned numAtribs);
void configVAO2(unsigned VAO, unsigned VBO, unsigned EBO);

/// Get a 2D texture from a file
unsigned createTexture2D(const char *fileAddress, int internalFormat);

#endif
