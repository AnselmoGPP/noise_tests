/*
 *  https://stackoverflow.com/questions/57336940/how-to-glutdisplayfunc-glutmainloop-in-glfw
 */

// Includes --------------------

#ifdef IMGUI_IMPL_OPENGL_LOADER_GLEW
#include "GL/glew.h"
#elif IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "glad/glad.h"
#endif
#include "GLFW/glfw3.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "FastNoiseLite.h"

#include "auxiliar.hpp"     // chronometer, fps
#include "camera.hpp"
#include "shader.hpp"

#include <iostream>

// Function declarations --------------------

void framebuffer_resize_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

void printOGLdata();

// Settings (typedef and global data section) --------------------

// window size
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera cam(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX =  SCR_WIDTH  / 2.0;
float lastY =  SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
timerSet timer;

// lighting
glm::vec3 lightPos(0.0f, 0.0f, 10.0f);

// Function definitions --------------------

int main()
{
    // glfw: initialize and configure
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 0);                                // antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // To make MacOS happy; should not be needed
#endif

    // ----- GLFW window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Testing", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window (note: Intel GPUs are not 3.3 compatible)" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // ----- Event callbacks and control handling
    glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);        // Sticky keys: Make sure that any pressed key is captured

    // ----- Load OGL function pointers with GLEW or GLAD
#ifdef IMGUI_IMPL_OPENGL_LOADER_GLEW
    glewExperimental = true;        // Needed for core profile (no more from GLEW 2.0)
    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK)
    {
        std::cerr << "GLEW error (" << glewErr << "): " << glewGetErrorString(glewErr) << "\n" << std::endl;
        glfwTerminate();
        return -1;
    }
#elif IMGUI_IMPL_OPENGL_LOADER_GLAD
    // ----- GLAD: Load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "GLAD initialization failed" << std::endl;
        return -1;
    }
#endif

    // ----- OGL general options
    printOGLdata();

    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CCW);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);    // Wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);    // Back to default

    // ----- Build and compile our shader program
    Shader myProgram(
                "../../../code/tester/shaders/vertexShader.vs",
                "../../../code/tester/shaders/fragmentShader.fs" );

    //Shader lightSourceProgram(
    //            "../../../code/tester/shaders/vertexShader.vs",
    //            "../../../code/tester/shaders/lightSourceFragS.fs" );

    // ----- Set up vertex data, buffers, and configure vertex attributes

    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

    const size_t siz = 128;
    float field[siz][siz][11];  //float (*field)[11] = new float[siz*siz][11];  // delete[] field;

    for(int y = 0; y < siz; y++)
        for(int x = 0; x < siz; x++)
        {
            // positions
            field[x][y][0] = x;
            field[x][y][1] = y;
            field[x][y][2] = noise.GetNoise((float)x, (float)y);

            // colors
            field[x][y][3] = 0.5f;
            field[x][y][4] = 0.5f;
            field[x][y][5] = 0.5f;

            // textures
            field[x][y][6] = x;
            field[x][y][7] = y;

            // normals
            field[x][y][8] = 0.0f;
            field[x][y][9] = 0.0f;
            field[x][y][10]= 1.0f;
        }

    unsigned int drawnVertices = (siz-1)*(siz-1)*2*3;
    unsigned int indices[drawnVertices/3][3];       //unsigned int (*indices)[3] = new float[(siz-1)*(siz-1)*2][3]; // delete[] indices;
    int index = 0;

    for(int y = 0; y < siz-1; y++)
        for(int x = 0; x < siz-1; x++)
        {
            // indices
            indices[index  ][0] = x;
            indices[index  ][1] = x + siz;
            indices[index++][2] = x + 1;
            indices[index  ][0] = x + siz;
            indices[index  ][1] = x + siz + 1;
            indices[index++][2] = x + 1;
        }

    unsigned fieldVAO, VBO, EBO;
    glGenVertexArrays(1, &fieldVAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(fieldVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(field), field, GL_STATIC_DRAW);  // GL_DYNAMIC_DRAW, GL_STATIC_DRAW, GL_STREAM_DRAW
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*(siz-1)*(siz-1)*2*3, indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void *)nullptr);              // position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void *)(3 * sizeof(float)));  // color attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));   // texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));   // normals coords
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);           // unbind VBO (not usual)
    glBindVertexArray(0);                       // unbind VAO (not usual)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);   // unbind EBO

