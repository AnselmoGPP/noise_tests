#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <iostream>
#include <vector>
#include <random>

#include "FastNoiseLite.h"

// -----------------------------------------------------------------------------------

/// Stores all the configuration data that defines some terrain. Used for drawing and in GUI
struct terrainData
{
    // Constructors and operator overloading
    terrainData();                                                                  ///< Default constructor. Configures a default terrain
    terrainData(const terrainData& obj);                                            ///< Copy constructor
    bool operator!= (terrainData& obj);                                             ///< Operator != overloading
    friend std::ostream& operator << (std::ostream& os, const terrainData& obj);    ///< Operator << overloading

    // Noise data   
    FastNoiseLite::NoiseType    noiseType;      ///< Enum (FastNoiseLite::NoiseType): <ul> <li>FastNoiseLite::NoiseType_Value</li> <li>FastNoiseLite::NoiseType_ValueCubic</li> <li>FastNoiseLite::NoiseType_Perlin</li> <li>FastNoiseLite::NoiseType_Cellular</li> <li>FastNoiseLite::NoiseType_OpenSimplex2</li> <li>FastNoiseLite::NoiseType_OpenSimplex2S</li> </ul>
    unsigned int                octaves;        ///< The bigger, the greater (x,y) computed for the highest octaves (exponential thanks to lacunarity), which has consequences: FastNoiseLite::GetNoise(x,y) output -nan(ind)
    float                       lacunarity;     ///< Determines frequency for each octave
    float                       persistance;    ///< Determines amplitude for each octave
    float                       scale;          ///< The bigger, the closer to detail; the smaller, the further away
    float                       multiplier;     ///< Increases height
    unsigned int                seed;           ///< Used for generating different octaves (used if addRandomOffset==true)
    float                       offset[2];      ///< Offset for all octaves

    // Others
    size_t                      dimensions[2];      ///< Dimensions of the map (terrain buffer)
    bool                        newConfig;          ///< Indicates whether the state has changed (used for recomputing and redrawing terrain)
};

std::ostream& operator << (std::ostream& os, const terrainData& obj);

// -----------------------------------------------------------------------------------

/// Given some terrainData object, computes the heigth for certain (x,y) coordinates
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
    noiseSet(terrainData& td, bool addRandomOffset = false);

    float GetNoise(float x, float y, bool includeOffset = false);
    float GetExtreme();
};

// -----------------------------------------------------------------------------------

/// Given a terrainData object, uses noiseSet for building a complete terrain buffer
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

    void computeTerrain(terrainData &td);
    void printData();               // for debugging
};

#endif

// -----------------------------------------------------------------------------------
