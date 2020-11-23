/*
 *  https://stackoverflow.com/questions/57336940/how-to-glutdisplayfunc-glutmainloop-in-glfw
 */

// Includes --------------------

#include <iostream>

#ifdef IMGUI_IMPL_OPENGL_LOADER_GLEW
#include "GL/glew.h"
#elif IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "glad/glad.h"
#endif
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "auxiliar.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "geometry.hpp"
#include "myGUI.hpp"
#include "canvas.hpp"

// Function declarations --------------------

void framebuffer_resize_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow *window);

void GUI_terrainConfig();
void printOGLdata();

void setUniforms(Shader &myProgram, unsigned texture1);

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
glm::vec3 lightPos(0.0f, 0.0f, 30.0f);
glm::vec3 lightDir(-0.57735f, 0.57735f, 0.57735f);

// buffers
const size_t numBuffers = 2;
enum VBO { terr, axis };

// Terrain data (for GUI)
noiseSet noise;
terrainGenerator terrain(noise, 128, 128);
bool newTerrain = true;


// Function definitions --------------------

int main()
{
    // glfw: initialize and configure
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 1);                                // antialiasing
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
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);  // DISABLED, HIDDEN, NORMAL
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
                "../../../projects/tester_2/shaders/vertexShader.vs",
                "../../../projects/tester_2/shaders/fragmentShader.fs" );

    //Shader lightSourceProgram(
    //            "../../../code/tester/shaders/vertexShader.vs",
    //            "../../../code/tester/shaders/lightSourceFragS.fs" );

    // ----- Set up vertex data, buffers, and configure vertex attributes

    // Terrain creation
    terrain.computeTerrain(noise, 128, 128);

    unsigned int VAO = createVAO();
    unsigned int VBO = createVBO(sizeof(float)*terrain.totalVert*11, terrain.field, GL_STATIC_DRAW);
    unsigned int EBO = createEBO(sizeof(unsigned int)*terrain.totalVertUsed, terrain.indices, GL_STATIC_DRAW);

    int sizes[4] = { 3, 3, 2, 3 };
    configVAO(VAO, VBO, EBO, sizes, 4);

    // Axis
    float coordSys[12][3];
    fillAxis(coordSys, 100);
    unsigned int axisVAO, axisVBO;
    glGenVertexArrays(1, &axisVAO);
    glGenVertexArrays(1, &axisVBO);
/*
    unsigned int lightSourceVAO;
    glGenVertexArrays(1, &lightSourceVAO);
    glBindVertexArray(lightSourceVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)nullptr);
    glEnableVertexAttribArray(0);
*/

    // ----- Load and create a texture
    unsigned texture1 = createTexture2D("../../../textures/grass2.png", GL_RGB);

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
    myProgram.setInt("texture1", 0);        //glUniform1i(glGetUniformLocation(myProgram.ID, "texture1"), 0);
    //glUniform1i(glGetUniformLocation(myProgram.ID, "texture2"), 1);

    // ----- GUI (1) Initialization
    /*
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    ImGui::StyleColorsDark();                                   // ImGui::StyleColorsClassic();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char* glsl_version = "#version 330";
    ImGui_ImplOpenGL3_Init(glsl_version);
*/
    // ----- Other operations

    myGUI gui(window);

    timer.startTime();
    
    // >>>>> RENDER LOOP <<<<<

    while (!glfwWindowShouldClose(window))
    {
        timer.computeDeltaTime();
        
        processInput(window);

        // RENDERING ------------------------
        // ----------------------------------

        glClearColor(0.0f, 0.242f, 0.391f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);           // GL_STENCIL_BUFFER_BIT
        
        // GUI
        gui.implement_NewFrame();
        GUI_terrainConfig();
        mouseOverGUI = gui.cursorOverGUI();

        // Uniforms
        setUniforms(myProgram, texture1);

        // Redraw terrain
        if(newTerrain)
        {
            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float)*terrain.totalVert*11, terrain.field, GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * terrain.totalVertUsed, terrain.indices, GL_STATIC_DRAW);

            newTerrain = false;
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, terrain.totalVertUsed, GL_UNSIGNED_INT, nullptr);    //glDrawArrays(GL_TRIANGLES, 0, 36);

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
        // GUI
        gui.render();

        // ----------------------------------
        // ----------------------------------

        //timer.printTimeData();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // Render loop End


    // ----- De-allocate all resources

    glDeleteVertexArrays(1, &VAO);
    //glDeleteVertexArrays(1, &lightSourceVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(myProgram.ID);
    //glDeleteProgram(lightSourceProgram.ID);
    
    // GUI
    gui.cleanup();
    
    glfwTerminate();

    return 0;
}

// Event handling --------------------------------------------------------------------

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
    if (LMBpressed)
    {
        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = lastX - xpos;
        float yoffset = lastY - ypos;
        lastX = xpos;
        lastY = ypos;

        cam.ProcessMouseMovement(xoffset, yoffset, 1);
    }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    if (!mouseOverGUI)
        cam.ProcessMouseScroll(yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (!mouseOverGUI)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            LMBpressed = true;
        }
        else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            LMBpressed = false;
            firstMouse = true;
        }
    }
}

// Others ----------------------------------------------------------------------------

