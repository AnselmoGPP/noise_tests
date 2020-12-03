#ifndef GLOBAL_DATA_HPP
#define GLOBAL_DATA_HPP

#include "camera.hpp"
#include "auxiliar.hpp"
#include "geometry.hpp"

// Settings (typedef and global data section)

// camera
Camera cam(glm::vec3(128.0f, -30.0f, 150.0f));
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

// Terrain data (for GUI)
noiseSet noise;
terrainGenerator terrain(noise, 0, 0, 1, 256, 256);
bool newTerrain = true;
float seaLevel = 0;

#endif
