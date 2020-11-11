#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <iostream>
#include <vector>
#include <random>

#include "FastNoiseLite.h"


struct terrainData        // for GUI and drawing terrain
{
    terrainData();
    terrainData(const terrainData& obj);
    bool operator!= (terrainData& obj);

    bool newConfig;      // flag for recomputing terrain

    size_t dimensions[2];
    FastNoiseLite::NoiseType noiseType[6] = { FastNoiseLite::NoiseType_Value, FastNoiseLite::NoiseType_ValueCubic, FastNoiseLite::NoiseType_Perlin, FastNoiseLite::NoiseType_Cellular, FastNoiseLite::NoiseType_OpenSimplex2, FastNoiseLite::NoiseType_OpenSimplex2S };
    unsigned int item_current;
    unsigned int octaves;       // the biggest, the ...
    float lacunarity;
    float persistance;
    float scale;
    float multiplier;
    unsigned int seed;
    float offset[2];            // <<<<<<<<<<< TODO
};

class noiseSet
{
    FastNoiseLite noise;
    FastNoiseLite::NoiseType noiseType;
    unsigned int numOctaves;
    float lacunarity;   // (determines frequency) [1, inf] Increases number of small features
    float persistance;  // (determines amplitude) [0, 1]   Increases influence of small features
    float scale;
    float multiplier;
    std::vector<float[2]> octaveOffsets;

    enum indx{x, y};
    float extreme;     // output range: [-extreme, extreme]

public:
    /*  Configure your noise generator:
     *      NumOctaves: Number of octaves
     *      Lacunarity: Should be in range [1, infinite]
     *      Persistance: Should be in range [0, 1]
     *      Scale: Scaling magnitude
     *      Multiplier: The resulting noise value is multiplied by this
     *      Seed: Generator's seed
     *      Offset: Offset for each octave. Array[n][2]. If nullptr, it's automatically configured
     *      NoiseType: FastNoiseLite::NoiseType_Perlin, FastNoiseLite::NoiseType_OpenSimplex2, etc.
     */
    noiseSet(
            unsigned int NumOctaves,
            float Lacunarity,
            float Persistance,
            float Scale,
            float Multiplier,
            unsigned int Seed = 0,
            float (*Offset)[2] = nullptr,
            FastNoiseLite::NoiseType NoiseType = FastNoiseLite::NoiseType_Perlin
            );

    float GetNoise(float x, float y);
    float GetExtreme();
};

class terrainGenerator
{
    size_t getPos(size_t x, size_t y);

public:
    terrainGenerator();
    ~terrainGenerator();

    size_t side[2];
    unsigned int totalVert;         // Amount of vertices passed to main (example: 3)
    unsigned int totalVertUsed;     // Amount of vertices used for drawing (example: 3*3)

    float (*field)[11];
    unsigned int (*indices)[3];

    void computeTerrain(terrainData td);
    void printData();               // for debugging
};

#endif
