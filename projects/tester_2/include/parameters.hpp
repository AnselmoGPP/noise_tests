#ifndef PARAMETERS_HPP
#define PARAMETERS_HPP

#include "camera.hpp"
#include "auxiliar.hpp"
#include "geometry.hpp"

// Settings (typedef and global data section) --------------------

// window size
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera cam(glm::vec3(64.0f, -30.0f, 90.0f));
float lastX =  SCR_WIDTH  / 2.0;
float lastY =  SCR_HEIGHT / 2.0;
bool firstMouse = true;
bool LMBpressed = false;
bool mouseOverGUI = false;

// timing
timerSet timer(30);

// lighting
glm::vec3 lightCol(1.f, 1.f, 1.f);
glm::vec3 lightPos(-557.f, 577.f, 577.f);
glm::vec3 lightDir(-0.57735f, 0.57735f, 0.57735f);

// buffers
const size_t numBuffers = 2;
enum VBO { terr, axis };

// Terrain data (for GUI)
noiseSet noise;
terrainGenerator terrain(noise, 0, 0, 1, 128, 128);
bool newTerrain = true;
float seaLevel = 0;

#endif
