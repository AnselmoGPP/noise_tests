#ifndef SHADER_HPP
#define SHADER_HPP

#ifdef IMGUI_IMPL_OPENGL_LOADER_GLEW
#include <GL/glew.h>
#elif IMGUI_IMPL_OPENGL_LOADER_GLAD
#include <glad/glad.h>
#endif
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
    void checkCompileErrors(unsigned int shaderID, std::string type);

public:
    unsigned int ID;

    Shader(const char *vertexPath, const char *fragmentPath);
    void UseProgram();

    // >> Uniforms << --------------------------------------------

    // Boolean ---------------
    void setBool(const std::string &name, bool value) const;
    // Integer ---------------
    void setInt(const std::string &name, int value) const;
    // Float ---------------
    void setFloat(const std::string &name, float value) const;
    // Vector 2 ---------------
    void setVec2(const std::string &name, const glm::vec2 &value) const;
    void setVec2(const std::string &name, float x, float y) const;
    // Vector 3 ---------------
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setVec3(const std::string &name, float x, float y, float z) const;
    // Vector 4 ---------------
    void setVec4(const std::string &name, const glm::vec4 &value) const;
    void setVec4(const std::string &name, float x, float y, float z, float w) const;
    // Matrix 2x2 ---------------
    void setMat2(const std::string &name, const glm::mat2 &mat) const;
    // Matrix 3x3 ---------------
    void setMat3(const std::string &name, const glm::mat3 &mat) const;
    // Matrix 4x4 ---------------
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
};

#endif
