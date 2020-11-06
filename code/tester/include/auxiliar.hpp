#ifndef AUXILIAR_HPP
#define AUXILIAR_HPP

#include "Eigen/Dense"

#include <chrono>

class timerSet
{
    std::chrono::high_resolution_clock::time_point timeZero;
    std::chrono::high_resolution_clock::time_point lastTime;
    std::chrono::high_resolution_clock::time_point currentTime;

    long double currentTimeSeconds;

    long double deltaTime;

    int FPS;
    int maxFPS;

    size_t frameCounter;

public:
    timerSet(int maxFPS = 0);

    void        startTime();            // Set starting time for the chronometer (startingTime)
    void        computeDeltaTime();     // Compute frame's duration (time between two calls to this)
    void        printTimeData();        // Print relevant values

    long double getDeltaTime();         // Returns time (seconds) increment between frames (deltaTime)
    long double getTime();              // Returns time (seconds) when computeDeltaTime() was called
    long double getTimeNow();           // Returns time (seconds) since timeZero, at the moment of calling GetTimeNow()
    int         getFPS();               // Get FPS
    size_t      getFrameCounter();      // Get frame number (depends on the number of times getDeltaTime() was called)

    void        setMaxFPS(int fps);     // Given a maximum fps, put thread to sleep to get it
};

class stdTime
{
    std::chrono::high_resolution_clock::time_point startingTime;
    std::chrono::high_resolution_clock::time_point currentTime;

public:
    stdTime();

    long double GetTime();       // Returns time in seconds since object creation
};

class fpsCheck
{
    std::chrono::high_resolution_clock::time_point previousTime;
    std::chrono::high_resolution_clock::time_point currentTime;

public:
    fpsCheck();

    int GetFPS();       // Get fps: as a function of time difference between 2 frames
};

namespace EigenCG
{
// Model matrix
Eigen::Matrix4f translate(Eigen::Matrix4f matrix, Eigen::Vector3f position);
Eigen::Matrix4f rotate(Eigen::Matrix4f matrix, float radians, Eigen::Vector3f axis);
Eigen::Matrix4f scale(Eigen::Matrix4f matrix, Eigen::Vector3f factor);

// View matrix
Eigen::Matrix4f lookAt(Eigen::Vector3f camPosition, Eigen::Vector3f target, Eigen::Vector3f upVector);

// Projection matrix
Eigen::Matrix4f ortho(float left, float right, float bottom, float top, float nearP);
Eigen::Matrix4f perspective(float radians, float ratio, float nearPlane, float farPlane);

// Auxiliar
float radians(float sexagesimalDegrees);
float * value_ptr(Eigen::Matrix4f matrix);

} // EigenCG end




#endif