void printOGLdata()
{
    int maxNumberAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxNumberAttributes);

    std::cout << "-------------------- \n" <<
                 "OpenGL data: " <<
                 "\n    - Version: "                    << glGetString(GL_VERSION) <<
                 "\n    - Vendor: "                     << glGetString(GL_VENDOR) <<
                 "\n    - Renderer: "                   << glGetString(GL_RENDERER) <<
                 "\n    - GLSL version: "               << glGetString(GL_SHADING_LANGUAGE_VERSION) <<
                 "\n    - Max. attributes supported: "  << maxNumberAttributes << std::endl <<
                 "-------------------- \n" << std::endl;
}

void GUI_terrainConfig()
{
    /*
    temp = terrConf;

    // Temporal types
    int dimensionX  = (int)terrConf.dimensions[0];
    int dimensionY  = (int)terrConf.dimensions[1];
    const char* noiseTypeString[6] = { "OpenSimplex2", "OpenSimplex2S", "Cellular", "Perlin", "ValueCubic", "Value" };
    int noiseType   = (int)terrConf.noiseType;
    int octaves     = (int)terrConf.octaves;
    int multiplier  = (int)terrConf.multiplier;
    int seed        = (int)terrConf.seed;
    int offsetX     = (int)terrConf.offset[0];
    int offsetY     = (int)terrConf.offset[1];
*/

    int dimensionX      = terrain.getXside();
    int dimensionY      = terrain.getYside();

    const char* noiseTypeString[6] = { "OpenSimplex2", "OpenSimplex2S", "Cellular", "Perlin", "ValueCubic", "Value" };
    int noiseType       = noise.getNoiseType();
    int numOctaves      = noise.getNumOctaves();
    float lacunarity    = noise.getLacunarity();
    float persistance   = noise.getPersistance();
    float scale         = noise.getScale();
    int multiplier      = noise.getMultiplier();
    int offsetX         = noise.getOffsetX();
    int offsetY         = noise.getOffsetY();
    int seed            = noise.getSeed();

    // Window
    ImGui::Begin("Noise configuration");
    //ImGui::Checkbox("Another Window", &show_another_window);
    ImGui::Text("Map dimensions:");
    ImGui::SliderInt("X dimension", &dimensionX, 2, 256);
    ImGui::SliderInt("Y dimension", &dimensionY, 2, 256);
    ImGui::Text("Noise configuration: ");
    ImGui::Combo("Noise type", &noiseType, noiseTypeString, IM_ARRAYSIZE(noiseTypeString));
    ImGui::SliderInt("Octaves", &numOctaves, 1, 10);
    ImGui::SliderFloat("Lacunarity", &lacunarity, 1.0f, 2.5f);
    ImGui::SliderFloat("Persistance", &persistance, 0.0f, 1.0f);
    ImGui::SliderFloat("Scale", &scale, 0.01f, 1.0f);
    ImGui::SliderInt("Multiplier", &multiplier, 1, 100);
    ImGui::Text("Map selection: ");
    ImGui::SliderInt("Seed", &seed, 0, 100000);
    ImGui::SliderInt("X offset", &offsetX, -500, 500);
    ImGui::SliderInt("Y offset", &offsetY, -500, 500);
    //ImGui::ColorEdit3("clear color", (float*)&clear_color);
    //if (ImGui::Button("Button")) counter++;  ImGui::SameLine();  ImGui::Text("counter = %d", counter);
    //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();

    // Types conversion
    if( dimensionX != terrain.getXside() ||
        dimensionY != terrain.getYside() ||
        noiseType != noise.getNoiseType() ||
        numOctaves != noise.getNumOctaves() ||
        lacunarity != noise.getLacunarity() ||
        persistance != noise.getPersistance() ||
        scale != noise.getScale() ||
        multiplier != noise.getMultiplier() ||
        offsetX != noise.getOffsetX() ||
        offsetY != noise.getOffsetY() ||
        seed != noise.getSeed()
      )
    {
        newTerrain = true;

        noiseSet newNoise((unsigned)numOctaves, lacunarity, persistance, scale, multiplier, offsetX, offsetY, (FastNoiseLite::NoiseType)noiseType, false, (unsigned)seed);
        noise = newNoise;

        terrain.computeTerrain(noise, dimensionX, dimensionY);
    }


/*    terrConf.dimensions[0] = (size_t)dimensionX;
    terrConf.dimensions[1] = (size_t)dimensionY;
    terrConf.noiseType = (FastNoiseLite::NoiseType)noiseType;
    terrConf.octaves = (unsigned int)octaves;
    terrConf.multiplier = (float)multiplier;
    terrConf.seed = (unsigned int)seed;
    terrConf.offset[0] = (float)offsetX;
    terrConf.offset[1] = (float)offsetY;
*/
    // Flag for drawing the terrain again  because data changed
    //if (terrConf != temp)
    //    terrConf.newConfig = true;
}

void setUniforms(Shader &myProgram, unsigned texture1)
{
    myProgram.UseProgram();

    // Fragment shader uniforms
    myProgram.setVec3("objectColor", 0.1f, 0.6f, 0.1f);
    myProgram.setVec3("lightColor",  1.0f, 1.0f, 1.0f);
    myProgram.setVec3("lightPos", lightPos);
    myProgram.setVec3("lightDir", lightDir);
    myProgram.setVec3("camPos", cam.Position);

    // Textures uniforms
    glActiveTexture(GL_TEXTURE0);               // Bind textures on corresponding texture unit
    glBindTexture(GL_TEXTURE_2D, texture1);
    //glActiveTexture(GL_TEXTURE1);
    //glBindTexture(GL_TEXTURE_2D, texture2);

    // Vertex shader uniforms
    glm::mat4 projection = glm::perspective(glm::radians(cam.fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f); // If it doesn't change each frame, it can be placed outside the render loop
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

}