/*
    unsigned int lightSourceVAO;
    glGenVertexArrays(1, &lightSourceVAO);
    glBindVertexArray(lightSourceVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)nullptr);
    glEnableVertexAttribArray(0);
*/

    // ----- Load and create a texture
    unsigned texture1, texture2;
    int width, height, numberChannels;
    unsigned char *image;

    stbi_set_flip_vertically_on_load(true);

    // Texture 1
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       // Texture wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   // Texture filtering (?)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    image = stbi_load("../../../code/tester/textures/grass1.jpg", &width, &height, &numberChannels, 0);
    if(image)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, (numberChannels == 4? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else std::cout << "Failed to load texture" << std::endl;

    stbi_image_free(image);

/*
    // Texture 2
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    image = stbi_load("../../../textures/note.png", &width, &height, &numberChannels, 0);
    if(image)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, (numberChannels == 4? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else std::cout << "Failed to load texture" << std::endl;

    stbi_image_free(image);
*/

    // Tell OGL for each sampler to which texture unit it belongs to (only has to be done once)
    myProgram.UseProgram();
    glUniform1i(glGetUniformLocation(myProgram.ID, "texture1"), 0);
    //glUniform1i(glGetUniformLocation(myProgram.ID, "texture2"), 1);

    // ----- Other operations

    timer.startTime();
    timer.setMaxFPS(30);

    // ----- Render loop
    while (!glfwWindowShouldClose(window))
    {
        timer.computeDeltaTime();

        processInput(window);

        // render ----------

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);           // GL_STENCIL_BUFFER_BIT

        myProgram.UseProgram();
        myProgram.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
        myProgram.setVec3("lightColor",  1.0f, 1.0f, 1.0f);
        myProgram.setVec3("lightPos", lightPos);
        myProgram.setVec3("camPos", cam.Position);

        glActiveTexture(GL_TEXTURE0);               // Bind textures on corresponding texture unit
        glBindTexture(GL_TEXTURE_2D, texture1);
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, texture2);

        myProgram.UseProgram();

        //glm::mat4 model = glm::mat4(1.0f);
        //model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        //model = glm::rotate(model, (float)chron.GetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
        //model = glm::scale(model, glm::vec3(1.0, 1.0, 1.0));

        glm::mat4 projection = glm::perspective(glm::radians(cam.fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f); // If it doesn't change each frame, it can be placed outside the render loop
        myProgram.setMat4("projection", projection);

        glm::mat4 view = cam.GetViewMatrix();
        myProgram.setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        myProgram.setMat4("model", model);

        glm::mat3 normalMatrix = glm::mat3( glm::transpose(glm::inverse(model)) );      // Used when the model matrix applies non-uniform scaling (normal won't be scaled correctly). Otherwise, use glm::vec3(model)
        myProgram.setMat3("normalMatrix", normalMatrix);

        glBindVertexArray(fieldVAO);
        glDrawElements(GL_TRIANGLES, (siz-1)*(siz-1)*2*3, GL_UNSIGNED_INT, nullptr);    //glDrawArrays(GL_TRIANGLES, 0, 36);

        /*
        glBindVertexArray(VAO);
        for(unsigned i = 0; i < 10; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            model = glm::rotate(model, currentTime * glm::radians(20.0f * i), glm::vec3(1.0f, 0.3f, 0.5f));
            model = glm::scale(model, glm::vec3(1.0, 1.0, 1.0));
            glUniformMatrix4fv(glGetUniformLocation(myProgram.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLES, 0, 6*2*3);
            //glDrawElements(GL_TRIANGLES, 3*12, GL_UNSIGNED_INT, nullptr);
        }
        */
/*
        lightSourceProgram.UseProgram();
        lightSourceProgram.setMat4("projection", projection);
        lightSourceProgram.setMat4("view", view);

        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(0.2f));
        lightSourceProgram.setMat4("model", model);

        normalMatrix = glm::mat3(model);
        lightingProgram.setMat3("normalMatrix", normalMatrix);

        glBindVertexArray(lightSourceVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
*/
        // -----------------

        //timer.printTimeData();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // Render loop End


    // ----- De-allocate all resources
    glDeleteVertexArrays(1, &fieldVAO);
    //glDeleteVertexArrays(1, &lightSourceVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(myProgram.ID);
    //glDeleteProgram(lightSourceProgram.ID);

    glfwTerminate();

    return 0;
}

// -----------------------------------------------------------------------------------

// GLFW: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_resize_callback(GLFWwindow* window, int width, int height)
{
    //glfwGetFramebufferSize(window, &width, &height);  // Get viewport size from GLFW
    glViewport(0, 0, width, height);                    // Tell OGL the viewport size
    // projection adjustments
}

// Process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Get cameraPos from keys
    float cameraSpeed = 2.5 * timer.getDeltaTime();
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cam.ProcessKeyboard(FORWARD, timer.getDeltaTime());
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cam.ProcessKeyboard(BACKWARD, timer.getDeltaTime());
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cam.ProcessKeyboard(LEFT, timer.getDeltaTime());
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cam.ProcessKeyboard(RIGHT, timer.getDeltaTime());
}

// Get cameraFront from the mouse
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;   // reversed since y-coords go from bottom to top
    lastX = xpos;
    lastY = ypos;

    cam.ProcessMouseMovement(xoffset, yoffset, 0);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    cam.ProcessMouseScroll(yoffset);
}

// Others ----------------------------------

void printOGLdata()
{
    int maxNumberAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxNumberAttributes);

    std::cout << "-------------------- \n" <<
                 "OpenGL data: " <<
                 "\n    - Version: " << glGetString(GL_VERSION) <<
                 "\n    - Vendor: " << glGetString(GL_VENDOR) <<
                 "\n    - Renderer: " << glGetString(GL_RENDERER) <<
                 "\n    - GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) <<
                 "\n    - Max. attributes supported: " << maxNumberAttributes << std::endl <<
                 "-------------------- \n" << std::endl;
}
