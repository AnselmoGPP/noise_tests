#ifndef EVENTS_HPP
#define EVENTS_HPP

#include "GLFW/glfw3.h"
#include "imgui.h"
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"

#define GLSL_VERSION "#version 330"

class myGUI
{
    ImGuiIO *io;

public:
    myGUI(GLFWwindow* window);

    void implement_NewFrame();
    void render();
    void cleanup();

    bool cursorOverGUI();
};

#endif
